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

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Some available Networks to test:

const String OPEN_FCT_NETWORK_NAME = "OpenFCT";
const String OPEN_FCT_NETWORK_PASSWORD = "";

const String RUBEN_HOME_NETWORK_NAME = "MEO-50FDD6";
const String RUBEN_HOME_NETWORK_PASSWORD = "6F137F7E95";

//const String RUBEN_MOBILE_HOTSPOT_NAME = "";
//const String RUBEN_MOBILE_HOTSPOT_PASSWORD = "";

// The Sensor's ID
char sensorID[50];
       
void setup() {
  Serial.begin(115200);

  // The delay (5 seconds) needed
  // before calling the WiFi.begin method
  delay(5000);
  WiFi.begin(OPEN_FCT_NETWORK_NAME, OPEN_FCT_NETWORK_PASSWORD);
  
  tryWiFiConnection();
}

void loop() {
  
  // Check the Network status once every 6 seconds:
  delay(6000);
   
  if( WiFi.status() == WL_CONNECTED ) {
    Serial.write(1);
  
    String messageReceived = Serial.readString();

    Serial.println();

    // Declare object of class HTTPClient
    HTTPClient http;
       
    char messageReceivedBuffer[60];
    messageReceived.toCharArray(messageReceivedBuffer, 60);
    
    if(messageReceivedBuffer[0] == 'M') {

      // REGISTER ITSELF
      if(messageReceivedBuffer[1] == '0') {
       
       sscanf(messageReceivedBuffer, "M0 - {[ %s ]}",
          &sensorID);

       Serial.println("Setting up the sensor with the ID:");
       Serial.println(sensorID);       
       
       Serial.print("Sending a HTTP POST request to: ");
       Serial.println("- http://firefighting-240516.appspot.com/register/sensor");
       
       http.begin("http://firefighting-240516.appspot.com/register/sensor");
       http.addHeader("Content-Type", "text/plain");
       
       // Send the HTTP request
       int httpCode = http.POST(sensorID);
       String payload = http.getString(); 

       if(httpCode == 200) {
        Serial.println("HTTP Request Response Status Code: 200");
        Serial.println("Sensor registered itself with success!!!");
       }
       else if(httpCode == 400) {
        Serial.println("HTTP Request Response Status Code: 400");
        Serial.println("Sensor already registered!!!");
       }
       else {
        Serial.print("HTTP Request Response Status Code: ");
        Serial.println(httpCode);
        Serial.println("Some error occurred with the HTTP request!!!");
       }

       Serial.println("Payload of the HTTP Request Response:");
       Serial.println(payload);

       Serial.println();
       
       http.end();
      }
      
      // MEASUREMENT/READING
      if(messageReceivedBuffer[1] == '1') {
        int temperatureDHTMeasuredValue;
        int humidityDHTMeasuredValue;
         
        sscanf(messageReceivedBuffer, "M1 - {[Temperature = %d; Humidity = %d}}",
               &temperatureDHTMeasuredValue, &humidityDHTMeasuredValue);
  
        Serial.println("Received a Measurement/Reading from the Sensor...");
        Serial.print("INFO: {Temperature = ");
        Serial.print(temperatureDHTMeasuredValue);
        Serial.print("; Humidity = ");
        Serial.print(humidityDHTMeasuredValue);
        Serial.println("}");

        Serial.println("Measurement/Reading from the Sensor's ID:");
        Serial.println(sensorID);       
       
        Serial.print("Sending a HTTP PUT request to: ");
        Serial.println("- http://firefighting-240516.appspot.com/sensor/read");
       
        http.begin("http://firefighting-240516.appspot.com/sensor/read");
        http.addHeader("Content-Type", "text/plain");

        char sensorMeasurement[80];
        sprintf(sensorMeasurement, "%s %d %d", sensorID, temperatureDHTMeasuredValue, humidityDHTMeasuredValue); 
        
        // Send the HTTP request
        int httpCode = http.PUT(sensorMeasurement);
        String payload = http.getString();
        
       if(httpCode == 200) {
        Serial.println("HTTP Request Response Status Code: 200");
        Serial.println("Sensor's Measurement/Reading registered with success!!!");
       }
       else if(httpCode == 400) {
        Serial.println("HTTP Request Response Status Code: 400");
        Serial.println("Sensor don't registered or don't exist!!!");
       }
       else {
        Serial.print("HTTP Request Response Status Code: ");
        Serial.println(httpCode);
        Serial.println("Some error occurred with the HTTP request!!!");
       }

       Serial.println("Payload of the HTTP Request Response:");
       Serial.println(payload);

       Serial.println();
       
       http.end();
      }
  
      // FIRE ALERT
      if(messageReceivedBuffer[1] == '2') {
        int flameSensorMeasuredValue;
         
        sscanf(messageReceivedBuffer, "M2 - {[Flame's IR = %d]}",
               &flameSensorMeasuredValue);
               
        Serial.println("Received a Fire Alert from the Sensor...");
        Serial.print("INFO: {Flame's IR = ");
        Serial.print(flameSensorMeasuredValue);
        Serial.println("}");

        Serial.println("Fire Alert from the Sensor's ID:");
        Serial.println(sensorID);       
       
        Serial.print("Sending a HTTP POST request to: ");
        Serial.println("- http://firefighting-240516.appspot.com/sensor/alert");
       
        http.begin("http://firefighting-240516.appspot.com/sensor/alert");
        http.addHeader("Content-Type", "text/plain");

        char sensorFireAlert[40];
        sprintf(sensorFireAlert, "%s", sensorID); 
        
        // Send the HTTP request
        int httpCode = http.POST(sensorFireAlert);
        String payload = http.getString();
        
       if(httpCode == 200) {
        Serial.println("HTTP Request Response Status Code: 200");
        Serial.println("Sensor's Fire Alert registered with success!!!");
       }
       else if(httpCode == 400) {
        Serial.println("HTTP Request Response Status Code: 400");
        Serial.println("Sensor don't registered or don't exist!!!");
       }
       else {
        Serial.print("HTTP Request Response Status Code: ");
        Serial.println(httpCode);
        Serial.println("Some error occurred with the HTTP request!!!");
       }

       Serial.println("Payload of the HTTP Request Response:");
       Serial.println(payload);

       Serial.println();
       
       http.end();
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
