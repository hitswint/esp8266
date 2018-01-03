/* #include <Arduino.h> */
/* #include <debug.h> */
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// 定义DS18B20数据口连接arduino的2号IO上
#define ONE_WIRE_BUS 13
// 初始连接在单总线上的单总线设备
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int ledPin = 12;

const char* ssid = "NETGEAR37";
const char* password = "wwghmq2017";

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

void http_communication()
{
        float value = 0.00;
        char value_string[10];
        String value_sent;
        sensors.requestTemperatures(); // 发送命令获取温度
        value=sensors.getTempCByIndex(0);

        //Check WiFi connection status
        if(WiFi.status()== WL_CONNECTED)
        {
                /* 发送http post */
                HTTPClient http;    //Declare object of class HTTPClient

                http.begin("http://192.168.1.102/"); //Specify request destination
                /* http.begin("http://67.209.178.147:80/"); //Specify request destination */
                /* http.addHeader("Content-Type", "text/plain"); //Specify content-type header */
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                dtostrf(value, 6, 3, value_string);  // 相當於 %6.3f
                value_sent += "temperature_data=";
                value_sent += value_string;
                value_sent += "&";
                int httpCode = http.POST(value_sent); //Send the request
                String payload = http.getString(); //Get the response payload

                if(payload == "1") {
                        digitalWrite(ledPin, HIGH);
                } else {
                        digitalWrite(ledPin, LOW);
                }

                Serial.println(value_sent); //Print HTTP return code
                Serial.println(httpCode);   //Print HTTP return code
                Serial.println(payload);    //Print request response payload

                http.end();  //Close connection
        }
}


void setup() {
        Serial.begin(115200);
        delay(100);
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
                delay(100);

                http_communication();
                delay(100);

                Serial.print("going to deep sleep, num_seconds="); Serial.println(num_seconds_to_sleep);
                s.connect_state = CONNECT_STATE_PREPARE;
                put_wifi_state(&s);
                ESP.deepSleep(num_seconds_to_sleep * 1000000, RF_DISABLED);
        } break;
        }
}
