/*
  Switching Power Plugs dependent of ambient light by CONTROLLINO
  
                                                                       ///Light
  Light Sensor is a photo resitor connected to A1.     |--/\/\/\--|--/\/\/\--- VCC
  Power Plugs are switched by Relais_00 and _01                    A1
  Created by: Claus Kühnel 2017-02-08 info@ckuehnel.ch
*/
#include <Controllino.h>
#include <Wire.h>
#include "Arduino.h"

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTEthernet.h>

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard (BYOD).
char username[] = "bd6f7450-a9bd-11e6-a85d-c165103f15c2";
char password[] = "18dc9339c1698e9e9733a5ad8d2d2e5ccfacd095";
char clientID[] = "abf8afd0-ee13-11e6-8577-0128e408a1ba";

int SWITCH_ON = 200;  // higher values mean more light
int SWITCH_OFF = SWITCH_ON + 5;
int light = 0;
boolean state = false; // false means R0 off

void setup() 
{
  /* Here we prepare D0, R0, and R1 as output, A1 as input */
  pinMode(CONTROLLINO_D0, OUTPUT);
  pinMode(CONTROLLINO_RELAY_00, OUTPUT);
  pinMode(CONTROLLINO_RELAY_01, OUTPUT);
  pinMode(CONTROLLINO_A1, INPUT);

  /* Here we initialize serial at 115200 baudrate for reporting */
  Serial.begin(115200);
  while (!Serial); // wait for conecting monitor
  
  Serial.println("\n\nLight Control MQTT");
  Serial.println("Initializing...");

  /* If we are using Controllino MEGA, we want to try using non arduino supported pins, so we initialize them */
  #ifdef CONTROLLINO_MEGA
    /* We need to set direction of all pins to output (1). We are going to use pins PD4(D20), PD5 (D21), PD6(D22) and PJ4(D23) and set them to low*/
    DDRD |= B01110000;
    DDRJ |= B00010000;$
    PORTD &= B10001111;
    PORTJ &= B11101111;
  #endif
  /* When using Controllino MEGA or MAXI we have access to /OVL pin and RS458 /RE DE pins */
  #if defined(CONTROLLINO_MAXI) || defined(CONTROLLINO_MEGA)
    /* Direction for /RE (PJ5) DE (PJ6) pins is output (1). For /OLV (PE7) its input (0) */
    DDRJ |= B01100000;
    DDRE &= B01111111;
    PORTJ &= B10011111;
    Serial.print("/OVL: "); Serial.println(PINE >> 7);
  #endif
  
  // Connect to Cayenne MQTT broker
  Cayenne.begin(username, password, clientID);
  
  /* Now we report start of application */
  Serial.println("Starting CONTROLLINO application now");
}

void loop() 
{
   /* We set digital output D0 to low voltage */
  digitalWrite(CONTROLLINO_D0, HIGH);

  Cayenne.loop();
  
  Serial.println("-------------");
  float volt = (5.0 * analogRead(A1) * 100.0) / 1024;
  Serial.print("Res:   "); Serial.println(volt);
  Serial.print("State: "); Serial.println(state);

  if (state == false)
  {
    if (volt < SWITCH_ON) 
    {
      state = !state;
      digitalWrite(CONTROLLINO_RELAY_00, HIGH);
      Serial.println("Lamps on");
      Cayenne.virtualWrite(0, state);
      Cayenne.virtualWrite(1, volt);
    }
  }
  if (state == true)
  {
    if (volt > SWITCH_OFF) 
    {
      state = !state;
      digitalWrite(CONTROLLINO_RELAY_00, LOW);
      Serial.println("Lamps off");
      Cayenne.virtualWrite(0, state);
      Cayenne.virtualWrite(1, volt);
    }
  }
  digitalWrite(CONTROLLINO_D0, LOW);
  delay(1000);
}

//Default function for processing actuator commands from the Cayenne Dashboard.
//You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("CAYENNE_IN_DEFAULT(%u) - %s, %s", request.channel, getValue.getId(), getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
  
}
