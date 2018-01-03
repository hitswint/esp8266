/* #include <Arduino.h> */
/* #include <debug.h> */
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <WebSocketClient.h>

// 定义DS18B20数据口连接arduino的2号IO上
#define ONE_WIRE_BUS 13
// 初始连接在单总线上的单总线设备
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int ledPin = 12;

const char* ssid = "NETGEAR37";
const char* password = "wwghmq2017";

char path[] = "/";
char host[] = "192.168.1.102";

WebSocketClient webSocketClient;
WiFiClient client;

int num_seconds_to_sleep = 60;

#define CONNECT_STATE_PREPARE 0
#define CONNECT_STATE_CONNECTING 1
#define CONNECT_STATE_CONNECTED 2

struct wifi_state_retain_s {
        int initializer = 0b10101010;
        int connect_state = CONNECT_STATE_PREPARE;
};

void dump_wifi_state(struct wifi_state_retain_s *s) {
        Serial.print("connect_state=");
        switch(s->connect_state) {
        case CONNECT_STATE_PREPARE: {
                Serial.println("prepare");
        } break;
        case CONNECT_STATE_CONNECTING: {
                Serial.println("connecting");
        } break;
        case CONNECT_STATE_CONNECTED: {
                Serial.println("connected");
        } break;
        }
}

void get_wifi_state(struct wifi_state_retain_s *s) {
        EEPROM.get(0,*s);
}

void put_wifi_state(struct wifi_state_retain_s *s) {
        EEPROM.put(0,*s);
        EEPROM.commit();
}

void init_wifi_state() {
        struct wifi_state_retain_s s;
        get_wifi_state(&s);
        if (s.initializer != 0b01010101) {
                s.initializer = 0b01010101;
                s.connect_state = CONNECT_STATE_PREPARE;
                put_wifi_state(&s);
        }
}

void websocket_communication()
{
        if (client.connect(host, 80)) {
                Serial.println("Connected");
        } else {
                Serial.println("Connection failed.");
        }
        webSocketClient.path = path;
        webSocketClient.host = host;
        if (webSocketClient.handshake(client)) {
                Serial.println("Handshake successful");
        } else {
                Serial.println("Handshake failed.");
        }

        float data = 0.00;
        char data_string[10];
        String data_received;
        /* String value_sent; */
        sensors.requestTemperatures(); // 发送命令获取温度
        data=sensors.getTempCByIndex(0);
        dtostrf(data, 6, 3, data_string);  // 相當於 %6.3f

        if (client.connected()) {
                webSocketClient.sendData(data_string);
                webSocketClient.getData(data_received);
                Serial.println(data_string);
                Serial.println(data_received);
                if (data_received.length() > 0) {
                        if (data_received == "1") {
                                digitalWrite(ledPin, HIGH);
                        } else {
                                digitalWrite(ledPin, LOW);
                        }
                }
        } else {
                Serial.println("Client disconnected.");
        }

        delay(10);
}


void setup() {
        Serial.begin(115200);
        delay(10);
        /* Serial.setDebugOutput(true); */
        Serial.println("setup()");

        /* EEPROM.begin(size) before you start reading or writing, size being the number of bytes you want to use. Size can be anywhere between 4 and 4096 bytes. */
        EEPROM.begin(512);
        init_wifi_state();

        // turn off Wifi and disable features we do not want.
        WiFi.mode(WIFI_OFF);
        WiFi.disconnect(true);
        WiFi.setAutoConnect(false);
        WiFi.stopSmartConfig();

        pinMode(D0, WAKEUP_PULLUP);

        pinMode(ledPin, OUTPUT);

        // 初始库
        sensors.begin();

        Serial.println("setup() done.");
}


void loop() {
        struct wifi_state_retain_s s;
        get_wifi_state(&s);
        dump_wifi_state(&s);
        switch(s.connect_state) {
        case CONNECT_STATE_PREPARE: {
                s.connect_state = CONNECT_STATE_CONNECTING;
                put_wifi_state(&s);
                // as stated in a number of blogs, this is necessary to
                // make the module activate RF again. But it's a mode of sleep
                // and will continue in setup()
                ESP.deepSleep(1, WAKE_RF_DEFAULT);
        } break;

        case CONNECT_STATE_CONNECTING: {
                if ( WiFi.status() != WL_CONNECTED) {
                        WiFi.mode(WIFI_STA);
                        WiFi.begin(ssid, password);

                        while ( WiFi.status() != WL_CONNECTED) {
                                Serial.print(".");
                                delay(100);
                        }

                        s.connect_state = CONNECT_STATE_CONNECTED;
                        put_wifi_state(&s);
                }
        } break;

        case CONNECT_STATE_CONNECTED: {
                Serial.println("We're connected, do something with wifi..");
                delay(2000);

                websocket_communication();
                delay(2000);

                Serial.print("going to deep sleep, num_seconds="); Serial.println(num_seconds_to_sleep);
                s.connect_state = CONNECT_STATE_PREPARE;
                put_wifi_state(&s);
                ESP.deepSleep(num_seconds_to_sleep * 1000000, RF_DISABLED);
        } break;
        }
}
