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

bool canCharge = false;
bool canDischarge = false;

const int relayChargeDischarge = 4; // Digital Arduino Pins used to activate the relays
const int relayGridPower = 5;
const int relayChOnOff = 6;
const int relayChHighLow = 7;

int timer = 20;

byte ok[8] = {
    0b00000,
    0b00001,
    0b10010,
    0b01010,
    0b01100,
    0b01000,
    0b00000,
    0b00000
};

byte cross[8] = {
    0b00000,
    0b10001,
    0b01010,
    0b00100,
    0b01010,
    0b10001,
    0b10001,
    0b00000
};

LiquidCrystal_I2C LCD(0x27,16,2); // définit le type d'ecran lcd 16 x 2

/**
 * @brief Function which setups the charger with the grid not powered,
 * in charging mode, with the charger at modes OFF and LOW
 */
void setupCheckSystem();

/**
 * @brief Function which allows the user to choose
 * between charge and discharge and, in charging mode,
 * between slow and fast charge
 */
void chooseMode();

/**
 * @brief Function which clears the bottom line of the LCD screen
 */
void LCDClearBottom();

void setup() {
    Serial.begin(9600);
    //Definition of the Arduino pins as digital outputs
    pinMode(relayChargeDischarge, OUTPUT); //Relay for charge/discharge
    pinMode(relayGridPower, OUTPUT); //Relay for grid powering
    pinMode(relayChOnOff, OUTPUT); //Relay for activating the charger
    pinMode(relayChHighLow, OUTPUT); //Relay for choosing HIGH/LOW mode of the charger

    pinMode(button1, INPUT);
    pinMode(button2, INPUT);

    pinMode(gridPoweredSensor, INPUT);

    LCD.init(); // initialisation de l'afficheur
    LCD.createChar(1, ok);
    LCD.createChar(2, cross);
    LCD.backlight();
    LCD.display();
    LCD.clear();
    LCD.print("starting");
}

void loop() {
    //Setup check system
    setupCheckSystem();
    //End of setup check system
    //Choosing mode
    chooseMode();
    if (chargeDischarge == CHARGE){  //Charging mode
        Serial.println("Entering charging mode");
        if(canCharge){
            digitalWrite(relayGridPower, ON);   //Grid powered
            digitalWrite(relayChOnOff, ON);  //Charger set on
            //Choosing charger speed
            if(highLowMode == M_LOW) {  //Slow charge
                Serial.println("Slow charge");
                for(int i = 0; i < timer; i++){        //Slow charge during 30 secondes
                    measuredChargingCurrent = measureChargingCurrent();
                    LCD.clear();
                    LCD.print("Measure: ");
                    LCD.setCursor(9,0);
                    LCD.print(measuredChargingCurrent);
                    LCD.setCursor(14,0);
                    LCD.print("A");
                    LCD.setCursor(0,1);
                    LCD.print("SLOW CHARGING");
                    delay(200);
                }
            }
            if (highLowMode == M_HIGH){    //Speed charge
                digitalWrite(relayChHighLow, ON);   //Charger in HIGH mode
                Serial.println("Fast charge");
                for(int i = 0; i < timer; i++){   //Fast charge during 10 secondes
                    measuredChargingCurrent = measureChargingCurrent();
                    LCD.clear();
                    LCD.print("Measure: ");
                    LCD.setCursor(9,0);
                    LCD.print(measuredChargingCurrent);
                    LCD.setCursor(14,0);
                    LCD.print("A");
                    LCD.setCursor(0,1);
                    LCD.print("FAST CHARGING");
                    delay(200);
                }
            }
        } else{
            LCD.clear();
            LCD.print("BATTERY FULL");
            delay(2000);
        }
    }
    if (chargeDischarge == DISCHARGE){  //Discharging mode
        if(canDischarge){
            digitalWrite(relayChargeDischarge, ON);   //Discharging mode
            digitalWrite(relayGridPower, ON);   //Grid powered
            Serial.println("Entering discharging mode");
            for(int i = 0; i < timer; i++){
                measuredDischargingCurrent = measureDischargingCurrent();
                LCD.clear();
                LCD.print("Measure: ");
                LCD.setCursor(9,0);
                LCD.print(measuredDischargingCurrent);
                LCD.setCursor(14,0);
                LCD.print("A");
                LCD.setCursor(0,1);
                LCD.print("DISCHARGING");
                delay(200);   //Discharging during 30 secondes
            }
        } else{
            LCD.clear();
            LCD.print("BATTERY EMPTY");
            delay(2000);
        }
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

void chooseMode(){
    bool choiceChDch = CHARGE;
    bool choiceMode = M_LOW;
    LCD.clear();
    LCD.print("Charge:");
    LCD.setCursor(7,0);
    if(canCharge) LCD.print(char(1));
    else LCD.print(char(2));
    LCD.setCursor(9,0);
    LCD.print("Disch:");
    LCD.setCursor(15,0);
    if(canDischarge)LCD.print(char(1));
    else LCD.print(char(2));
    button2State = LOW;
    while(button2State != HIGH){
        LCDClearBottom();
        button1State = digitalRead(button1);
        if(button1State == HIGH) choiceChDch = (choiceChDch + 1)%2;
        if(choiceChDch == CHARGE) LCD.print("CH or DCH: CH?");
        else LCD.print("CH or DCH: DCH?");
        button2State = digitalRead(button2);
        delay(100);
    }
    chargeDischarge = choiceChDch;
    if (choiceChDch == DISCHARGE) return;
    button2State = LOW;
    while(button2State != HIGH){
        LCDClearBottom();
        button1State = digitalRead(button1);
        if(button1State == HIGH) choiceMode = (choiceMode + 1)%2;
        if(choiceMode == M_LOW) LCD.print("CH MODE: LOW?");
        else LCD.print("CH MODE: HIGH?");
        button2State = digitalRead(button2);
        delay(100);
    }
    highLowMode = choiceMode;
    return;
}

void LCDClearBottom(){
    LCD.setCursor(0,1);
    LCD.print("                ");
    LCD.setCursor(0,1);
}