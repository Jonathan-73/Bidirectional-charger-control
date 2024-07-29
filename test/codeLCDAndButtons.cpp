#include <Arduino.h>
#include "Wire.h"
#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C LCD(0x27,16,2); // définit le type d'ecran lcd 16 x 2

const int button1 = 2;
const int button2 = 3;

int button1State = 0;
int button2State = 0;

void setup() {
    Serial.begin(9600);
    LCD.init(); // initialisation de l'afficheur
    LCD.backlight();
    LCD.display();

    pinMode(button1, INPUT);
    pinMode(button2, INPUT);

}

void loop() {
    LCD.clear();
    button1State = digitalRead(button1);
    button2State = digitalRead(button2);
    LCD.setCursor(0,0);
    if(button1State == HIGH) {
        LCD.print("Bouton 1 ON");
        Serial.println("Bouton 1 ON");
    }
    else {
        LCD.print("Bouton 1 OFF");
        Serial.println("Bouton 1 OFF");
    }
    LCD.setCursor(0,1);
    if(button2State == HIGH) {
        LCD.print("Bouton 2 ON");
        Serial.println("Bouton 2 ON");
    }
    else {
        LCD.print("Bouton 2 OFF");
        Serial.println("Bouton 2 OFF");
    }
    delay(1000);
}
