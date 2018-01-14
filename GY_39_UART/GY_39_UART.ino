#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
/* ESPhttpUpdate需flash空间足够大，BOARD不能设置为generic，需为espino。*/
#include <ESP8266httpUpdate.h>

const int FW_VERSION = 18011324;
const char* fwUrlBase = "http://47.94.151.140/";

const char* ssid = "SJZU";
/* const char* password = ""; */

int num_seconds_to_sleep = 600;
char Sensor_name[]="GY_39_01";
int Sensor_vcc_pin = 5;
int Sensor_gnd_pin = 4;

unsigned char Re_buf[15],counter=0;
unsigned char Re_buf_bme[15];
unsigned char Re_buf_max[9];
unsigned char sign_bme=0, sign_max=0;

typedef struct
{
        uint32_t P;
        uint16_t Temp;
        uint16_t Hum;
        /* uint16_t Alt; */
} bme;

bme Bme;
uint32_t Lux;

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

void checkForUpdates()
{
        String fwURL = String( fwUrlBase );
        fwURL.concat( "Fota/" );
        fwURL.concat( Sensor_name );
        String fwVersionURL = fwURL;
        fwVersionURL.concat( ".version" );

        /* Serial.print( "Firmware: " ); */
        /* Serial.println( fwVersionURL ); */

        HTTPClient httpClient;
        httpClient.begin( fwVersionURL );
        int httpCode = httpClient.GET();
        if( httpCode == 200 ) {
                String newFWVersion = httpClient.getString();

                /* Serial.print( "Curr: " ); */
                /* Serial.println( FW_VERSION ); */
                /* Serial.print( "Available: " ); */
                /* Serial.println( newFWVersion ); */

                int newVersion = newFWVersion.toInt();

                if( newVersion > FW_VERSION ) {
                        /* Serial.println( "update" ); */

                        String fwImageURL = fwURL;
                        fwImageURL.concat( ".bin" );
                        t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );

                        /* switch(ret) { */
                        /* case HTTP_UPDATE_FAILED: */
                        /*         Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); */
                        /*         break; */

                        /* case HTTP_UPDATE_NO_UPDATES: */
                        /*         Serial.println("HTTP_UPDATE_NO_UPDATES"); */
                        /*         break; */

                        /* case HTTP_UPDATE_OK: */
                        /*         serial.println("HTTP_UPDATE_OK"); */
                        /*         break; */
                        /* } */
                }
                /* else { */
                /*         Serial.println( "Already on latest version" ); */
                /* } */
        }
        /* else { */
        /*         Serial.print( "Check failed, got HTTP response code " ); */
        /*         Serial.println( httpCode ); */
        /* } */
        httpClient.end();
}

void serialEvent() {
        while (Serial.available()) {
                Re_buf[counter]=(unsigned char)Serial.read();
                counter++;

                if ((counter==1&&Re_buf[0]!=0x5A) || (counter==2&&Re_buf[1]!=0x5A))
                {
                        counter=0;
                        continue;      // 检查帧头
                }

                if (Re_buf[2]==0X15 && counter==9)
                {
                        counter=0;                 //重新赋值，准备下一帧数据的接收
                        unsigned char j=0,sum_max=0;
                        for(j=0;j<8;j++)
                        {
                                Re_buf_max[j]=Re_buf[j];
                                sum_max+=Re_buf_max[j];
                        }
                        if(sum_max==Re_buf[j])
                        {
                                Re_buf_max[j]=Re_buf[j];
                                sign_max=1;
                        }
                }

                if (Re_buf[2]==0X45 && counter==15)
                {
                        counter=0;                 //重新赋值，准备下一帧数据的接收
                        unsigned char i=0,sum_bme=0;
                        for(i=0;i<14;i++)
                        {
                                Re_buf_bme[i]=Re_buf[i];
                                sum_bme+=Re_buf_bme[i];
                        }
                        if(sum_bme==Re_buf[i])
                        {
                                Re_buf_bme[i]=Re_buf[i];
                                sign_bme=1;
                        }
                }
                /* 从GY-39共传输128组数据，过早读取lux会显示为0。 */
                /* if (sign_bme && sign_max) break; */
                if (counter>14) counter=0;
        }
}

void uart_communication()
{
        while (1)
        {

                if (Serial.isRxEnabled() && Serial.available())
                {
                        serialEvent();
                }
                else
                {
                        Serial.write(0XA5);
                        Serial.write(0X03);    //初始化,连续输出模式
                        Serial.write(0XA8);    //初始化,连续输出模式
                        delay(1000);
                }
                if (sign_bme && sign_max)
                        break;
        }

        uint16_t data_16[2]={0};

        Bme.Temp=(Re_buf_bme[4]<<8)|Re_buf_bme[5];
        data_16[0]=(Re_buf_bme[6]<<8)|Re_buf_bme[7];
        data_16[1]=(Re_buf_bme[8]<<8)|Re_buf_bme[9];
        Bme.P=(((uint32_t)data_16[0])<<16)|data_16[1];
        Bme.Hum=(Re_buf_bme[10]<<8)|Re_buf_bme[11];
        /* Bme.Alt=(Re_buf_bme[12]<<8)|Re_buf_bme[13]; */
        Lux=(Re_buf_max[4]<<24)|(Re_buf_max[5]<<16)|(Re_buf_max[6]<<8)|Re_buf_max[7];

        /* Serial.println(""); */
        /* Serial.print("Sensor: "); */
        /* Serial.println(Sensor_name); */
        /* Serial.print("Temp: "); */
        /* Serial.print( (float)Bme.Temp/100); */
        /* Serial.print(" Press: "); */
        /* Serial.print( ((float)Bme.P)/100); */
        /* Serial.print(" Hum: "); */
        /* Serial.print( (float)Bme.Hum/100); */
        /* /\* Serial.print(" ALT: "); *\/ */
        /* /\* Serial.print(Bme.Alt); *\/ */
        /* Serial.print(" Lux: "); */
        /* Serial.print((float)Lux/100); */
        /* Serial.println(""); */

        sign_max=0;
        sign_bme=0;
}

void http_post()
{
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

                String fwUrlUpload = String( fwUrlBase );
                http.begin(fwUrlUpload); //Specify request destination
                /* http.addHeader("Content-Type", "text/plain"); //Specify content-type header */
                http.addHeader("Content-Type", "application/x-www-form-urlencoded");

                dtostrf((float)Bme.Temp/100, 6, 2, Temp_string);  // 相當於 %6.2f
                dtostrf((float)Bme.Hum/100, 6, 2, Hum_string);  // 相當於 %6.2f
                dtostrf((float)Bme.P/100, 6, 2, P_string);  // 相當於 %6.2f
                /* dtostrf((float)Bme.Alt, 6, 2, Alt_string);  // 相當於 %6.2f */
                dtostrf((float)Lux/100, 6, 2, Lux_string);

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
        Serial.begin(9600);
        delay(10);

        pinMode(Sensor_vcc_pin, OUTPUT);
        pinMode(Sensor_gnd_pin, OUTPUT);

        digitalWrite(Sensor_vcc_pin, HIGH);
        digitalWrite(Sensor_gnd_pin, LOW);

        EEPROM.begin(512);
        init_wifi_state();
        // turn off Wifi and disable features we do not want.
        WiFi.mode(WIFI_OFF);
        WiFi.disconnect(true);
        WiFi.setAutoConnect(false);
        WiFi.stopSmartConfig();

        pinMode(16, WAKEUP_PULLUP);


        /* Serial.write(0XA5); */
        /* Serial.write(0X03);    //初始化,连续输出模式 */
        /* Serial.write(0XA8);    //初始化,连续输出模式 */
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
                                /* Serial.print("."); */
                                delay(100);
                        }

                        s.connect_state = CONNECT_STATE_CONNECTED;
                        put_wifi_state(&s);
                }
        } break;

        case CONNECT_STATE_CONNECTED: {
                /* Serial.println("We're connected, do something with wifi.."); */
                delay(100);

                digitalWrite(Sensor_vcc_pin, HIGH);
                digitalWrite(Sensor_gnd_pin, LOW);

                delay(500);

                uart_communication();
                http_post();

                delay(100);

                checkForUpdates();

                delay(100);

                /* Serial.print("going to deep sleep, num_seconds="); */
                /* Serial.println(num_seconds_to_sleep); */
                s.connect_state = CONNECT_STATE_PREPARE;
                put_wifi_state(&s);
                ESP.deepSleep(num_seconds_to_sleep * 1000000, RF_DISABLED);
        } break;
        }
}
