#include <Arduino.h>
#include <sensors.h>
#include "Wire.h"
#include <LiquidCrystal_I2C.h>

#define ON 0
#define OFF 1

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
bool isGridConnected = false;

const int relayChargeDischarge = 4; // Digital Arduino Pins used to activate the relays
const int relayGridPower = 5;
const int relayChOnOff = 6;
const int relayChHighLow = 7;

const int canChargePin = 8;
const int canDischargePin = 9;

byte ok[8] = {      //Caracter for check ✓ sign on LCD screen
    0b00000,
    0b00001,
    0b10010,
    0b01010,
    0b01100,
    0b01000,
    0b00000,
    0b00000
};

LiquidCrystal_I2C LCD(0x27,16,2); // definition of the screen with its adress and size

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
 * and set the cursor at the beginning of the bottom line
 */
void LCDClearBottom();

/**
 * @brief Function which verifies if the BMS allows the charge
 */
void verifyIfCanCharge();

/**
 * @brief Function which verifies if the BMS allows the discharge
 */
void verifyIfCanDischarge();

void setup() {
    Serial.begin(9600);
    //Definition of the Arduino pins as digital outputs
    pinMode(relayChargeDischarge, OUTPUT); //Relay for charge/discharge
    pinMode(relayGridPower, OUTPUT); //Relay for grid powering
    pinMode(relayChOnOff, OUTPUT); //Relay for activating the charger
    pinMode(relayChHighLow, OUTPUT); //Relay for choosing HIGH/LOW mode of the charger
    setupCheckSystem();

    pinMode(button1, INPUT);    //Definition of the 2 buttons as an input
    pinMode(button2, INPUT);    //for the HMI

    pinMode(gridPoweredSensor, INPUT);  //Pin for the binary sensor which verifies if the grid is powered

    pinMode(canChargePin, INPUT);       //pins connected to the relays of the BMS
    pinMode(canDischargePin, INPUT);    //which allows or not charge or discharge

    LCD.init(); // initialisation of the screen
    LCD.createChar(1, ok);
    LCD.backlight();
    LCD.display();
    LCD.clear();
    LCD.print("starting");
}

void loop() {
    // if(digitalRead(canChargePin) == HIGH) Serial.print("Pin Charge HIGH      ");
    // else Serial.print("Pin Charge LOW       ");
    // if(digitalRead(canDischargePin) == HIGH) Serial.println("Pin Discharge HIGH      ");
    // else Serial.println("Pin Discharge LOW       ");

    //Setup check system
    setupCheckSystem();
    //End of setup check system
    do{
        if(digitalRead(gridPoweredSensor) == HIGH) isGridConnected = true;
        else isGridConnected = false;
        LCD.clear();
        LCD.print("Waiting for");
        LCD.setCursor(0,1);
        LCD.print("grid connection");
        delay(200);
    }
    while(!isGridConnected);

    //Choosing mode
    chooseMode();
    if (chargeDischarge == CHARGE){  //Charging mode
        Serial.println("Entering charging mode");
        while(canCharge){
            digitalWrite(relayGridPower, ON);   //Grid powered
            digitalWrite(relayChOnOff, ON);  //Charger set on
            //Choosing charger speed
            if(highLowMode == M_LOW) {  //Slow charge
                Serial.println("Slow charge");
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
            if (highLowMode == M_HIGH){    //Speed charge
                while(canCharge){
                    digitalWrite(relayChHighLow, ON);   //Charger in HIGH mode
                    Serial.println("Fast charge");
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
                    verifyIfCanCharge();
                }
                digitalWrite(relayChOnOff, OFF);  //Charger set on
                delay(60000);    //Waiting 1 minute for balancing of battery
                verifyIfCanCharge();    //If battery not full
                while(canCharge){       //Finishing the charge in slow mode
                    digitalWrite(relayChHighLow, OFF);   //Charger in LOW mode
                    digitalWrite(relayChOnOff, ON);  //Charger set on
                    Serial.println("Slow charge");
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
                    verifyIfCanCharge();
                }
            }
            verifyIfCanCharge();
        }
        LCD.clear();
        LCD.print("BATTERY FULL");
        delay(2000);
    }
    if (chargeDischarge == DISCHARGE){  //Discharging mode
        while(canDischarge){
            digitalWrite(relayChargeDischarge, ON);   //Discharging mode
            digitalWrite(relayGridPower, ON);   //Grid powered
            Serial.println("Entering discharging mode");
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
            verifyIfCanDischarge();
        }
        LCD.clear();
        LCD.print("BATTERY EMPTY");
        delay(2000);
    }
}

void setupCheckSystem(){
    Serial.println("Setup check system begins");
    digitalWrite(relayGridPower, OFF);   //Grid not powered
    digitalWrite(relayChargeDischarge, OFF);   //Charging mode
    digitalWrite(relayChOnOff, OFF);  //Charger set off
    digitalWrite(relayChHighLow, OFF);  //Charger in LOW mode
    isGridConnected = false;
    Serial.println("Setup check system ends");
}

void chooseMode(){
    bool choiceChDch = CHARGE;
    bool choiceMode = M_LOW;
    verifyIfCanCharge();
    verifyIfCanDischarge();
    //Printing of the selection screen
    LCD.clear();
    LCD.print("Charge:");
    LCD.setCursor(7,0);
    if(canCharge) LCD.print(char(1));
    else LCD.print("X");
    LCD.setCursor(9,0);
    LCD.print("Disch:");
    LCD.setCursor(15,0);
    if(canDischarge)LCD.print(char(1));
    else LCD.print("X");

    button2State = LOW;
    while(button2State != HIGH){    //First selection screen
        LCDClearBottom();
        button1State = digitalRead(button1);
        if(button1State == HIGH) choiceChDch = (choiceChDch + 1)%2;
        if(choiceChDch == CHARGE) LCD.print("CH or DCH: CH?");
        else LCD.print("CH or DCH: DCH?");
        button2State = digitalRead(button2);
        delay(100);
    }
    chargeDischarge = choiceChDch;
    if (choiceChDch == DISCHARGE) return;   //If discharge mode chosen no need for more information
    button2State = LOW;
    while(button2State != HIGH){    //Second choice screen to choose between high and low modes
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

void verifyIfCanCharge(){
    if (digitalRead(canChargePin) == HIGH) canCharge = true;
    else canCharge = false;
}

void verifyIfCanDischarge(){
    if (digitalRead(canDischargePin) == HIGH) canDischarge = true;
    else canDischarge = false;
}