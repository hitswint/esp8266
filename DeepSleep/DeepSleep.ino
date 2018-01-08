/* #include <Arduino.h> */
/* #include <debug.h> */
#include <ESP8266WiFi.h>
#include <EEPROM.h>

const char* ssid     = "SSID";
const char* password = "password";

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

int num_seconds_to_sleep = 10;

void setup() {
        Serial.begin(115200);
        Serial.setDebugOutput(true);
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

        pinMode(12, OUTPUT);

        Serial.println("setup() done.");
}

void loop() {
        Serial.println("in loop()");

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

                digitalWrite(12, HIGH);        //对观察灯写入状态值
                delay(2000);
                digitalWrite(12, LOW);        //对观察灯写入状态值
                delay(2000);

                Serial.print("going to deep sleep, num_seconds="); Serial.println(num_seconds_to_sleep);
                s.connect_state = CONNECT_STATE_PREPARE;
                put_wifi_state(&s);
                ESP.deepSleep(num_seconds_to_sleep * 1000000, RF_DISABLED);
        } break;
        }
}
