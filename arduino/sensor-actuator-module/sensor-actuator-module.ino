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

//#include <ESP8266WiFi.h>

#include <SoftwareSerial.h>
#include "dht.h"

const String SENSOR_ID = "fire-fighting-sensor-" + random(10);

const int IDLE_SUCCESS_CONNECTION_STATUS_1 = 1;
const int IDLE_SUCCESS_CONNECTION_STATUS_2 = 51; 


// The Ports of Pins in use, in this sensor's node:

// The Analog Pin
// on the Arduino that the positive lead of
// Flame Sensor is attached
// for Flames' detection and measures
const int FLAME_PIN = A0;

// The Digital Pin
// on the Arduino that the positive lead of 
// Piezo Buzzer is attached
const int PIEZO_PIN = 6;

// The Digital Pin
// on the Arduino that the positive lead of
// DHT Sensor is attached
// for Humidity and Temperature measures
const int DHT11_PIN = 7;

// The Digital Pin
// on the Arduino that the positive lead of
// RGB LED is attached
const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

const int temperatureDHTYellowAlertThreshold = 35;
const int temperatureDHTOrangeAlertThreshold = 40;
const int temperatureDHTRedAlertThreshold = 45;

dht DHT;

// RX (Receive) = 2, TX (Transmit) = 3
SoftwareSerial fireFightingSensorSerial(2, 3);

int lastFlameSensorMeasuredValue = 0;
int lastTemperatureDHTMeasuredValue = 0;
int lastHumidityDHTMeasuredValue = 0;

int flameSensorThreshold;

void setup() {
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  Serial.begin(9600);
  fireFightingSensorSerial.begin(115200);
  
  lastFlameSensorMeasuredValue = analogRead(FLAME_PIN);

  if(lastFlameSensorMeasuredValue == 0) {
    flameSensorThreshold = 20;
  }
  else {
    flameSensorThreshold = 120;
  }
    
  DHT.read11(DHT11_PIN);
  lastTemperatureDHTMeasuredValue = DHT.temperature;
  lastHumidityDHTMeasuredValue = DHT.humidity;

}

void loop() {

  if(fireFightingSensorSerial.available()) {
    delay(600);

    tryToDetectFire();
    
    byte byteReceived = fireFightingSensorSerial.read();
    
    if( (byteReceived == IDLE_SUCCESS_CONNECTION_STATUS_1) || (byteReceived == IDLE_SUCCESS_CONNECTION_STATUS_2) ) {
      performMeasurementsOrReadings();
    }
    else {
      // Check WiFi connection status
      connectingWiFiState();
    }
  }

}

void tryToDetectFire() {
  
  int flameSensorMeasuredValue = analogRead(FLAME_PIN);
 
  while(flameSensorMeasuredValue > flameSensorThreshold) {
    Serial.println("FIRE ALERT!!! MAYDAY, MAYDAY, MAYDAY!!!");
  
    Serial.print("Current Data Measured: [Flame Intensity = ");
    Serial.print(flameSensorMeasuredValue);
    Serial.println("]");
    
    fireAlert();
    flameSensorMeasuredValue = analogRead(FLAME_PIN);
  }
  
  turnOffAlert();

}

void performMeasurementsOrReadings() {
   
  int checkDHTMeasuredValue = DHT.read11(DHT11_PIN);
  int temperatureDHTMeasuredValue = DHT.temperature;
  int humidityDHTMeasuredValue = DHT.humidity;

  sendMeasurementOrReading(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
  
  if(( DHT.temperature >= temperatureDHTRedAlertThreshold) || ( (lastTemperatureDHTMeasuredValue >= temperatureDHTRedAlertThreshold) && ((lastTemperatureDHTMeasuredValue + DHT.temperature) < 0) ) ) {
   Serial.println("RED ALERT - IMMINENT OCCURENCE OF FIRE ALERT!!!");
   imminentOccurrenceFireAlert();
  }
  else if(( DHT.temperature >= temperatureDHTOrangeAlertThreshold) || ( (lastTemperatureDHTMeasuredValue >= temperatureDHTOrangeAlertThreshold) && ((lastTemperatureDHTMeasuredValue + DHT.temperature) < 0) ) ) {
    Serial.println("ORANGE ALERT - VERY POSSIBLE OCCURENCE OF FIRE ALERT!!!");
    veryPossibleOccurrenceFireAlert();
  }
  else if(( DHT.temperature >= temperatureDHTYellowAlertThreshold) || ( (lastTemperatureDHTMeasuredValue >= temperatureDHTYellowAlertThreshold) && ((lastTemperatureDHTMeasuredValue + DHT.temperature) < 0) ) ) {
    Serial.println("YELLOW ALERT - POSSIBLE OCCURENCE OF FIRE ALERT!!!");
    possibleOccurrenceFireAlert();
  }
  else {
    Serial.println("IDLE STATUS - READING...");
    IdleState();
  }

  // Just for debug
  Serial.print("Current Data Measured: [Temperature = ");
  Serial.print(temperatureDHTMeasuredValue);
  Serial.print("; Humidity = ");
  Serial.print(humidityDHTMeasuredValue);
  Serial.println("]");

  lastTemperatureDHTMeasuredValue = temperatureDHTMeasuredValue;
  lastHumidityDHTMeasuredValue = humidityDHTMeasuredValue;

  delay(200);

}

// TODO
void sendFireAlert(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  String measurementReadingToSend = "Data Measurement/Reading: [" + String(temperatureDHTMeasuredValue) + " , " + String(humidityDHTMeasuredValue) + "]";

  Serial.print("SENDING MEASUREMENT/READING...");
  
  sendDataStringToWiFiModule(1, measurementReadingToSend);

}

void sendMeasurementOrReading(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  String measurementReadingToSend = "Data Measurement/Reading: [" + String(temperatureDHTMeasuredValue) + " , " + String(humidityDHTMeasuredValue) + "]";

  Serial.print("SENDING MEASUREMENT/READING...");
  
  sendDataStringToWiFiModule(1, measurementReadingToSend);

}

void sendDataStringToWiFiModule(int messageType, String stringData) {

  fireFightingSensorSerial.write(messageType);
  
  for(int i = 0; i < stringData.length(); i++) {
    fireFightingSensorSerial.write(stringData[i]);
  }
  
}

void connectingWiFiState() {
  
  // Blue (turn just the BLUE LED on):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, HIGH);

  tone(PIEZO_PIN, 1000, 10);

  delay(50);

  // Off (all LEDs off):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
    
  tone(PIEZO_PIN, 1000, 10);

  delay(50);

}

void IdleState() {
  
  // Green (turn just the GREEN LED on):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);

  tone(PIEZO_PIN, 100, 100);

  delay(2000);

  // Off (all LEDs off):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
    
  tone(PIEZO_PIN, 100, 100);

  delay(2000);

}

void possibleOccurrenceFireAlert() {

  // Yellow (turn just the GREEN and RED LED on):
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);

  tone(PIEZO_PIN, 100, 100);

  delay(600);

  // Off (all LEDs off):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
    
  tone(PIEZO_PIN, 100, 100);

  delay(600);

}

void veryPossibleOccurrenceFireAlert() {

  // Orange (turn just the GREEN and RED LED on):
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);

  analogWrite(RED_PIN, 255);
  analogWrite(GREEN_PIN, 30);
  analogWrite(BLUE_PIN, 0);
  
  tone(PIEZO_PIN, 100, 100);

  delay(200);

  // Off (all LEDs off):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);
    
  tone(PIEZO_PIN, 100, 100);

  delay(200);

}

void imminentOccurrenceFireAlert() {
  
  // Red (turn just the RED LED on):
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

  tone(PIEZO_PIN, 100, 100);

  delay(100);

  // Off (all LEDs off):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
    
  tone(PIEZO_PIN, 100, 100);

  delay(100);

}

void fireAlert() {

  // Red (turn just the red LED on):
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

  tone(PIEZO_PIN, 10, 10000);

}

void turnOffAlert() {
  
  // Off (all LEDs off):
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

}
