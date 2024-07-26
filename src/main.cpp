#include <Arduino.h>

//Pin definition
#define chargingSensor A0
#define dischargingSensor A1
#define gridPoweredSensor A2

// Variables for Measured Voltage and Calculated Current
double VOutCharging = 0;
double VOutDischarging = 0;
double chargingCurrent = 0;
double dischargingCurrent = 0;
double measuredDischargingCurrent = 0;
double measuredChargingCurrent = 0;

// Constants for Scale Factor
// Use one that matches your version of ACS712

//const double scale_factor = 0.185; // 5A
const double scale_factor = 0.1; // 20A
//const double scale_factor = 0.066; // 30A

// Constants for A/D converter resolution
// Arduino has 10-bit ADC, so 1024 possible values
// Reference voltage is 5V if not using AREF external reference
// Zero point is half of Reference Voltage

const double vRef = 5.00;
const double resConvert = 1024;
double resADC = vRef/resConvert;
double zeroPoint = vRef/2;

double measureDischargingCurrent();
double measureChargingCurrent();

void setup(){ 
    Serial.begin(9600);
    pinMode(gridPoweredSensor, INPUT);
}

void loop(){
    measuredChargingCurrent = measureChargingCurrent();
    measuredDischargingCurrent = measureDischargingCurrent();
    if(digitalRead(gridPoweredSensor) == HIGH) Serial.println("Grid powered");
    else Serial.println("Grid not powered");                           

    delay(1000); 
}

double measureDischargingCurrent(){
    // Vout is read 1000 Times for precision
    for(int i = 0; i < 1000; i++) {
        VOutDischarging = (VOutDischarging + (resADC * analogRead(dischargingSensor))); 
        delay(1);
    }
  
    // Get Vout in mv
    VOutDischarging = VOutDischarging /1000;
  
    // Convert Vout into Current using Scale Factor
    dischargingCurrent = (VOutDischarging - zeroPoint)/ scale_factor;                   

    // Print Vout and Current;                  

    Serial.print("Vout discharging = ");           
    Serial.print(VOutDischarging,2); 
    Serial.print(" V");                            
    Serial.print("\t Discharging current = ");                  
    Serial.print(dischargingCurrent,2);
    Serial.println(" A");
    return dischargingCurrent;
}

double measureChargingCurrent(){
    // Vout is read 1000 Times for precision
    for(int i = 0; i < 1000; i++) {
        VOutCharging = (VOutCharging + (resADC * analogRead(chargingSensor))); 
        delay(1);
    }
  
    // Get Vout in mv
    VOutCharging = VOutCharging /1000;
  
    // Convert Vout into Current using Scale Factor
    chargingCurrent = (VOutCharging - zeroPoint)/ scale_factor;                   

    // Print Vout and Current;                  

    Serial.print("Vout charging = ");           
    Serial.print(VOutCharging,2); 
    Serial.print(" V");                            
    Serial.print("\t Charging current = ");                  
    Serial.print(chargingCurrent,2);
    Serial.println(" A");
    return chargingCurrent;
}