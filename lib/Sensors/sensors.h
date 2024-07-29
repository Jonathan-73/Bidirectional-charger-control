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

/**
 * @brief Function which measures the current
 * between the battery and the onduler
 * @return a double giving the current in Amperes
 */
double measureDischargingCurrent();

/**
 * @brief Function which measures the current
 * between the battery and the charger
 * @return a double giving the current in Amperes
 */
double measureChargingCurrent();