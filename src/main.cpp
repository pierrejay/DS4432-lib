#include <Arduino.h>
#include <Wire.h>
#include "DS4432.h"

#define SDA_PIN D8
#define SCL_PIN D7

void testValue(uint8_t channel, int8_t expected) {
    int8_t actual = DS4432::get(Wire, channel);
    Serial.print("Channel ");
    Serial.print(channel);
    Serial.print(" - Expected: ");
    Serial.print(expected);
    Serial.print(", Read: ");
    Serial.print(actual);
    Serial.print(" -> ");
    Serial.println((expected == actual) ? "OK" : "ERROR");
}

void scanI2C() {
  Serial.println("Scanning I2C bus...");
  byte count = 0;
  
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Device found at address 0x");
      if (i < 16) Serial.print("0");
      Serial.print(i, HEX);
      Serial.print(" (0x");
      if (i*2 < 16) Serial.print("0");
      Serial.print(i*2, HEX);
      Serial.println(" in 8-bit format)");
      count++;
    }
  }
  
  Serial.print("Found ");
  Serial.print(count);
  Serial.println(" device(s).");
}

void setup() {
    delay(3000);

    Serial.begin(9600);
    delay(10);

    Serial.println("\nDS4432 test");
    
    // Init I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    // Wire.setClock(100000); // Réduire la vitesse I2C à 100kHz pour plus de fiabilité
    vTaskDelay(pdMS_TO_TICKS(100)); // Augmenter le délai pour laisser plus de temps au DS4432 de s'initialiser
    
    // Scanner le bus I2C
    scanI2C();
    
    // Vérifier si le DS4432 est présent
    Wire.beginTransmission(DS4432::ADDR);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
        Serial.println("DS4432 found!");
    } else {
        Serial.printf("DS4432 not found! Error: %d\n", error);
    }
    
    // Test canal 0
    Serial.println("\nTesting channel 0 (source)");
    if (DS4432::set(Wire, 0, 100)) {
        Serial.println("Write OK");
        testValue(0, 100);
    } else {
        Serial.println("Write error!");
    }
    
    // Test canal 1
    Serial.println("\nTesting channel 1 (sink)");
    if (DS4432::set(Wire, 1, -75)) {
        Serial.println("Write OK");
        testValue(1, -75);
    } else {
        Serial.println("Write error!");
    }
    
    // Test valeur nulle
    Serial.println("\nTesting channel 0 (zero)");
    if (DS4432::set(Wire, 0, 0)) {
        Serial.println("Write OK");
        testValue(0, 0);
    } else {
        Serial.println("Write error!");
    }
    
    Serial.println("\nTests finished");
}

void loop() {
    static const int8_t values[] = {-127, -100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100, 127};
    // static const int8_t values[] = {127};
    static const int8_t nbValues = sizeof(values) / sizeof(values[0]);
    static uint8_t index = 0;
    
    // Applique la valeur aux deux canaux
    int8_t value = values[index];
    Serial.print("\nNew value: ");
    Serial.println(value);
    
    for(uint8_t channel = 0; channel <= 1; channel++) {
        if (DS4432::set(Wire, channel, value)) {
            testValue(channel, value);
        } else {
            Serial.printf("Write error on channel %d!\n", channel);
        }
    }
    
    // Passe à la valeur suivante
    index = (index + 1) % nbValues;
    
    delay(2000);
}