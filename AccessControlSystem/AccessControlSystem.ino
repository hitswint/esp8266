#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WebSocketClient.h>
#include <ESP8266httpUpdate.h>

const int FW_VERSION = 18040916;
const char* fwUrlBase = "http://47.94.151.140/";

int Pin_un_lock = 14;
int Pin_spk = 15;

const char* ssid = "ssid";
const char* password = "password";

char path[] = "/";
char host[] = "47.94.151.140";

WebSocketClient webSocketClient;
WiFiClient client;

// ESP8266 Timer Example
extern "C" {
#include "user_interface.h"
}

os_timer_t myTimer;

bool tickOccured;

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
  os_timer_arm(&myTimer, 60000, true);
}

void checkForUpdates()
{
  String fwURL = String( fwUrlBase );
  fwURL.concat( "Fota/" );
  String fwVersionURL = fwURL;
  fwVersionURL.concat( "image.version" );
  HTTPClient httpClient;
  httpClient.begin( fwVersionURL );
  int httpCode = httpClient.GET();
  if( httpCode == 200 ) {
    String newFWVersion = httpClient.getString();
    int newVersion = newFWVersion.toInt();
    if( newVersion > FW_VERSION ) {
      String fwImageURL = fwURL;
      fwImageURL.concat( "image.bin" );
      t_httpUpdate_return ret = ESPhttpUpdate.update( fwImageURL );
    }
  }
  httpClient.end();
}

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(Pin_un_lock, OUTPUT);
  pinMode(Pin_spk, OUTPUT);
  digitalWrite(Pin_un_lock, LOW);
  digitalWrite(Pin_spk, LOW);
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    /* Serial.print("."); */
  }
  delay(1000);

  tickOccured = false;
  user_init();

  client.connect(host, 80);
  /* if (client.connect(host, 80)) { */
  /*   Serial.println("Connected"); */
  /* } else { */
  /*   Serial.println("Connection failed."); */
  /* } */
  webSocketClient.path = path;
  webSocketClient.host = host;
  webSocketClient.handshake(client);
  /* if (webSocketClient.handshake(client)) { */
  /*   Serial.println("Handshake successful"); */
  /* } else { */
  /*   Serial.println("Handshake failed."); */
  /* } */
}

void loop() {
  if (client.connected()) {
    if (tickOccured == true)
    {
      /* Serial.println("Tick Occurred"); */
      /* 每隔一段时间向服务器发送数据以保持连接。 */
      webSocketClient.sendData("Info to be echoed back");
      tickOccured = false;
    }
    /* Serial.println("Connected"); */
    String data_received;
    webSocketClient.getData(data_received);
    /* Serial.println(data_received); */
    if (data_received.length() > 0) {
      if (data_received == "1") {
        /* Serial.println("un_lock"); */
        digitalWrite(Pin_un_lock, HIGH);
        delay(500);
        digitalWrite(Pin_un_lock, LOW);
      } else if (data_received == "2") {
        /* Serial.println("spk"); */
        digitalWrite(Pin_spk, HIGH);
        delay(500);
        digitalWrite(Pin_spk, LOW);
      } else if (data_received == "3") {
        /* Serial.println("spk"); */
        digitalWrite(Pin_spk, HIGH);
        delay(500);
        digitalWrite(Pin_spk, LOW);
        delay(2000);
        digitalWrite(Pin_un_lock, HIGH);
        delay(500);
        digitalWrite(Pin_un_lock, LOW);
      } else if (data_received == "0") {
        /* Serial.println("update"); */
        checkForUpdates();
      } else {
        webSocketClient.sendData("Info to be echoed back");
      }
    }
  } else {
    client.connect(host, 80);
    webSocketClient.handshake(client);
    /* Serial.println("Client disconnected."); */
  }

  delay(1000);
}
