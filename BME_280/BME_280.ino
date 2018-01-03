/*
  BME280 I2C Test.ino
  This code shows how to record data from the BME280 environmental sensor
  using I2C interface. This file is an example file, part of the Arduino
  BME280 library.
  GNU General Public License
  Written: Dec 30 2015.
  Last Updated: Oct 07 2017.
  Connecting the BME280 Sensor:
  Sensor              ->  Board
  -----------------------------
  Vin (Voltage In)    ->  3.3V
  Gnd (Ground)        ->  Gnd
  SDA (Serial Data)   ->  A4 on Uno/Pro-Mini, 20 on Mega2560/Due, 2 Leonardo/Pro-Micro
  SCK (Serial Clock)  ->  A5 on Uno/Pro-Mini, 21 on Mega2560/Due, 3 Leonardo/Pro-Micro
*/

#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "BME280.h"
#include "Ambient.h"

extern "C" {
#include "user_interface.h"
}

#define _DEBUG 1
#if _DEBUG
#define DBG(...) { Serial.print(__VA_ARGS__); }
#define DBGLED(...) { digitalWrite(__VA_ARGS__); }
#else
#define DBG(...)
#define DBGLED(...)
#endif /* _DBG */

/* 对于ESP8266来说，顺序2045可行，5402不可行，推测0不可以作为SCL/SDA口，只能作为普通IO口。 */
/* 对于D1 mini来说，两者都可以。推测是内部0口连线不同。 */
int VCC_Pin = 2;
int GND_Pin = 0;
#define SCL 4
#define SDA 5

const char* ssid = "SJZU";
/* const char* password = ""; */
int num_seconds_to_sleep = 600;

BME280 bme280;
char Sensor_name[]="GY_39_2801";
uint32_t Lux=0;

#define CONNECT_STATE_PREPARE 0
#define CONNECT_STATE_CONNECTING 1
#define CONNECT_STATE_CONNECTED 2

struct wifi_state_retain_s {
        int initializer = 0b10101010;
        int connect_state = CONNECT_STATE_PREPARE;
};

/* void dump_wifi_state(struct wifi_state_retain_s *s) { */
/*         Serial.print("connect_state="); */
/*         switch(s->connect_state) { */
/*         case CONNECT_STATE_PREPARE: { */
/*                 Serial.println("prepare"); */
/*         } break; */
/*         case CONNECT_STATE_CONNECTING: { */
/*                 Serial.println("connecting"); */
/*         } break; */
/*         case CONNECT_STATE_CONNECTED: { */
/*                 Serial.println("connected"); */
/*         } break; */
/*         } */
/* } */

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

void uart_communication()
{
        pinMode(VCC_Pin, OUTPUT);
        pinMode(GND_Pin, OUTPUT);
        digitalWrite(VCC_Pin, HIGH);
        digitalWrite(GND_Pin, LOW);

        delay(100);
        bme280.begin(SDA, SCL);
        delay(100);

        // 電源投入直後の値は不安定なので、読み捨てる
        bme280.readTemperature();
        bme280.readHumidity();
        bme280.readPressure();
        double temp = 0.0, press = 0.0, hum=0.0;
        temp = bme280.readTemperature();
        hum = bme280.readHumidity();
        press = bme280.readPressure();

        DBG("");
        DBG("Sensor: ");
        DBG(Sensor_name);
        DBG(" Temp: ");
        DBG(temp);
        DBG(" Press: ");
        DBG(press*100);
        DBG(" Hum: ");
        DBG(hum);
        /* DBG("% RH"); */
        /* DBG(" ALT: "); */
        /* DBG(Bme.Alt); */
        DBG(" Lux: ");
        DBG(0);
        DBG("");

        /* 上传。 */
        if(WiFi.status()== WL_CONNECTED)
        {
                /* 发送http post */
                char Temp_string[10];
                char Hum_string[10];
                char P_string[10];
                /* char Alt_string[10]; */
                char Lux_string[10];

                String value_sent;
                HTTPClient http;    //Declare object of class HTTPClient

                http.begin("http://47.94.151.140/"); //Specify request destination
                /* http.addHeader("Content-Type", "text/plain"); //Specify content-type header */
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");

                dtostrf(temp, 6, 2, Temp_string);  // 相當於 %6.2f
                dtostrf(hum, 6, 2, Hum_string);  // 相當於 %6.2f
                dtostrf(press*100, 6, 2, P_string);  // 相當於 %6.2f
                /* dtostrf((float)Bme.Alt, 6, 2, Alt_string);  // 相當於 %6.2f */
                dtostrf(Lux, 6, 2, Lux_string);

                value_sent += "Sensor_name=";
                value_sent += Sensor_name;
                value_sent += "&";
                value_sent += "T=";
                value_sent += Temp_string;
                value_sent += "&";
                value_sent += "H=";
                value_sent += Hum_string;
                value_sent += "&";
                value_sent += "P=";
                value_sent += P_string;
                /* value_sent += "&"; */
                /* value_sent += "A="; */
                /* value_sent += Alt_string; */
                value_sent += "&";
                value_sent += "L=";
                value_sent += Lux_string;
                value_sent += "&";

                int httpCode = http.POST(value_sent); //Send the request
                /* String payload = http.getString(); //Get the response payload */
                http.end();  //Close connection
        }
}

void setup() {
#ifdef _DEBUG
        Serial.begin(115200);
        delay(10);
        /* Serial.end(); */
#endif
        EEPROM.begin(512);
        init_wifi_state();
        // turn off Wifi and disable features we do not want.
        WiFi.mode(WIFI_OFF);
        WiFi.disconnect(true);
        WiFi.setAutoConnect(false);
        WiFi.stopSmartConfig();
        pinMode(16, WAKEUP_PULLUP);
}

void loop() {
        struct wifi_state_retain_s s;
        get_wifi_state(&s);
        /* dump_wifi_state(&s); */
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
                        WiFi.begin(ssid);

                        while ( WiFi.status() != WL_CONNECTED) {
                                DBG(".");
                                delay(100);
                        }

                        s.connect_state = CONNECT_STATE_CONNECTED;
                        put_wifi_state(&s);
                }
        } break;

        case CONNECT_STATE_CONNECTED: {
                delay(100);

                uart_communication();

                delay(100);
                s.connect_state = CONNECT_STATE_PREPARE;
                put_wifi_state(&s);
                ESP.deepSleep(num_seconds_to_sleep * 1000000, RF_DISABLED);
        } break;
        }
}
