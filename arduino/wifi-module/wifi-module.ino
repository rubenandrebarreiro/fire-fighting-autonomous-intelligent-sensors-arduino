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
//#include <SPI.h>
#include <SoftwareSerial.h>

// Some available Networks to test:
const String OPEN_FCT_NETWORK_NAME = "OpenFCT";
const String OPEN_FCT_NETWORK_PASSWORD = "";

const String RUBEN_HOME_NETWORK_NAME = "MEO-50FDD6";
const String RUBEN_HOME_NETWORK_PASSWORD = "6F137F7E95";

const String RUBEN_MOBILE_HOTSPOT_NAME = "";
const String RUBEN_MOBILE_HOTSPOT_PASSWORD = "";

// The Web Server
const String WEB_SERVER = "https://firefighting-240516.appspot.com";

// The Endpoint to register a sensor, using a @POST
const String endpointRegister[] = "/register/sensor";

// The Endpoint to send reads/measuremenrs, using @PUT
const String endpointRead[] = "/sensor/read";

// The Endpoint to send alerts, using @POST
const String endpointAlert[] = "/sensor/alert";

// Messages to display in the Serial (115200 baud)
const String MSG_CONNECTING_TO_THE_NETWORK = "Connecting to the Network: '";
const String MSG_SUCCESSFUL_CONNECTED_TO_THE_NETWORK = "Successful connected to the Network: '";
const String MSG_CONNECTED_IP_ADDRESS = "Connected, IP address: ";
const String MSG_RECONNECTING_TO_THE_NETWORK = "Reconnecting to the Network: '";

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
   
  if( (WiFi.status() == WL_CONNECTED) ) {
    Serial.write(1);

    String a = Serial.read();
    Serial.print(a);
  }
  else {
    if( (WiFi.status() == WL_NO_SSID_AVAIL) || (WiFi.status() == WL_DISCONNECTED) || (WiFi.status() == WL_CONNECTION_LOST) ) {
      retryWiFiConnection();
    } 
  }
  
}

void tryWiFiConnection() {
  
  Serial.println();
  
  Serial.print(MSG_CONNECTING_TO_THE_NETWORK);
  Serial.print(rubenHomeNetworkName);
  Serial.println("'...");

  Serial.println();
  
  while(WiFi.status() != WL_CONNECTED) {
    Serial.write(0);
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  
  Serial.print(MSG_SUCCESSFUL_CONNECTED_TO_THE_NETWORK);
  Serial.print(rubenHomeNetworkName);
  Serial.print("'!!!");
  Serial.println();
  
  Serial.print(MSG_CONNECTED_IP_ADDRESS);
  Serial.print("- ");
  Serial.println(WiFi.localIP());
  
}

void retryWiFiConnection() {
  
  Serial.println();
  
  Serial.print(MSG_RECONNECTING_TO_THE_NETWORK);
  Serial.print(rubenHomeNetworkName);
  Serial.println("'...");

  Serial.println();

  while( (WiFi.status() == WL_CONNECTION_LOST) || (WiFi.status() == WL_NO_SSID_AVAIL) || (WiFi.status() == WL_DISCONNECTED) ) {
   Serial.write(0);
   delay(1000);
   Serial.print(".");
  }
  
  Serial.println();
  
  Serial.print(MSG_SUCCESSFUL_CONNECTED_TO_THE_NETWORK);
  Serial.print(rubenHomeNetworkName);
  Serial.print("'!!!");
  Serial.println();
 
  Serial.print(MSG_CONNECTED_IP_ADDRESS);
  Serial.print("- ");
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
