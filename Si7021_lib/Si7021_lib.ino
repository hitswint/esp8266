/* 需在Arduino IDE中安装相关library。 */
#include <Wire.h>
#include <SI7021.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/* SCL接D1，SDA接D2。 */
#define SCL 5
#define SDA 4

SI7021 sensor;

const char* ssid = "NETGEAR37";
const char* password = "wwghmq2017";

// ESP8266 Timer Example
extern "C" {
#include "user_interface.h"
}

os_timer_t myTimer;

bool tickOccured;

void request_post()
{
        int temperature = sensor.getCelsiusHundredths();
        int humidity = sensor.getHumidityPercent();

        Serial.print("TEMP: ");
        Serial.print(temperature);
        Serial.print(" HUMI: ");
        Serial.print(humidity);
        Serial.println("");
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

        sensor.begin(SDA,SCL);

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
}
