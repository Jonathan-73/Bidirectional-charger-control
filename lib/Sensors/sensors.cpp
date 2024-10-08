#include "sensors.h"

double measureDischargingCurrent(){
    double dischargingCurrent = 0;
    double VOutDischarging = 0;
    const double scale_factor = 0.1;
    const double vRef = 5.00;
    const double resConvert = 1024;
    double resADC = vRef/resConvert;
    double zeroPoint = vRef/2;
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
    double VOutCharging = 0;
    double chargingCurrent = 0;
    const double scale_factor = 0.1;
    const double vRef = 5.00;
    const double resConvert = 1024;
    double resADC = vRef/resConvert;
    double zeroPoint = vRef/2;
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