/* https://yoursunny.com/t/2017/ESP8266-captive-login/ */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
/* 改变mac地址。https://yoursunny.com/t/2017/change-ESP8266-MAC/ */
/* #include "ChangeMac.hpp" */
extern "C" {
#include <user_interface.h>
}

const char* ssid     = "SJZU";
/* const char* password = "......"; */

bool captiveLogin() {
        static const char* LOCATION = "Location";
        static const char* SET_COOKIE = "Set-Cookie";
        static const char* HEADER_NAMES[] = {LOCATION, SET_COOKIE};

        /* http：若在当前页面返回，返回码为200；若转向其他页面返回，返回码为302。 */
        /* 获取redirect的地址。 */
        String uri;
        {
                HTTPClient http;
                http.begin("http://captive.apple.com/");
                http.collectHeaders(HEADER_NAMES, 2);
                int httpCode = http.GET();
                if (httpCode == 200) {
                        return true;
                }
                if (httpCode != 302 || !http.hasHeader(LOCATION)) {
                        return false;
                }
                uri = http.header(LOCATION);
                Serial.print("portal=");
                Serial.println(uri);
                delay(2000);
        }

        String cookie;
        {
                HTTPClient http;
                http.begin(uri);
                http.collectHeaders(HEADER_NAMES, 2);
                int httpCode = http.GET();
                if (httpCode != 200 || !http.hasHeader(SET_COOKIE)) {
                        return false;
                }
                cookie = http.header(SET_COOKIE);
                Serial.print("cookie=");
                Serial.println(cookie);
                delay(3000);
        }

        {
                int pos = uri.lastIndexOf("/?");
                if (pos < 0) {
                        return false;
                }
                HTTPClient http;
                http.begin(uri.substring(0, pos) + "/login");
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                http.addHeader("Cookie", cookie);
                http.collectHeaders(HEADER_NAMES, 2);
                int httpCode = http.POST("connect=Connect");
                if (httpCode != 302 || !http.hasHeader(LOCATION)) {
                        return false;
                }
                uri = http.header(LOCATION);
                cookie = http.header(SET_COOKIE);
                Serial.print("redirect=");
                Serial.println(uri);
                delay(500);
        }

        {
                HTTPClient http;
                http.begin(uri);
                int httpCode = http.GET();
                if (httpCode != 302) {
                        return false;
                }
                delay(500);
        }

        {
                HTTPClient http;
                http.begin("http://captive.apple.com/");
                int httpCode = http.GET();
                if (httpCode == 200) {
                        return true;
                }
        }

        {
                HTTPClient http;
                http.begin("http://portquiz.net:8080/");
                int httpCode = http.GET();
                return httpCode == 200;
        }
}

void setup() {
        Serial.begin(115200);
        Serial.println();
        Serial.println();

        WiFi.mode(WIFI_STA);
        WiFi.persistent(false);

        /* 改变mac地址。 */
        uint8_t mac[] = {0x9C, 0xC1, 0x72, 0xCA, 0x0F, 0x92};
        /* makeRandomMac(mac); */
        /* changeMac(mac); */
        wifi_set_macaddr(STATION_IF, &mac[0]);
        Serial.print("MAC address is ");
        Serial.println(WiFi.macAddress());

        /* 更改hostname。 */
        /* String hostname = "GY39"; */
        /* 随机hostname。 */
        /* hostname += random(10); */
        /* hostname += random(10); */
        /* hostname += random(10); */
        /* hostname += random(10); */
        /* WiFi.hostname(hostname); */
        /* Serial.print("Hostname is "); */
        /* Serial.println(hostname); */

        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(WiFi.status());
        }

        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());

        if (!captiveLogin()) {
                ESP.restart();
        }
}

void loop() {
}
