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

const char* ssid = "SSID";
const char* password = "password";

char path[] = "/";
char host[] = "192.168.1.102";

WebSocketClient webSocketClient;
WiFiClient client;

// ESP8266 Timer Example
extern "C" {
#include "user_interface.h"
}

os_timer_t myTimer;

bool tickOccured;

void websocket_communication()
{
        float data = 0.00;
        char data_string[10];
        /* String value_sent; */
        sensors.requestTemperatures(); // 发送命令获取温度
        data=sensors.getTempCByIndex(0);
        dtostrf(data, 6, 3, data_string);  // 相當於 %6.3f

        if (client.connected()) {
                webSocketClient.sendData(data_string);
                Serial.println(data_string);
        } else {
                Serial.println("Client disconnected.");
        }
}

// start of timerCallback
void timerCallback(void *pArg) {
        tickOccured = true;
}

void user_init(void) {
        /*
          os_timer_setfn - Define a function to be called when the timer fires
          void os_timer_setfn(
          os_timer_t *pTimer,
          os_timer_func_t *pFunction,
          void *pArg)
          Define the callback function that will be called when the timer reaches zero. The pTimer parameters is a pointer to the timer control structure.
          The pFunction parameters is a pointer to the callback function.
          The pArg parameter is a value that will be passed into the called back function. The callback function should have the signature:
          void (*functionName)(void *pArg)
          The pArg parameter is the value registered with the callback function.
        */
        os_timer_setfn(&myTimer, timerCallback, NULL);

        /*
          os_timer_arm -  Enable a millisecond granularity timer.
          void os_timer_arm(
          os_timer_t *pTimer,
          uint32_t milliseconds,
          bool repeat)
          Arm a timer such that is starts ticking and fires when the clock reaches zero.
          The pTimer parameter is a pointed to a timer control structure.
          The milliseconds parameter is the duration of the timer measured in milliseconds. The repeat parameter is whether or not the timer will restart once it has reached zero.
        */
        os_timer_arm(&myTimer, 10000, true);
}

void setup() {
        Serial.begin(115200);
        delay(10);
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);
        // Connect to WiFi network
        Serial.println();
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        delay(5000);
        // 初始库
        sensors.begin();
        tickOccured = false;
        user_init();

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
}

void loop() {
        if (tickOccured == true)
        {
                Serial.println("Tick Occurred");
                websocket_communication();
                tickOccured = false;
        }

        String data_received;
        if (client.connected()) {
                webSocketClient.getData(data_received);
                /* Serial.println(data_received); */
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
