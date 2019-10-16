/* 需在Arduino IDE中安装相关library。 */
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>

// 定义DS18B20数据口连接arduino的2号IO上
#define ONE_WIRE_BUS 13
// 初始连接在单总线上的单总线设备
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char* ssid = "";//记得修改为你家的wifi名字密码。
const char* password = "";

WiFiServer server(80);

void setup() {
        Serial.begin(115200);
        delay(10);
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
}
void loop() {
        float value = 0.00;
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

        sensors.requestTemperatures(); // 发送命令获取温度
        value=sensors.getTempCByIndex(0);

        // Set ledPin according to the request
        //digitalWrite(ledPin, value);
        // Return the response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html>");
        client.print("Temperature is: ");
        client.print(value);
        client.println("<br><br>");
        client.println("</html>");
        delay(1);
        Serial.println("Client disonnected");
        Serial.println("");
}
