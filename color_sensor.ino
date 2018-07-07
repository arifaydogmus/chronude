/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com
*********/
#include <SoftwareSerial.h>
#include <Nextion.h>

SoftwareSerial nextion(11, 12);// Nextion TX to pin 2 and RX to pin 3 of Arduino
Nextion myNextion(nextion, 9600);

// TCS230 or TCS3200 pins wiring to Arduino
enum Colours {RED, GREEN, BLUE, CLEAR};
#define S0 3
#define S1 4
#define S2 5
#define S3 6
#define sensorOut 9
#define sensorLeds 7

// Stores frequency read by the photodiodes
byte R;
byte G;
byte B;
byte C;

unsigned long fR;
unsigned long fG;
unsigned long fB;
unsigned long fC;

String rgbColor = "";

void setup() {
  // Setting the outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorLeds, OUTPUT);

  // Setting the sensorOut as an input
  pinMode(sensorOut, INPUT);

  // Setting frequency scaling to 20%
  digitalWrite(S0, LOW);
  digitalWrite(S1, HIGH);

  // Turn off LEDs
  digitalWrite(sensorLeds, LOW);
  // Begins serial communication
  Serial.begin(115200);
  //myNextion.init();
}

void loop() {
  ReadColour(RED);
  ReadColour(GREEN);
  ReadColour(BLUE);
  ReadColour(CLEAR);

  // Printing the RED (R) value
  Serial.print(fR);
  Serial.print("\t");
  Serial.print(fG);
  Serial.print("\t");
  Serial.print(fB);
  Serial.print("\t");
  Serial.print(fC);
  Serial.print("----");
  Serial.print(R);
  Serial.print("\t");
  Serial.print(G);
  Serial.print("\t");
  Serial.print(B);
  Serial.print("\t");
  Serial.println(C);
  rgbColor = "";
  rgbColor.concat(R);
  rgbColor.concat("-");
  rgbColor.concat(G);
  rgbColor.concat("-");
  rgbColor.concat(B);
  rgbColor.concat("-");
  rgbColor.concat(C);

/*
  uint16_t rgb565Color = rgb565(R, G, B);
  String nextionColor = "b0.bco=";
  nextionColor.concat(rgb565Color);
  myNextion.setComponentText("t0", rgbColor);
  myNextion.sendCommand(nextionColor.c_str());
  */
  delay(1);
}


uint16_t rgb565(int R, int G, int B)
{
  uint16_t ret  = (R & 0xF8) << 8;  // 5 bits
  ret |= (G & 0xFC) << 3;  // 6 bits
  ret |= (B & 0xF8) >> 3;  // 5 bits

  return ( ret);
}

void ReadColour(byte Colour)
{
  switch (Colour)
  {
    case RED:
      digitalWrite(S2, LOW);
      digitalWrite(S3, LOW);
      fR = pulseIn(sensorOut, HIGH);
      R = map(fR, 0, 25000, 255, 0);
      break;

    case GREEN:
      digitalWrite(S2, HIGH);
      digitalWrite(S3, HIGH);
      fG = pulseIn(sensorOut, HIGH);
      G = map(fG, 0, 25000, 255, 0);
      break;

    case BLUE:
      digitalWrite(S2, LOW);
      digitalWrite(S3, HIGH);
      fB = pulseIn(sensorOut, HIGH);
      B = map(fB, 0, 25000, 255, 0);
      break;

    case CLEAR:
      digitalWrite(S2, HIGH);
      digitalWrite(S3, LOW);
      fC = pulseIn(sensorOut, HIGH);
      C = map(fC, 0, 25000, 255, 0);
      break;
  }
}

uint16_t calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
{
  float X, Y, Z;      /* RGB to XYZ correlation      */
  float xc, yc;       /* Chromaticity co-ordinates   */
  float n;            /* McCamy's formula            */
  float cct;

  /* 1. Map RGB values to their XYZ counterparts.    */
  /* Based on 6500K fluorescent, 3000K fluorescent   */
  /* and 60W incandescent values for a wide range.   */
  /* Note: Y = Illuminance or lux                    */
  X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
  Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
  Z = (-0.68202F * r) + (0.77073F * g) + ( 0.56332F * b);

  /* 2. Calculate the chromaticity co-ordinates      */
  xc = (X) / (X + Y + Z);
  yc = (Y) / (X + Y + Z);

  /* 3. Use McCamy's formula to determine the CCT    */
  n = (xc - 0.3320F) / (0.1858F - yc);

  /* Calculate the final CCT */
  cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n) + 5520.33F;

  /* Return the results in degrees Kelvin */
  Serial.print(cct);
  Serial.print("K");
  Serial.println("");
  return (uint16_t)cct;
}

uint16_t calculateLux(uint16_t r, uint16_t g, uint16_t b)
{
  float illuminance;

  /* This only uses RGB ... how can we integrate clear or calculate lux */
  /* based exclusively on clear since this might be more reliable?      */
  illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
  Serial.print(illuminance);
  Serial.print("Lux");
  Serial.println("");
  return (uint16_t)illuminance;
}
