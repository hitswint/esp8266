/* Arduino IDE中C-M-s在当前目录下生成.bin文件，命名为Sensor_name + FW_VERSION + .bin，然后拷贝到Fota文件夹中。 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

const char* ssid = "changlang";
/* const char* password = ""; */
const int FW_VERSION = 18010811;
const char* fwUrlBase = "http://202.199.66.23/Fota/";
char Sensor_name[]="GY_39_01";

void checkForUpdates()
{
        String fwURL = String( fwUrlBase );
        fwURL.concat( Sensor_name );
        String fwVersionURL = fwURL;
        fwVersionURL.concat( ".version" );

        Serial.print( "Firmware: " );
        Serial.println( fwVersionURL );

        HTTPClient httpClient;
        httpClient.begin( fwVersionURL );
        int httpCode = httpClient.GET();
        if( httpCode == 200 ) {
                String newFWVersion = httpClient.getString();

                Serial.print( "Curr: " );
                Serial.println( FW_VERSION );
                Serial.print( "Available: " );
                Serial.println( newFWVersion );

                int newVersion = newFWVersion.toInt();

                if( newVersion > FW_VERSION ) {
                        Serial.println( "update" );

                        String fwImageURL = fwURL;
                        fwImageURL.concat( ".bin" );
                        t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );

                        switch(ret) {
                        case HTTP_UPDATE_FAILED:
                                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                                break;

                        case HTTP_UPDATE_NO_UPDATES:
                                Serial.println("HTTP_UPDATE_NO_UPDATES");
                                break;
                        }
                }
                else {
                        Serial.println( "Already on latest version" );
                }
        }
        else {
                Serial.print( "Check failed, got HTTP response code " );
                Serial.println( httpCode );
        }
        httpClient.end();
}

void setup() {
        // initialize digital pin 13 as an output.
        Serial.begin(115200);

        WiFi.begin(ssid);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
        }
        checkForUpdates();
}
// the loop function runs over and over again forever
void loop() {
        delay(1000);              // wait for a second
}
