#include <Wire.h>
#include "Adafruit_TCS34725.h"

Adafruit_TCS34725 tcs = Adafruit_TCS34725();

// connect LED to digital 4 or GROUND for ambient light sensing
// connect SCL to analog 5
// connect SDA to analog 4
// connect Vin to 3.3-5V DC
// connect GROUND to common ground

// some magic numbers for this device from the DN40 application note
#define TCS34725_R_Coef 0.136
#define TCS34725_G_Coef 1.000
#define TCS34725_B_Coef -0.444
#define TCS34725_GA 1.0
#define TCS34725_DF 310.0
#define TCS34725_CT_Coef 3810.0
#define TCS34725_CT_Offset 1391.0



boolean isAvailable, isSaturated;
uint16_t againx, atime, atime_ms, aLux;
uint16_t r, g, b, c;
uint16_t ir;
uint16_t r_comp, g_comp, b_comp, c_comp;
uint16_t saturation, saturation75, colorTemp;
float cratio, cpl, ct, lux, maxlux;
float vR, vG, vB;


void setup(void) {
  Serial.begin(115200);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  pinMode(4, OUTPUT);
  pinMode(2, INPUT);
  digitalWrite(4, LOW); // @gremlins Bright light, bright light!

  againx = 1;
  atime = int(TCS34725_INTEGRATIONTIME_2_4MS);
  atime_ms = ((256 - atime) * 2.4);
  Serial.println("ready...");
  getData();
  Serial.print(F("Ambient Lux:"));
  Serial.println(lux);


}

void loop(void) {
  if (digitalRead(2) == HIGH) {
    for (int i = 0; i < 1000; i++) {
      getData();
      if (ct > 5000 && ct < 6000) {
        Serial.print(F("Raw: "));
        Serial.print(r);
        Serial.print(F("-"));
        Serial.print(g);
        Serial.print(F("-"));
        Serial.print(b);
        Serial.print(F("-"));
        Serial.print(c);

        Serial.print(F("\tCompensated:"));
        Serial.print(r_comp);
        Serial.print(F("-"));
        Serial.print(g_comp);
        Serial.print(F("-"));
        Serial.print(b_comp);
        Serial.print(F("-"));
        Serial.print(c_comp);

        Serial.print(F("\tIR:"));
        Serial.print(ir);
        Serial.print(F(" CRATIO:"));
        Serial.print(cratio);
        Serial.print(F(" Sat:"));
        Serial.print(saturation);
        Serial.print(F(" Sat75:"));
        Serial.print(saturation75);
        Serial.print(F(" "));
        Serial.print(isSaturated ? "*SATURATED*" : "");
        Serial.print(F("CPL:"));
        Serial.print(cpl);
        Serial.print(F(" Max lux:"));
        Serial.print(maxlux);
        Serial.print(F("Lux:"));
        Serial.print(lux);
        Serial.print(F(" CT:"));
        Serial.println(ct);
 

      }
    }
    Serial.println(F("---------------------------------------------------------"));
  }

}


void getData(void) {
  // read the sensor and autorange if necessary
  tcs.getRawData(&r, &g, &b, &c);
  aLux = tcs.calculateLux(r, g, b);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  // DN40 calculations
  ir = (r + g + b > c) ? (r + g + b - c) / 2 : 0;
  r_comp = r - ir;
  g_comp = g - ir;
  b_comp = b - ir;
  c_comp = c - ir;
  cratio = float(ir) / float(c);

  vR = r /= c;
  vG = g /= c;
  vB = b /= c;
  vR *= 256; 
  vG *= 256; 
  vB *= 256;

  saturation = ((256 - atime) > 63) ? 65535 : 1024 * (256 - atime);
  saturation75 = (atime_ms < 150) ? (saturation - saturation / 4) : saturation;
  isSaturated = (atime_ms < 150 && c > saturation75) ? 1 : 0;
  cpl = (atime_ms * againx) / (TCS34725_GA * TCS34725_DF);
  maxlux = 65535 / (cpl * 3);

  lux = (TCS34725_R_Coef * float(r_comp) + TCS34725_G_Coef * float(g_comp) + TCS34725_B_Coef * float(b_comp)) / cpl;
  ct = TCS34725_CT_Coef * float(b_comp) / float(r_comp) + TCS34725_CT_Offset;
  
}
