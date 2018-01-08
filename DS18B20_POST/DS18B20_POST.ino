/* 需在Arduino IDE中安装相关library。 */
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

const char* ssid = "SSID";
const char* password = "password";

WiFiServer server(80);

// ESP8266 Timer Example
extern "C" {
#include "user_interface.h"
}

os_timer_t myTimer;

bool tickOccured;

void request_post()
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
        os_timer_arm(&myTimer, 5000, true);
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
        // Start the server
        server.begin();
        Serial.println("Server started");
        // Print the IP address
        Serial.print("Use this URL to connect: ");
        Serial.print("http://");
        Serial.print(WiFi.localIP());
        Serial.println("/");
        // 初始库
        sensors.begin();
        tickOccured = false;
        user_init();
}



void loop() {

        if (tickOccured == true)
        {
                Serial.println("Tick Occurred");
                request_post();
                tickOccured = false;

        }
        /* 开启server. */
        // Check if a client has connected
        WiFiClient client = server.available();
        if (!client) {
                return;
        }
        // Wait until the client sends some data
        Serial.println("new client");
        while(!client.available()){
                delay(1);
        }
        // Read the first line of the request
        String request = client.readStringUntil('\r');
        Serial.println(request);
        client.flush();
        // Match the request
        int on_off = LOW;
        if (request.indexOf("/LED=ON") != -1)  {
                digitalWrite(ledPin, HIGH);
                on_off = HIGH;
        }
        if (request.indexOf("/LED=OFF") != -1)  {
                digitalWrite(ledPin, LOW);
                on_off = LOW;
        }
        // Return the response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.print("Led pin is now: ");
        if(on_off == HIGH) {
                client.print("On");
        } else {
                client.print("Off");
        }
        client.println("<br><br>");
        client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
        client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");
        client.println("</html>");
        delay(1);
        Serial.println("Client disonnected");
        Serial.println("");
}
