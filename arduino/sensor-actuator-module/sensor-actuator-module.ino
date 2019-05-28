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
#include "dht.h"

dht DHT;

// RX (Receive) = 2, TX (Transmit) = 3
SoftwareSerial fireFightingSensorSerial(2, 3);

unsigned long timeSinceLastSendingFireAlert;
unsigned long timeSinceLastSendingMeasurementReading;

int lastFlameSensorMeasuredValue;
int lastTemperatureDHTMeasuredValue;
int lastHumidityDHTMeasuredValue;

int flameSensorThreshold;

bool reconnectingWiFiState;

void setup() {
  
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  
  Serial.begin(9600);
  fireFightingSensorSerial.begin(115200);
  
  lastFlameSensorMeasuredValue = analogRead(A0);

  if(lastFlameSensorMeasuredValue == 0) {
    flameSensorThreshold = 20;
  }
  else {
    flameSensorThreshold = 120;
  }
    
  DHT.read11(7);
  lastTemperatureDHTMeasuredValue = DHT.temperature;
  lastHumidityDHTMeasuredValue = DHT.humidity;

  reconnectingWiFiState = false;
  
  while(!fireFightingSensorSerial.available()) {
    connectionWiFiAlert();
    
    delay(2000);
    Serial.print(".");
  }
    
  sendRegisterItselfInfo();

  timeSinceLastSendingFireAlert = 0;
  timeSinceLastSendingMeasurementReading = 0;
}

void loop() {
  if(fireFightingSensorSerial.available()) {
    delay(1000);

    tryToDetectFire();
    
    byte byteReceived = fireFightingSensorSerial.read();
    
    if(byteReceived == 0) {
      if(!reconnectingWiFiState) {
       // Check WiFi connection status
       reconnectingWiFiState = true;    
      }
      else {
        delay(1000);
        Serial.print(".");
      }
      
      connectionWiFiAlert();
    }
    else {
      reconnectingWiFiState = false;
      performMeasurementsOrReadings();
    }
  }
}

void tryToDetectFire() {
  int flameSensorMeasuredValue = analogRead(A0);

  bool detectingFire = false;
  
  while(flameSensorMeasuredValue > flameSensorThreshold) {
    Serial.print("Current Data Measured: [Flame's IR = ");
    Serial.print(flameSensorMeasuredValue);
    Serial.println("]");

    if(!detectingFire) {
      verifySendAlertMsg(2, flameSensorMeasuredValue, 0, 0);  
    }

    detectingFire = true;
    
    fireAlert();
    
    flameSensorMeasuredValue = analogRead(A0);
  }
  
  turnOffAlert();
}

void performMeasurementsOrReadings() {
   
  int checkDHTMeasuredValue = DHT.read11(7);
  int temperatureDHTMeasuredValue = DHT.temperature;
  int humidityDHTMeasuredValue = DHT.humidity;

  // If the temperature is equal or greater than 45 degrees, it will be generated a RED alert or an ORANGE alert
  if( (temperatureDHTMeasuredValue >= 45) || ( (lastTemperatureDHTMeasuredValue >= 45) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {

   // If the humidity is equal or greater than 40%, it will be generated an ORANGE alert
   if( (humidityDHTMeasuredValue >= 40) || ( (lastHumidityDHTMeasuredValue >= 40) && ( (lastHumidityDHTMeasuredValue + humidityDHTMeasuredValue) < 0) ) ) {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");

    veryPossibleOccurrenceFireAlert(); 
   }
   // If the humidity is less than 40%, it will be generated a RED alert
   else {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");

    imminentOccurrenceFireAlert(); 
   }  
  }
  // If the temperature is equal or greater than 40 degrees, it will be generated an ORANGE alert or a YELLOW alert
  else if( (temperatureDHTMeasuredValue >= 40) || ( (lastTemperatureDHTMeasuredValue >= 40) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {
   
   // If the humidity is equal or greater than 40%, it will be generated an YELLOW alert
   if( (humidityDHTMeasuredValue >= 40) || ( (lastHumidityDHTMeasuredValue >= 40) && ( (lastHumidityDHTMeasuredValue + humidityDHTMeasuredValue) < 0) ) ) {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");
     
    possibleOccurrenceFireAlert();
   }
   // If the humidity is less than 40%, it will be generated an ORANGE alert
   else {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");
     
    veryPossibleOccurrenceFireAlert();
   }
  }
  // If the temperature is equal or greater than 35 degrees, it will be generated a YELLOW alert or no alert, at all
  else if( (temperatureDHTMeasuredValue >= 35) || ( (lastTemperatureDHTMeasuredValue >= 35) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {

   // If the humidity is equal or greater than 40%, it will be generated no alert
   if( (humidityDHTMeasuredValue >= 40) || ( (lastHumidityDHTMeasuredValue >= 40) && ( (lastHumidityDHTMeasuredValue + humidityDHTMeasuredValue) < 0) ) ) {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");
    
    idleState();
   }
   // If the humidity is less than 40%, it will be generated a YELLOW alert
   else {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");

    possibleOccurrenceFireAlert();
   }
  }
  else {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");
    
    idleState();
  }

  verifySendAlertMsg(1, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);

  lastTemperatureDHTMeasuredValue = temperatureDHTMeasuredValue;
  lastHumidityDHTMeasuredValue = humidityDHTMeasuredValue;

  delay(200);
}

void sendRegisterItselfInfo() {
  char registerItselfInfoMsgToSend[50];
  
  sprintf(registerItselfInfoMsgToSend, "M%d - {[ fire-fighting-sensor-00 ]}", 0); 
           
  Serial.println("SENDING SENSOR'S REGISTRATION INFO...");

  fireFightingSensorSerial.write(registerItselfInfoMsgToSend);
}

void verifySendAlertMsg(int messageType, int flameSensorMeasuredValue, int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {

  // MEASUREMENT/READING
  if(messageType == 1) {
    long timeOfMeasurementReadingOccurrence = 0;

    timeOfMeasurementReadingOccurrence = millis() - timeSinceLastSendingMeasurementReading;

    if(timeOfMeasurementReadingOccurrence > 60000) {
      sendMeasurementReading(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingMeasurementReading = millis();
    }
  }

  // FIRE ALERT
  if(messageType == 2) {
    
    long timeOfFireAlertOccurrence = 0;

    timeOfFireAlertOccurrence = millis() - timeSinceLastSendingFireAlert;

    if(timeOfFireAlertOccurrence > 5000) {
      sendFireAlert(flameSensorMeasuredValue);
      timeSinceLastSendingFireAlert = millis();
    }
  }
}

void sendMeasurementReading(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  char measurementReadingMsgToSend[50];
  
  sprintf(measurementReadingMsgToSend, "M%d - {[Temperature = %d; Humidity = %d}}",
          1, temperatureDHTMeasuredValue, humidityDHTMeasuredValue); 
  
  fireFightingSensorSerial.write(measurementReadingMsgToSend);
}


void sendFireAlert(int flameSensorMeasuredValue) {

  char fireAlertMsgToSend[50];
  
  sprintf(fireAlertMsgToSend, "M%d - {[Flame's IR = %d]}",
          2, flameSensorMeasuredValue); 
           
  Serial.println("SENDING FIRE ALERT...");

  fireFightingSensorSerial.write(fireAlertMsgToSend); 
}

void connectionWiFiAlert() {
  
  // Blue (turn just the BLUE LED on):
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, HIGH);

  tone(6, 1000, 10);

  delay(50);

  // Off (all LEDs off):
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
    
  tone(6, 1000, 10);

  delay(50);
}

void idleState() {
  
  // Green (turn just the GREEN LED on):
  digitalWrite(9, LOW);
  digitalWrite(10, HIGH);
  digitalWrite(11, LOW);

  tone(6, 100, 100);

  delay(2000);

  // Off (all LEDs off):
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
    
  tone(6, 100, 100);

  delay(2000);
}

void possibleOccurrenceFireAlert() {

  // Yellow (turn just the GREEN and RED LED on):
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, LOW);

  tone(6, 100, 100);

  delay(600);

  // Off (all LEDs off):
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
    
  tone(6, 100, 100);

  delay(600);
}

void veryPossibleOccurrenceFireAlert() {

  // Orange (turn just the GREEN and RED LED on):
  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, LOW);

  analogWrite(9, 255);
  analogWrite(10, 30);
  analogWrite(11, 0);
  
  tone(6, 100, 100);

  delay(200);

  // Off (all LEDs off):
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);

  analogWrite(9, 0);
  analogWrite(10, 0);
  analogWrite(11, 0);
    
  tone(6, 100, 100);

  delay(200);
}

void imminentOccurrenceFireAlert() {
  
  // Red (turn just the RED LED on):
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);

  tone(6, 100, 100);

  delay(100);

  // Off (all LEDs off):
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
    
  tone(6, 100, 100);

  delay(100);
}

void fireAlert() {

  // Red (turn just the red LED on):
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);

  tone(6, 10, 10000);
}

void turnOffAlert() {
  
  // Off (all LEDs off):
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
}
