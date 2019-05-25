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

const String SENSOR_ID = "fire-fighting-sensor-00";
//const String SENSOR_ID = "fire-fighting-sensor-" + random(10);

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

const String MSG_FIRE_ALERT = "FIRE ALERT!!! MAYDAY, MAYDAY, MAYDAY!!!";

const String MSG_FIRE_ALERT_CURRENT_DATA_MEASURED_FLAME_SENSOR_1 = "FIRE ALERT - ";
const String MSG_CURRENT_DATA_MEASURED_FLAME_SENSOR_1 = "Current Data Measured: [Flame Intensity = ";
const String MSG_CURRENT_DATA_MEASURED_FLAME_SENSOR_2 = "]";

const String MSG_RED_ALERT = "RED ALERT - IMMINENT OCCURENCE OF FIRE ALERT!!!";
const String MSG_ORANGE_ALERT = "ORANGE ALERT - VERY POSSIBLE OCCURENCE OF FIRE ALERT!!!";
const String MSG_YELLOW_ALERT = "YELLOW ALERT - POSSIBLE OCCURENCE OF FIRE ALERT!!!";
const String MSG_IDLE_STATUS = "IDLE STATUS - READING...";

const String MSG_YELLOW_ALERT_CURRENT_DATA_MEASURED_DHT_SENSOR_1 = "YELLOW ALERT - ";
const String MSG_ORANGE_ALERT_CURRENT_DATA_MEASURED_DHT_SENSOR_1 = "ORANGE ALERT - ";
const String MSG_RED_ALERT_CURRENT_DATA_MEASURED_DHT_SENSOR_1 = "RED ALERT - ";

const String MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_1 = "Current Data Measured: [Temperature = ";
const String MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_2 = "; Humidity = ";
const String MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_3 = "]";

const String MSG_REGISTER_ITSELF_INFO_1 = "Sensor's Registration Info: [";
const String MSG_REGISTER_ITSELF_INFO_2 = "]";
const String MSG_REGISTER_ITSELF_INFO_3 = "SENDING SENSOR'S REGISTRATION INFO...";

const String MSG_CURRENT_FLAME_MEASUREMENT_READING_TO_SEND_1 = "Data Measurement/Reading: [";
const String MSG_CURRENT_FLAME_MEASUREMENT_READING_TO_SEND_2 = "]";
const String MSG_CURRENT_FLAME_MEASUREMENT_READING_TO_SEND_3 = "SENDING FLAME MEASUREMENT/READING...";

const String MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_1 = "Data Measurement/Reading: [";
const String MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_2 = " , ";
const String MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_3 = "]";
const String MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_4 = "SENDING DHT MEASUREMENT/READING...";

const unsigned long TIME_TO_SEND_FIRE_ALERT = 5000;
const unsigned long TIME_TO_SEND_YELLOW_ALERT = 7200000;
const unsigned long TIME_TO_SEND_ORANGE_ALERT = 3600000;
const unsigned long TIME_TO_SEND_RED_ALERT = 1800000;
const unsigned long TIME_TO_SEND_MEASUREMENT_OR_READING = 18000000;


dht DHT;

// RX (Receive) = 2, TX (Transmit) = 3
SoftwareSerial fireFightingSensorSerial(2, 3);

unsigned long timeSinceLastSendingFireAlert;
unsigned long timeSinceLastSendingYellowAlert;
unsigned long timeSinceLastSendingOrangeAlert;
unsigned long timeSinceLastSendingRedAlert;
unsigned long timeSinceLastSendingMeasurementOrReading;

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

  delay(4000);
  sendRegisterItselfInfo();

  timeSinceLastSendingFireAlert = 0;
  timeSinceLastSendingYellowAlert = 0;
  timeSinceLastSendingOrangeAlert = 0;
  timeSinceLastSendingRedAlert = 0;
  timeSinceLastSendingMeasurementOrReading = 0;
  
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
    
    Serial.println(MSG_FIRE_ALERT);
  
    Serial.print(MSG_CURRENT_DATA_MEASURED_FLAME_SENSOR_1);
    Serial.print(flameSensorMeasuredValue);
    Serial.println(MSG_CURRENT_DATA_MEASURED_FLAME_SENSOR_2);
    
    verifySendAlertMsg(0, flameSensorMeasuredValue, 0, 0);

    fireAlert();
    
    flameSensorMeasuredValue = analogRead(FLAME_PIN);
  }
  
  turnOffAlert();

}

void performMeasurementsOrReadings() {
   
  int checkDHTMeasuredValue = DHT.read11(DHT11_PIN);
  int temperatureDHTMeasuredValue = DHT.temperature;
  int humidityDHTMeasuredValue = DHT.humidity;
  
  if( (temperatureDHTMeasuredValue >= temperatureDHTRedAlertThreshold) || ( (lastTemperatureDHTMeasuredValue >= temperatureDHTRedAlertThreshold) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {
   Serial.println(MSG_RED_ALERT);

   Serial.print(MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_1);
   Serial.print(temperatureDHTMeasuredValue);
   Serial.println(MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_2);
   Serial.print(humidityDHTMeasuredValue);
   Serial.println(MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_3);

   verifySendAlertMsg(3, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
   
   imminentOccurrenceFireAlert();   
  }
  else if( (temperatureDHTMeasuredValue >= temperatureDHTOrangeAlertThreshold) || ( (lastTemperatureDHTMeasuredValue >= temperatureDHTOrangeAlertThreshold) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {
    Serial.println(MSG_ORANGE_ALERT);

    verifySendAlertMsg(2, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
    
    veryPossibleOccurrenceFireAlert();
  }
  else if( (temperatureDHTMeasuredValue >= temperatureDHTYellowAlertThreshold) || ( (lastTemperatureDHTMeasuredValue >= temperatureDHTYellowAlertThreshold) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {
    Serial.println(MSG_YELLOW_ALERT);

    verifySendAlertMsg(1, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
    
    possibleOccurrenceFireAlert();
  }
  else {
    Serial.println(MSG_IDLE_STATUS);

    verifySendAlertMsg(4, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
    
    idleState();
  }

  // Just for debug
  Serial.print(MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_1);
  Serial.print(temperatureDHTMeasuredValue);
  Serial.print(MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_2);
  Serial.print(humidityDHTMeasuredValue);
  Serial.println(MSG_CURRENT_DATA_MEASURED_DHT_SENSOR_3);

  lastTemperatureDHTMeasuredValue = temperatureDHTMeasuredValue;
  lastHumidityDHTMeasuredValue = humidityDHTMeasuredValue;

  delay(200);

}

void sendRegisterItselfInfo() {
  
  String registerItselfInfoMsgToSend = MSG_REGISTER_ITSELF_INFO_1 + SENSOR_ID + MSG_REGISTER_ITSELF_INFO_2;

  Serial.print(MSG_REGISTER_ITSELF_INFO_3);
  
  sendDataStringToWiFiModule(0, registerItselfInfoMsgToSend);

}

void verifySendAlertMsg(int alertType, int flameSensorMeasuredValue, int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {

  // FIRE ALERT
  if(alertType == 0) {
    long timeOfFireAlertOccurrence = 0;

    timeOfFireAlertOccurrence = millis() - timeSinceLastSendingFireAlert;

    if(timeOfFireAlertOccurrence > TIME_TO_SEND_FIRE_ALERT) {
      sendFireAlert(flameSensorMeasuredValue);
      timeSinceLastSendingFireAlert = millis();
    }
  }

  // YELLOW ALERT
  if(alertType == 1) {
    long timeOfYellowAlertOccurrence = 0;

    timeOfYellowAlertOccurrence = millis() - timeSinceLastSendingYellowAlert;

    if(timeOfYellowAlertOccurrence > TIME_TO_SEND_YELLOW_ALERT) {
      sendYellowAlert(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingYellowAlert = millis();
    }
  }

  // ORANGE ALERT
  if(alertType == 2) {
    long timeOfOrangeAlertOccurrence = 0;

    timeOfOrangeAlertOccurrence = millis() - timeSinceLastSendingOrangeAlert;

    if(timeOfOrangeAlertOccurrence > TIME_TO_SEND_ORANGE_ALERT) {
      sendOrangeAlert(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingOrangeAlert = millis();
    }
  }

  // RED ALERT
  if(alertType == 3) {
    long timeOfRedAlertOccurrence = 0;

    timeOfRedAlertOccurrence = millis() - timeSinceLastSendingRedAlert;

    if(timeOfRedAlertOccurrence > TIME_TO_SEND_RED_ALERT) {
      sendRedAlert(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingRedAlert = millis();
    }
  }

  // IDLE ALERT
  if(alertType == 4) {
    
  }
}

void sendFireAlert(int flameSensorMeasuredValue) {
  
  String fireAlertMsgToSend = MSG_FIRE_ALERT_CURRENT_DATA_MEASURED_FLAME_SENSOR_1 + 
                              MSG_CURRENT_FLAME_MEASUREMENT_READING_TO_SEND_1 + 
                              String(flameSensorMeasuredValue) + 
                              MSG_CURRENT_FLAME_MEASUREMENT_READING_TO_SEND_2;

  Serial.print(MSG_CURRENT_FLAME_MEASUREMENT_READING_TO_SEND_3);
  
  sendDataStringToWiFiModule(1, fireAlertMsgToSend);

}

void sendYellowAlert(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  String yellowAlertMsgToSend = MSG_YELLOW_ALERT_CURRENT_DATA_MEASURED_DHT_SENSOR_1 + 
                                MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_1 + 
                                String(temperatureDHTMeasuredValue) + 
                                MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_2 + 
                                String(humidityDHTMeasuredValue) + 
                                MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_3;

  Serial.print(MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_4);
  
  sendDataStringToWiFiModule(2, yellowAlertMsgToSend);

}

void sendOrangeAlert(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  String orangeAlertMsgToSend = MSG_ORANGE_ALERT_CURRENT_DATA_MEASURED_DHT_SENSOR_1 + 
                                MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_1 + 
                                String(temperatureDHTMeasuredValue) + 
                                MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_2 + 
                                String(humidityDHTMeasuredValue) + 
                                MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_3;

  Serial.print(MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_4);
  
  sendDataStringToWiFiModule(3, orangeAlertMsgToSend);

}


void sendRedAlert(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  String redAlertMsgToSend = MSG_RED_ALERT_CURRENT_DATA_MEASURED_DHT_SENSOR_1 + 
                                    MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_1 + 
                                    String(temperatureDHTMeasuredValue) + 
                                    MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_2 + 
                                    String(humidityDHTMeasuredValue) + 
                                    MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_3;

  Serial.print(MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_4);
  
  sendDataStringToWiFiModule(4, redAlertMsgToSend);

}

void sendMeasurementOrReading(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  String measurementReadingMsgToSend = MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_1 + 
                                       String(temperatureDHTMeasuredValue) + 
                                       MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_2 + 
                                       String(humidityDHTMeasuredValue) + 
                                       MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_3;

  Serial.print(MSG_CURRENT_DHT_MEASUREMENT_READING_TO_SEND_4);
  
  sendDataStringToWiFiModule(5, measurementReadingMsgToSend);

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

void idleState() {
  
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
