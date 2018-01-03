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
/* 此库无法修改SCL/SDA，必须连接到5/4管脚。 */
/* 使用TX/RX作为Vcc/Gnd驱动BME280，但导致无法UART无法flash。 */

#include <SoftwareSerial.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/* #define SCL 5 */
/* #define SDA 4 */
int TX_Pin = 1;
int RX_Pin = 3;

const char* ssid = "SJZU";
/* const char* password = ""; */
int num_seconds_to_sleep = 600;
char Sensor_name[]="GY_39_2801";
BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
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

        /* 设置TX/RX为GPIO模式。 */
        /* 设置pinMode，似乎没用。 */
        /* pinMode(TX_Pin, FUNCTION_3); */
        /* pinMode(RX_Pin, FUNCTION_3); */
        pinMode(TX_Pin, OUTPUT);
        pinMode(RX_Pin, OUTPUT);
        digitalWrite(TX_Pin, HIGH);
        digitalWrite(RX_Pin, LOW);

        delay(100);

        Wire.begin();
        while(!bme.begin())
        {
                delay(1000);
        }

        bme.chipModel();
        delay(100);

        /* switch(bme.chipModel()) */
        /* { */
        /* case BME280::ChipModel_BME280: */
        /*         Serial.println("Found BME280 sensor! Success."); */
        /*         break; */
        /* case BME280::ChipModel_BMP280: */
        /*         Serial.println("Found BMP280 sensor! No Humidity available."); */
        /*         break; */
        /* default: */
        /*         Serial.println("Found UNKNOWN sensor! Error!"); */
        /* } */
        float temp(NAN), hum(NAN), pres(NAN);
        BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
        BME280::PresUnit presUnit(BME280::PresUnit_Pa);
        bme.read(pres, temp, hum, tempUnit, presUnit);

        /* 设置TX/RX为UART模式。 */
        /* 设置pinMode，似乎没用。 */
        /* pinMode(TX_Pin, FUNCTION_0); */
        /* pinMode(RX_Pin, FUNCTION_0); */

        /* pinMode(TX_Pin, OUTPUT); */
        /* pinMode(RX_Pin, INPUT); */
        /* SoftwareSerial esp8266 = SoftwareSerial(RX_Pin,TX_Pin); */
        /* esp8266.begin(115200); */
        /* esp8266.println(""); */
        /* esp8266.print("Sensor: "); */
        /* esp8266.println(Sensor_name); */
        /* esp8266.print("Temp: "); */
        /* esp8266.print(temp); */
        /* /\* esp8266.print("°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F')); *\/ */
        /* esp8266.print(" Press: "); */
        /* esp8266.print(pres); */
        /* /\* esp8266.println("Pa"); *\/ */
        /* esp8266.print(" Hum: "); */
        /* esp8266.print(hum); */
        /* /\* esp8266.print("% RH"); *\/ */
        /* /\* esp8266.print(" ALT: "); *\/ */
        /* /\* esp8266.print(Bme.Alt); *\/ */
        /* esp8266.print(" Lux: "); */
        /* esp8266.print(0); */
        /* esp8266.println(""); */
        /* /\* esp8266.end(); *\/ */

        /* 交换TX/RX为GPIO15/13。 */
        /* Serial.swap(); */
        /* Serial.end(); */
        /* Serial.begin(115200); */
        /* /\* 使用GPIO 15/13发送。 *\/ */
        /* Serial.println(""); */
        /* Serial.print("Sensor: "); */
        /* Serial.println(Sensor_name); */
        /* Serial.print("Temp: "); */
        /* Serial.print(temp); */
        /* /\* Serial.print("°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F')); *\/ */
        /* Serial.print(" Press: "); */
        /* Serial.print(pres); */
        /* /\* Serial.println("Pa"); *\/ */
        /* Serial.print(" Hum: "); */
        /* Serial.print(hum); */
        /* /\* Serial.print("% RH"); *\/ */
        /* /\* Serial.print(" ALT: "); *\/ */
        /* /\* Serial.print(Bme.Alt); *\/ */
        /* Serial.print(" Lux: "); */
        /* Serial.print(0); */
        /* Serial.println(""); */

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

                dtostrf((float)temp, 6, 2, Temp_string);  // 相當於 %6.2f
                dtostrf((float)hum, 6, 2, Hum_string);  // 相當於 %6.2f
                dtostrf((float)pres, 6, 2, P_string);  // 相當於 %6.2f
                /* dtostrf((float)Bme.Alt, 6, 2, Alt_string);  // 相當於 %6.2f */
                dtostrf((float)Lux, 6, 2, Lux_string);

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
                                Serial.print(".");
                                delay(100);
                        }

                        s.connect_state = CONNECT_STATE_CONNECTED;
                        put_wifi_state(&s);
                }
        } break;

        case CONNECT_STATE_CONNECTED: {
                /* Serial.println("We're connected, do something with wifi.."); */
                delay(100);

                uart_communication();

                delay(100);

                /* Serial.print("going to deep sleep, num_seconds="); */
                /* Serial.println(num_seconds_to_sleep); */
                s.connect_state = CONNECT_STATE_PREPARE;
                put_wifi_state(&s);
                ESP.deepSleep(num_seconds_to_sleep * 1000000, RF_DISABLED);
        } break;
        }
}
