#include <Arduino.h>
#include <sensors.h>
#include "Wire.h"
#include <SPI.h>
#include <mcp2515.h>
#include <LiquidCrystal_I2C.h>

#define ON 0
#define OFF 1

const int button1 = A3;
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

int SOCBattery = 0;

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

byte bottomArrow[8] = {      //Caracter for bottom arrow sign on LCD screen
    0b00000,
    0b00100,
    0b00100,
    0b00100,
    0b10101,
    0b01110,
    0b00100,
    0b00000
};

byte topArrow[8] = {      //Caracter for top arrow sign on LCD screen
    0b00000,
    0b00100,
    0b01110,
    0b10101,
    0b00100,
    0b00100,
    0b00100,
    0b00000
};

LiquidCrystal_I2C LCD(0x27,16,2); // definition of the screen with its adress and size

struct can_frame canMsg;    //Structure of an object which will receipt CAN messages
MCP2515 mcp2515(10);        //Object for controlling the MCP2515 module

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

/**
 * @brief Function which verifies if the grid is connected
 */
void verifyIfGridConnected();

/**
 * @brief Function which reads via the CAN connection the SOC of the battery
 * @return a percentage of the SOC of the battery
 */
int readSOCFromCAN();

void setup() {
    Serial.begin(115200);
    //Definition of the Arduino pins as digital outputs
    pinMode(relayChargeDischarge, OUTPUT); //Relay for charge/discharge
    pinMode(relayGridPower, OUTPUT); //Relay for grid powering
    pinMode(relayChOnOff, OUTPUT); //Relay for activating the charger
    pinMode(relayChHighLow, OUTPUT); //Relay for choosing HIGH/LOW mode of the charger
    setupCheckSystem();

    pinMode(button1, INPUT);    //Definition of the 2 buttons as an input
    pinMode(button2, INPUT);    //for the HMI

    pinMode(gridPoweredSensor, INPUT);  //Pin for the binary sensor which verifies if the grid is powered

    pinMode(canChargePin, INPUT_PULLUP);       //pins connected to the relays of the BMS
    pinMode(canDischargePin, INPUT_PULLUP);    //which allows or not charge or discharge

    mcp2515.reset();                            //Setting of the CAN communication
    mcp2515.setBitrate(CAN_125KBPS, MCP_8MHZ);  //with a bitrate of 125kbit/s and a clock of 8MHz
    mcp2515.setNormalMode();

    LCD.init(); // initialisation of the screen
    LCD.createChar(1, ok);
    LCD.createChar(2, bottomArrow);
    LCD.createChar(3, topArrow);
    LCD.backlight();
    LCD.display();
    LCD.clear();
    LCD.print("starting");
}

void loop() {
    //Setup check system
    setupCheckSystem();
    //End of setup check system
    do{
        verifyIfGridConnected();
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
        while(canCharge && isGridConnected){
            digitalWrite(relayGridPower, ON);   //Grid powered
            digitalWrite(relayChOnOff, ON);  //Charger set on
            //Choosing charger speed
            if(highLowMode == M_LOW) {  //Slow charge
                Serial.println("Slow charge");
                measuredChargingCurrent = measureChargingCurrent();
                LCD.clear();
                LCD.print("SOC:");
                LCD.setCursor(4,0);
                SOCBattery = readSOCFromCAN();
                if(SOCBattery != -1) {
                    LCD.print(SOCBattery);
                    LCD.setCursor(7,0);
                    LCD.print("%");
                } else LCD.print("NA");
                LCD.setCursor(9,0);
                LCD.print(char(2));
                LCD.setCursor(10,0);
                LCD.print(measuredChargingCurrent);
                LCD.setCursor(15,0);
                LCD.print("A");
                LCD.setCursor(0,1);
                LCD.print("SLOW CHARGING");
                delay(200);
            }
            if (highLowMode == M_HIGH){    //Speed charge
                while(canCharge && isGridConnected){
                    digitalWrite(relayChHighLow, ON);   //Charger in HIGH mode
                    Serial.println("Fast charge");
                    measuredChargingCurrent = measureChargingCurrent();
                    LCD.clear();
                    LCD.print("SOC:");
                    LCD.setCursor(4,0);
                    SOCBattery = readSOCFromCAN();
                    if(SOCBattery != -1) {
                        LCD.print(SOCBattery);
                        LCD.setCursor(7,0);
                        LCD.print("%");
                    } else LCD.print("NA");
                    LCD.setCursor(9,0);
                    LCD.print(char(2));
                    LCD.setCursor(10,0);
                    LCD.print(measuredChargingCurrent);
                    LCD.setCursor(15,0);
                    LCD.print("A");
                    LCD.setCursor(0,1);
                    LCD.print("FAST CHARGING");
                    delay(200);
                    verifyIfCanCharge();
                    verifyIfGridConnected();
                }
                digitalWrite(relayChOnOff, OFF);  //Charger set off
                LCD.clear();
                LCD.print("Balancing");
                for(int i = 0; i < 60; i++){    //Waiting 1 minute for balancing of battery
                    delay(1000);
                    verifyIfCanCharge();    //If battery not full
                    if(canCharge) break;
                }
                while(canCharge && isGridConnected){       //Finishing the charge in slow mode
                    digitalWrite(relayChHighLow, OFF);   //Charger in LOW mode
                    digitalWrite(relayChOnOff, ON);  //Charger set on
                    Serial.println("Slow charge");
                    measuredChargingCurrent = measureChargingCurrent();
                    LCD.clear();
                    LCD.print("SOC:");
                    LCD.setCursor(4,0);
                    SOCBattery = readSOCFromCAN();
                    if(SOCBattery != -1) {
                        LCD.print(SOCBattery);
                        LCD.setCursor(7,0);
                        LCD.print("%");
                    } else LCD.print("NA");
                    LCD.setCursor(9,0);
                    LCD.print(char(2));
                    LCD.setCursor(10,0);
                    LCD.print(measuredChargingCurrent);
                    LCD.setCursor(15,0);
                    LCD.print("A");
                    LCD.setCursor(0,1);
                    LCD.print("SLOW CHARGING");
                    delay(200);
                    verifyIfCanCharge();
                    verifyIfGridConnected();
                }
            }
            verifyIfCanCharge();
            verifyIfGridConnected();
        }
        LCD.clear();
        LCD.print("BATTERY FULL");
        delay(2000);
    }
    if (chargeDischarge == DISCHARGE){  //Discharging mode
        while(canDischarge && isGridConnected){
            digitalWrite(relayChargeDischarge, ON);   //Discharging mode
            digitalWrite(relayGridPower, ON);   //Grid powered
            Serial.println("Entering discharging mode");
            measuredDischargingCurrent = measureDischargingCurrent();
            LCD.clear();
            LCD.print("SOC:");
            LCD.setCursor(4,0);
            SOCBattery = readSOCFromCAN();
            if(SOCBattery != -1) {
                LCD.print(SOCBattery);
                LCD.setCursor(7,0);
                LCD.print("%");
            } else LCD.print("NA");
            LCD.setCursor(9,0);
            LCD.print(char(3));
            LCD.setCursor(10,0);
            LCD.print(measuredDischargingCurrent);
            LCD.setCursor(15,0);
            LCD.print("A");
            LCD.setCursor(0,1);
            LCD.print("DISCHARGING");
            delay(200);
            verifyIfCanDischarge();
            verifyIfGridConnected();
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
    Serial.println("Setup check system ends");
}

void chooseMode(){
    bool choiceChDch = CHARGE;
    bool choiceMode = M_LOW;
    button2State = LOW;
    while(button2State != HIGH){    //First selection screen
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
        LCD.setCursor(0,1);
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
        LCD.setCursor(0,1);
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
    if (digitalRead(canChargePin) == LOW) canCharge = true;
    else canCharge = false;
}

void verifyIfCanDischarge(){
    if (digitalRead(canDischargePin) == LOW) canDischarge = true;
    else canDischarge = false;
}

void verifyIfGridConnected(){
    if(digitalRead(gridPoweredSensor) == LOW) isGridConnected = true;
    else isGridConnected = false;
}

int readSOCFromCAN(){
    int SOC = -1;
    while(SOC == -1){
        if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
            if(canMsg.can_id == 0x11){
                SOC = int(canMsg.data[6] / 0x64 * 100);
                Serial.print(" SOC = ");
                Serial.print(SOC);
                Serial.println("%");
            }
        }
    }
    return SOC;
}