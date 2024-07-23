#include <Arduino.h>

enum ch_dch {CHARGE, DISCHARGE};
enum high_low {M_HIGH, M_LOW};
bool Charge_Discharge = CHARGE;
bool HIGH_LOW_Mode = LOW;

int RelayControl1 = 4; // Digital Arduino Pins used to activate the relays
int RelayControl2 = 5;
int RelayControl3 = 6;
int RelayControl4 = 7;

void setup() {
    // Definition of the Arduino pins as digital outputs
    pinMode(RelayControl1, OUTPUT); //Relay for charge/discharge
    pinMode(RelayControl2, OUTPUT); //Relay for grid powering
    pinMode(RelayControl3, OUTPUT); //Relay for activating the charger
    pinMode(RelayControl4, OUTPUT); //Relay for choosing HIGH/LOW mode of the charger
}

void loop() {
    //Setup check system
    digitalWrite(RelayControl2, LOW);   //Grid not powered
    digitalWrite(RelayControl1, LOW);   //Charging mode
    digitalWrite(RelayControl3, HIGH);  //Charger set off
    digitalWrite(RelayControl4, HIGH);  //Charger in LOW mode
    //End of setup check system

    //Choosing mode
    if (Charge_Discharge == CHARGE){  //Charging mode
        digitalWrite(RelayControl2, HIGH);   //Grid powered
        digitalWrite(RelayControl3, LOW);  //Charger set on
        //Choosing charger speed
        if(HIGH_LOW_Mode == M_LOW) {  //Slow charge
            delay(30000);   //Slow charge during 30 secondes
        }
        else if (HIGH_LOW_Mode == M_HIGH){    //Speed charge
            digitalWrite(RelayControl4, LOW);   //Charger in HIGH mode
            delay(30000);   //Fast charge during 30 secondes
        }
    }

    else if (Charge_Discharge == DISCHARGE){  //Discharging mode
        digitalWrite(RelayControl1, HIGH);   //Discharging mode
        digitalWrite(RelayControl2, HIGH);   //Grid powered
        delay(30000);   //Discharging during 30 secondes
    }
}
