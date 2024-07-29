#include <Arduino.h>
#include <sensors.h>
#include "Wire.h"
#include <LiquidCrystal_I2C.h>

#define ON 1
#define OFF 0

const int button1 = 2;
const int button2 = 3;

int button1State = 0;
int button2State = 0;

double measuredDischargingCurrent = 0;
double measuredChargingCurrent = 0;

enum ch_dch {CHARGE, DISCHARGE};
enum high_low {M_HIGH, M_LOW};
bool chargeDischarge = CHARGE;
bool highLowMode = M_LOW;

int relayChargeDischarge = 4; // Digital Arduino Pins used to activate the relays
int relayGridPower = 5;
int relayChOnOff = 6;
int relayChHighLow = 7;

LiquidCrystal_I2C LCD(0x27,16,2); // définit le type d'ecran lcd 16 x 2

/**
 * @brief Function which setups the charger with the grid not powered,
 * in charging mode, with the charger at modes OFF and LOW
 */
void setupCheckSystem();

void setup() {
    Serial.begin(9600);
    // Definition of the Arduino pins as digital outputs
    pinMode(relayChargeDischarge, OUTPUT); //Relay for charge/discharge
    pinMode(relayGridPower, OUTPUT); //Relay for grid powering
    pinMode(relayChOnOff, OUTPUT); //Relay for activating the charger
    pinMode(relayChHighLow, OUTPUT); //Relay for choosing HIGH/LOW mode of the charger

    pinMode(button1, INPUT);
    pinMode(button2, INPUT);

    pinMode(gridPoweredSensor, INPUT);

    LCD.init(); // initialisation de l'afficheur
    LCD.backlight();
    LCD.display();
}

void loop() {
    //Setup check system
    setupCheckSystem();
    //End of setup check system
    chargeDischarge = chooseMode();
    //Choosing mode
    if (chargeDischarge == CHARGE){  //Charging mode
        Serial.println("Entering charging mode");
        digitalWrite(relayGridPower, ON);   //Grid powered
        digitalWrite(relayChOnOff, ON);  //Charger set on
        //Choosing charger speed
        highLowMode = M_LOW;
        if(highLowMode == M_LOW) {  //Slow charge
            Serial.println("Slow charge");
            delay(30000);   //Slow charge during 30 secondes
        }
        highLowMode = M_HIGH;
        if (highLowMode == M_HIGH){    //Speed charge
            Serial.println("Fast charge");
            digitalWrite(relayChHighLow, ON);   //Charger in HIGH mode
            delay(10000);   //Fast charge during 10 secondes
        }
    }

    chargeDischarge = DISCHARGE;
    setupCheckSystem();

    if (chargeDischarge == DISCHARGE){  //Discharging mode
        Serial.println("Entering discharging mode");
        digitalWrite(relayChargeDischarge, ON);   //Discharging mode
        digitalWrite(relayGridPower, ON);   //Grid powered
        delay(30000);   //Discharging during 30 secondes
    }
}

void setupCheckSystem(){
    Serial.println("Setup check system begins");
    digitalWrite(relayGridPower, OFF);   //Grid not powered
    digitalWrite(relayChargeDischarge, OFF);   //Charging mode
    digitalWrite(relayChOnOff, OFF);  //Charger set off
    digitalWrite(relayChHighLow, OFF);  //Charger in LOW mode
    Serial.println("Setup check system ends");
}

bool chooseMode(){
    
}