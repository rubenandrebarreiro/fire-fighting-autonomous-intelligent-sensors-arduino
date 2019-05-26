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
unsigned long timeSinceLastSendingYellowAlert;
unsigned long timeSinceLastSendingOrangeAlert;
unsigned long timeSinceLastSendingRedAlert;
unsigned long timeSinceLastSendingIdleAlert;

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
  timeSinceLastSendingYellowAlert = 0;
  timeSinceLastSendingOrangeAlert = 0;
  timeSinceLastSendingRedAlert = 0;
  timeSinceLastSendingIdleAlert = 0;
  
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
 
  while(flameSensorMeasuredValue > flameSensorThreshold) {
    Serial.print("Current Data Measured: [Flame's IR = ");
    Serial.print(flameSensorMeasuredValue);
    Serial.println("]");
    
    verifySendAlertMsg(5, flameSensorMeasuredValue, 0, 0);

    fireAlert();
    
    flameSensorMeasuredValue = analogRead(A0);
  }
  
  turnOffAlert();
}

void performMeasurementsOrReadings() {

  int flameSensorMeasuredValue = analogRead(A0);
  
  int checkDHTMeasuredValue = DHT.read11(7);
  int temperatureDHTMeasuredValue = DHT.temperature;
  int humidityDHTMeasuredValue = DHT.humidity;
  
  if( (temperatureDHTMeasuredValue >= 45) || ( (lastTemperatureDHTMeasuredValue >= 45) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {
   Serial.print("Current Data Measured: [Temperature = ");
   Serial.print(temperatureDHTMeasuredValue);
   Serial.print("; Humidity = ");
   Serial.print(humidityDHTMeasuredValue);
   Serial.println("]");

   verifySendAlertMsg(4, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
   
   imminentOccurrenceFireAlert();   
  }
  else if( (temperatureDHTMeasuredValue >= 40) || ( (lastTemperatureDHTMeasuredValue >= 40) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {
   Serial.print("Current Data Measured: [Temperature = ");
   Serial.print(temperatureDHTMeasuredValue);
   Serial.print("; Humidity = ");
   Serial.print(humidityDHTMeasuredValue);
   Serial.println("]");
   
   verifySendAlertMsg(3, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
    
   veryPossibleOccurrenceFireAlert();
  }
  else if( (temperatureDHTMeasuredValue >= 35) || ( (lastTemperatureDHTMeasuredValue >= 35) && ( (lastTemperatureDHTMeasuredValue + temperatureDHTMeasuredValue) < 0) ) ) {
    Serial.print("Current Data Measured: [Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");

    verifySendAlertMsg(2, 0, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
    
    possibleOccurrenceFireAlert();
  }
  else {
    Serial.print("Current Data Measured: [Flame's IR = ");
    Serial.print(flameSensorMeasuredValue);
    Serial.print("; Temperature = ");
    Serial.print(temperatureDHTMeasuredValue);
    Serial.print("; Humidity = ");
    Serial.print(humidityDHTMeasuredValue);
    Serial.println("]");    
    
    verifySendAlertMsg(1, flameSensorMeasuredValue, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
    
    idleStateAlert();
  }

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

void verifySendAlertMsg(int alertType, int flameSensorMeasuredValue, int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {

  // IDLE ALERT
  if(alertType == 1) {
    long timeOfIdleAlertOccurrence = 0;

    timeOfIdleAlertOccurrence = millis() - timeSinceLastSendingIdleAlert;

    if(timeOfIdleAlertOccurrence > 60000) {
      sendIdleAlert(flameSensorMeasuredValue, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingIdleAlert = millis();
    }
  }

  // YELLOW ALERT
  if(alertType == 2) {
    long timeOfYellowAlertOccurrence = 0;

    timeOfYellowAlertOccurrence = millis() - timeSinceLastSendingYellowAlert;

    if(timeOfYellowAlertOccurrence > 7200000) {
      sendYellowAlert(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingYellowAlert = millis();
    }
  }

  // ORANGE ALERT
  if(alertType == 3) {
    long timeOfOrangeAlertOccurrence = 0;

    timeOfOrangeAlertOccurrence = millis() - timeSinceLastSendingOrangeAlert;

    if(timeOfOrangeAlertOccurrence > 3600000) {
      sendOrangeAlert(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingOrangeAlert = millis();
    }
  }

  // RED ALERT
  if(alertType == 4) {
    long timeOfRedAlertOccurrence = 0;

    timeOfRedAlertOccurrence = millis() - timeSinceLastSendingRedAlert;

    if(timeOfRedAlertOccurrence > 1800000) {
      sendRedAlert(temperatureDHTMeasuredValue, humidityDHTMeasuredValue);
      timeSinceLastSendingRedAlert = millis();
    }
  }

  // FIRE ALERT
  if(alertType == 5) {
    
    long timeOfFireAlertOccurrence = 0;

    timeOfFireAlertOccurrence = millis() - timeSinceLastSendingFireAlert;

    if(timeOfFireAlertOccurrence > 5000) {
      sendFireAlert(flameSensorMeasuredValue);
      timeSinceLastSendingFireAlert = millis();
    }
  }
}

void sendIdleAlert(int flameSensorMeasuredValue, int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  char idleAlertMsgToSend[50];
  
  sprintf(idleAlertMsgToSend, "M%d - {[Flame's IR = %d; Temperature = %d; Humidity = %d}}",
          1, flameSensorMeasuredValue, temperatureDHTMeasuredValue, humidityDHTMeasuredValue); 
  
  fireFightingSensorSerial.write(idleAlertMsgToSend);
}

void sendYellowAlert(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {

  char yellowAlertMsgToSend[50];
  
  sprintf(yellowAlertMsgToSend, "M%d - {[Temperature = %d; Humidity = %d]}",
          2, temperatureDHTMeasuredValue, humidityDHTMeasuredValue); 
           
  Serial.println("SENDING YELLOW ALERT...");

  fireFightingSensorSerial.write(yellowAlertMsgToSend);
}

void sendOrangeAlert(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  char orangeAlertMsgToSend[50];
  
  sprintf(orangeAlertMsgToSend, "M%d - {[Temperature = %d; Humidity = %d]}",
          3, temperatureDHTMeasuredValue, humidityDHTMeasuredValue); 
           
  Serial.println("SENDING ORANGE ALERT...");

  fireFightingSensorSerial.write(orangeAlertMsgToSend);
}

void sendRedAlert(int temperatureDHTMeasuredValue, int humidityDHTMeasuredValue) {
  
  char redAlertMsgToSend[50];
  
  sprintf(redAlertMsgToSend, "M%d - {[Temperature = %d; Humidity = %d]}",
          4, temperatureDHTMeasuredValue, humidityDHTMeasuredValue);  
           
  Serial.println("SENDING RED ALERT...");

  fireFightingSensorSerial.write(redAlertMsgToSend);
}


void sendFireAlert(int flameSensorMeasuredValue) {

  char fireAlertMsgToSend[50];
  
  sprintf(fireAlertMsgToSend, "M%d - {[Flame's IR = %d]}",
          5, flameSensorMeasuredValue); 
           
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

void idleStateAlert() {
  
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
