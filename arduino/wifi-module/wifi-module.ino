/*
 * 
 * Fire Fighting - Autonomous Intelligent Sensors (Arduino Component) 
 * 
 *  Mobile and Pervasive Computing (Practical Lab/Work Assignment)
 *  @ FCT NOVA (Faculty of Sciences and Technology of New University of Lisbon)
 * 
 * @authors:
 * - Bernardo Palma Albergaria, bd.albergaria@campus.fct.unl.pt (student no. 41931)
 * - Eduarado Manuel Silva, emf.silva@campus.fct.unl.pt (student no. 41798)
 * - Ruben Andre Barreiro, r.barreiro@campus.fct.unl.pt (student no. 42648)
 * 
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <SoftwareSerial.h>

// Some available Networks to test:
const String OPEN_FCT_NETWORK_NAME = "OpenFCT";
const String OPEN_FCT_NETWORK_PASSWORD = "";

const String RUBEN_HOME_NETWORK_NAME = "MEO-50FDD6";
const String RUBEN_HOME_NETWORK_PASSWORD = "6F137F7E95";

const String RUBEN_MOBILE_HOTSPOT_NAME = "";
const String RUBEN_MOBILE_HOTSPOT_PASSWORD = "";

// The Web Server
const char WEB_SERVER[] = "https://firefighting-240516.appspot.com";

// The Endpoint to register a sensor, using a @POST
const char ENDPOINT_REGISTER[] = "/register/sensor";

// The Endpoint to send reads/measuremenrs, using @PUT
const char ENDPOINT_READ[] = "/sensor/read";

// The Endpoint to send alerts, using @POST
const char ENDPOINT_ALERT[] = "/sensor/alert";

// The HTTP Status Codes
/*const int httpStatusCodeOK = 200;
const int httpStatusCodeNotFound = 404;
const int httpStatusCodeInternalServerError = 500;*/

#define TIME_HEADER "T"   // Header tag for serial time sync message
#define TIME_REQUEST 7    // ASCII bell character requests a time sync message 

void setup() {

  Serial.begin(115200);

  // The delay (5 seconds) needed
  // before calling the WiFi.begin method
  delay(5000);
  WiFi.begin(RUBEN_HOME_NETWORK_NAME, RUBEN_HOME_NETWORK_PASSWORD);
  
  tryWiFiConnection();

}

void loop() {
  
  // Check the Network status once every 6 seconds:
  delay(6000);
   
  if( WiFi.status() == WL_CONNECTED ) {
    Serial.write(1);

    WiFiClient client;
  
  const int httpPort = 80;

  // Specify HTTP request destination
  char serverPath[60];
  sprintf(serverPath, "%s", WEB_SERVER);

  char serverPathHost[80];
  sprintf(serverPathHost, "Host: %s", serverPath);

  char serverPathPost[80];
  sprintf(serverPathPost, "POST %s HTTP/1.0", ENDPOINT_REGISTER);
  
  if (!client.connect(serverPath, httpPort)) {
    Serial.println("Connection failed...");
    return;
  }
  
    String messageReceived = Serial.readString();

    Serial.println();
    
    char messageReceivedBuffer[60];

    messageReceived.toCharArray(messageReceivedBuffer, 60);
    
    if(messageReceivedBuffer[0] == 'M') {

      // REGISTER ITSELF
      if(messageReceivedBuffer[1] == '0') {
       char sensorID[50];
       
       sscanf(messageReceivedBuffer, "M0 - {[ %s ]}",
          &sensorID);

       Serial.println("Setting up the sensor with the ID:");
       Serial.println(sensorID);

       // Declare object of class HTTPClient
       /*HTTPClient http;

       Serial.print("Sending a HTTP POST request to: ");
       Serial.println(serverPath);
       
       http.begin(serverPath);
       http.header("POST / HTTP/1.0");

       
       //http.header(serverPathHost);
       */
       //http.header("Accept: */*");
       //http.header("Content-type: text/plain");

       // Send the HTTP request
       /*int httpCode = http.POST(sensorID);
       String payload = http.getString(); 

       // Print HTTP request return code
       Serial.println(httpCode);

       // Print HTTP request response payload
       Serial.println(payload);
       
       http.end();*/
       
       
        client.println(serverPathPost);
   client.println(serverPathHost);
   //client.println("Accept: */*");
   client.print("Content-Length: ");
   client.println(sizeof(sensorID));
   client.println("Content-Type: text/plain");
   client.println();
   client.print(sensorID);
   delay(1000); // Can be changed

  Serial.println("HTTP Response:");
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
      
    // I will neeed to filter the headers, but first I want to be able to write to TempGraph anything

    }

  
  if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
  }
      }
      
      // IDLE ALERT
      if(messageReceivedBuffer[1] == '1') {

       int flameSensorMeasuredValue;
       int temperatureDHTMeasuredValue;
       int humidityDHTMeasuredValue;
       
       sscanf(messageReceivedBuffer, "M1 - {[Flame's IR = %d; Temperature = %d; Humidity = %d}}",
          &flameSensorMeasuredValue, &temperatureDHTMeasuredValue, &humidityDHTMeasuredValue);

       Serial.println("Received an IDLE ALERT...");
       Serial.print("INFO: {Flame's IR = ");
       Serial.print(flameSensorMeasuredValue);
       Serial.print("; Temperature = ");
       Serial.print(temperatureDHTMeasuredValue);
       Serial.print("; Humidity = ");
       Serial.print(humidityDHTMeasuredValue);
       Serial.println("}");

       
      }

      // YELLOW ALERT
      if(messageReceivedBuffer[1] == '2') {

       int temperatureDHTMeasuredValue;
       int humidityDHTMeasuredValue;
       
       sscanf(messageReceivedBuffer, "M2 - {[Temperature = %d; Humidity = %d}}",
          &temperatureDHTMeasuredValue, &humidityDHTMeasuredValue);
          
       Serial.println(temperatureDHTMeasuredValue);
       Serial.println(humidityDHTMeasuredValue);
      }

      // ORANGE ALERT
      if(messageReceivedBuffer[1] == '3') {

       int temperatureDHTMeasuredValue;
       int humidityDHTMeasuredValue;
       
       sscanf(messageReceivedBuffer, "M3 - {[Temperature = %d; Humidity = %d}}",
          &temperatureDHTMeasuredValue, &humidityDHTMeasuredValue);
          
       Serial.println(temperatureDHTMeasuredValue);
       Serial.println(humidityDHTMeasuredValue);
      }

      // RED ALERT
      if(messageReceivedBuffer[1] == '4') {

       int temperatureDHTMeasuredValue;
       int humidityDHTMeasuredValue;
       
       sscanf(messageReceivedBuffer, "M4 - {[Temperature = %d; Humidity = %d}}",
          &temperatureDHTMeasuredValue, &humidityDHTMeasuredValue);
          
       Serial.println(temperatureDHTMeasuredValue);
       Serial.println(humidityDHTMeasuredValue);
      }

      // FIRE ALERT
      if(messageReceivedBuffer[1] == '5') {
       int flameSensorMeasuredValue;
       
       sscanf(messageReceivedBuffer, "M5 - {[Flame's IR = %d]}",
          &flameSensorMeasuredValue);
          
       Serial.println(flameSensorMeasuredValue);
      }
    }  
  }
  else {
    if( (WiFi.status() == WL_NO_SSID_AVAIL) || (WiFi.status() == WL_DISCONNECTED) || (WiFi.status() == WL_CONNECTION_LOST) ) {
      retryWiFiConnection();
    } 
  }
  
}

void tryWiFiConnection() {
  
  Serial.println();
  
  Serial.print("CONNECTING TO THE WIFI NETWORK: '");
  Serial.print(RUBEN_HOME_NETWORK_NAME);
  Serial.println("'...");

  Serial.println();
  
  while(WiFi.status() != WL_CONNECTED) {
    Serial.write(0);
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  
  Serial.print("CONNECTED SUCCESSFUL TO THE WIFI NETWORK: '");
  Serial.print(RUBEN_HOME_NETWORK_NAME);
  Serial.print("'!!!");
  Serial.println();
  
  Serial.print("Connected to the Local IP Address: ");
  Serial.println(WiFi.localIP());
  
}

void retryWiFiConnection() {
  
  Serial.println();
  
  Serial.print("RECONNECTING TO THE WIFI NETWORK: '");
  Serial.print(RUBEN_HOME_NETWORK_NAME);
  Serial.println("'...");

  Serial.println();

  while( (WiFi.status() == WL_CONNECTION_LOST) || (WiFi.status() == WL_NO_SSID_AVAIL) || (WiFi.status() == WL_DISCONNECTED) ) {
   Serial.write(0);
   delay(1000);
   Serial.print(".");
  }
  
  Serial.println();
  
  Serial.print("CONNECTED SUCCESSFUL TO THE WIFI NETWORK: '");
  Serial.print(RUBEN_HOME_NETWORK_NAME);
  Serial.print("'!!!");
  Serial.println();
 
  Serial.print("Connected to the Local IP Address: ");
  Serial.println(WiFi.localIP());
}



     /*if(WiFi.status()== WL_CONNECTED) {

         HTTPClient httpClient;   

         if(firstReading) {
          setupSensorData();
          firstReading = false;
         }
         else {
          updateSensorData();
         }

         
         // Start the resources from the HTTP Client
         // Specify the IP Address for the destination for the HTTP request
         httpClient.begin("localhost:8080/sensor/"); // TODO mudar

         // Specify the content-type header for th HTTP request
         httpClient.addHeader("Content-Type", "text/plain");
         //httpClient.addHeader("Content-Type", "application/json");

         // Send the actual HTTP POST request
         int httpResponseCode = httpClient.POST("POSTING from ESP32"); // TODO mudar

         // Verify the HTTP Status Code of the response to the HTTP request
         if(httpResponseCode > 0) {
            
            // Get the response to the HTTP request
            String response = http.getString();

            // The HTTP Status Code of the response to the HTTP request:
            // - 200 OK 
            if(httpResponseCode == httpStatusCodeOK) {
              // TODO
            }
            // The HTTP Status Code of the response to the HTTP request:
            // - 404 OK 
            else if(httpResponseCode == httpStatusCodeNotFound) {
              // TODO
            }
            // The HTTP Status Code of the response to the HTTP request:
            // - 500 OK 
            else if(httpResponseCode == httpStatusCodeInternalServerError) {
              // TODO
            }
            
            Serial.println(httpResponseCode);   //Print return code
            Serial.println(response);           //Print request answer
        }
        else {
          Serial.print("Error on sending the HTTP POST request: ");
          Serial.println(httpResponseCode);
        }

        // Free and terminate the resources from the HTTP Client
        httpClient.end();
  }
  else{
    Serial.println("Error in WiFi connection!!!");   
  }
  
  // Try to measure the sensor's measures and reading parameters and
  // send the HTTP POST request with that reading values,
  // continuously for every 10 seconds
  delay(10000);*/
