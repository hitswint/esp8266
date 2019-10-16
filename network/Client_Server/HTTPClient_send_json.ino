#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

void setup() {

        Serial.begin(115200);                            //Serial connection
        WiFi.begin("YourNetworkName", "YourPassword");   //WiFi connection

        while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion

                delay(500);
                Serial.println("Waiting for connection");

        }

}

void loop() {

        if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
                /* First, we declare an object of class StaticJsonBuffer for encoding the message. We need to specify a size that is big enough for the structure we are going to create. In this case, we will specify 300 bytes, which is more than enough. */
                /* Then, we get a reference to a JsonObject from the StaticJsonBuffer object we created, by calling the createObject method. */
                StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
                JsonObject& JSONencoder = JSONbuffer.createObject();

                /* Now, we already have what we need to start specifying the structure of our JSON message. We will use the dummy JSON example bellow which represents a possible data structure for measurements of a temperature IoT device. */
                /* { */
                /*         "sensorType": "Temperature", */
                /*                 "values": [20, 21, 23], */
                /*                 "timestamps": ["10:10", "10:20", "10:30"] */
                /*                 } */
                /* To create simple name/value pairs in the JSON structure, we use the subscript operator or, in other words, we use square brackets on the reference of the JsonObject. So, as seen bellow, we use this functionality to specify the “sensorType” attribute as “Temperature”. */
                JSONencoder["sensorType"] = "Temperature";

                /* Finally, we need to specify our arrays of measurement values and corresponding timestamps. To create arrays, we call the createNestedArray method on the JsonObject reference. Then we use the add method of the JsonArray reference to add the values we want on our array. */
                JsonArray& values = JSONencoder.createNestedArray("values"); //JSON array
                values.add(20); //Add value to array
                values.add(21); //Add value to array
                values.add(23); //Add value to array

                JsonArray& timestamps = JSONencoder.createNestedArray("timestamps"); //JSON array
                timestamps.add("10:10"); //Add value to array
                timestamps.add("10:20"); //Add value to array
                timestamps.add("10:30"); //Add value to array

                /* We will now need to print the JSON structure we defined to a string, so we can send it in our request to the server. To do so, we just need to declare a char buffer to hold our data and call the prettyPrintTo method, as indicated bellow. Note that we could have used another method, called printTo, which prints the JSON content with the less overhead possible, but also less easy for a person to read. */
                char JSONmessageBuffer[300];
                JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
                Serial.println(JSONmessageBuffer);

                /* In order for our request to be correctly interpreted by the server, we also specify the content-type as application/json. We do it by calling the addHeader method on the HTTPClient object. */
                HTTPClient http;    //Declare object of class HTTPClient

                http.begin("http://anteph.pythonanywhere.com/postjson");      //Specify request destination
                http.addHeader("Content-Type", "application/json");  //Specify content-type header

                /* Finally, we get the response HTTP code and the response message and print both to the serial port. We should get a 200 HTTP Code if everything is working correctly, and the response should be the message defined in the Python code. After that, we close the connection with the end method. */
                int httpCode = http.POST(JSONmessageBuffer);   //Send the request
                String payload = http.getString();                                        //Get the response payload

                Serial.println(httpCode);   //Print HTTP return code
                Serial.println(payload);    //Print request response payload

                http.end();  //Close connection

        } else {

                Serial.println("Error in WiFi connection");

        }

        delay(30000);  //Send a request every 30 seconds

}
