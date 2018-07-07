#include <SoftwareSerial.h>
#include <SPI.h>
#include <RH_NRF24.h>

#define BTTX 2    // HC-06 Bluetooth Tx Pin
#define BTRX 3    // HC-06 Bluetooth Rx Pin
#define Trigger 4 // Hot shoe optocupler
#define Focus 5   // Camera focus optocoupler
#define Shutter 6 // Camera shutter optocoupler
#define Dir 7     // TB6600 Direction + pin
#define NRF3 8    // NRF24L01+ Pin 3 -> CE
#define Pulse 9   // TB6600 Pulse + pin
#define NRF4 10   // NRF24L01+ Pin 4 -> CSN
#define NRF6 11   // NRF24L01+ Pin 6 -> MOSI
#define NRF7 12   // NRF24L01+ Pin 7 -> MISO
#define NRF5 13   // NRF24L01+ Pin 5 -> SCK
#define mts 1600

SoftwareSerial BTserial(BTTX, BTRX);
RH_NRF24 nrf24;

bool debug = false;
bool cledon = false;
bool modeling = false;
bool rvr = false;
int lp = 0;
long stime = 0;
volatile int shtdly = 10;
volatile int speedA = 1600;
volatile int speedB = 1600;
uint8_t data[] = "0";


void setup()
{
  BTserial.begin(9600);
  if (!nrf24.init())
    BTserial.println("init failed");
  if (!nrf24.setChannel(1))
    BTserial.println("setChannel failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    BTserial.println("setRF failed");
  pinMode(Pulse, OUTPUT);
  pinMode(Dir, OUTPUT);
  pinMode(Trigger, OUTPUT);
  pinMode(Shutter, OUTPUT);
  pinMode(Focus, OUTPUT);
  digitalWrite(Dir, LOW);
  digitalWrite(Pulse, LOW);
  BTserial.println("ChroNude initialized!...");
}

void loop()
{
  // str.toCharArray(buf, len);
  if (BTserial.available())
  {
    String str = BTserial.readString();
    if (str.compareTo("h") == 0) {
      BTserial.println(" Hello master :)");
      BTserial.println("Have a nice shooting time! ;-)");
      BTserial.println("h : Show this :))");
      BTserial.println("f : Fire flash!");
      BTserial.println("s : Press shutter button.");
      BTserial.println("l : Test LED.");
      BTserial.println("m : Modeling lamp on/off");
      BTserial.println("r : rvr motor rotation direction");
      BTserial.println("i : Incremental shooting");
      BTserial.println("4-32 : Shooting count");
      BTserial.println("cled : Led on during cross shot");
      BTserial.println("cr1 : Cross shot 11.25°");
      BTserial.println("cr2 : Cross shot 22.5°");
      BTserial.println("cr3 : Cross shot 45°");
      BTserial.println("d0-d8 : Shutter delay 0 to 8 sec.");
    }

    // SHUTTER DELAY
    if (str.substring(0, 1).compareTo("d") == 0) {
      shtdly = str.substring(1).toInt() * 1000;
      BTserial.print("Shutter delay ");
      BTserial.print(str.substring(1).toInt());
      BTserial.print(" sec.");
    }

    // MOTOR REVERSE
    if (str.compareTo("r") == 0) {
      rvr = !rvr;
      BTserial.print("Motor rotation is ");
      if (rvr) {
        BTserial.println("right to left.");
      } else {
        BTserial.println("left to right");
      }
    }

    // FLASH TEST FIRE
    if (str.compareTo("f") == 0) {
      BTserial.println("Flash triggered.");
      data[0] = 70;
      data[1] = 0;
      nrf24.send(data, sizeof(data));
    }

    // LED MODELING ON
    if (str.compareTo("m") == 0) {
      modeling = !modeling;
      BTserial.println("LED Modeling toggled.");
      data[0] = 0;
      data[1] = (modeling) ? lp : 0;
      nrf24.send(data, sizeof(data));
    }

    // LED TEST
    if (str.substring(0, 1).compareTo("l") == 0) {
      if (str.length() > 1 ) {
        lp = 48 + str.substring(1).toInt();
        BTserial.print("LED power is ");
        BTserial.println(lp);
      } else {
        data[0] = 0;
        BTserial.println("Testing LED light.");
        data[1] = 56;
        nrf24.send(data, sizeof(data));
        delay(250);
        data[1] = 53;
        nrf24.send(data, sizeof(data));
        delay(250);
        data[1] = 51;
        nrf24.send(data, sizeof(data));
        delay(250);
        data[1] = 48;
        nrf24.send(data, sizeof(data));
        delay(250);
      }
    }

    // SHUTTER RELEASE TEST
    if (str.compareTo("s") == 0) {
      BTserial.println("Shutter pressed.");
      shutter();
    }

    // speedA
    if (str.substring(0, 2).compareTo("sh") == 0 && (str.substring(2).toInt() > 1199 && str.substring(2).toInt() < 7500)) {
      speedA = str.substring(2).toInt();
      BTserial.print("Motor speed changed to ");
      BTserial.println(speedA);
    }

    // Free rotation
    if (str.substring(0, 1).compareTo("t") == 0 && (str.substring(1).toInt() > 0 && str.substring(1).toInt() < 361)) {
      int iMax = (int) str.substring(1).toInt() / 0.225;
      digitalWrite(Dir, (!rvr) ? LOW : HIGH);
      for (int i = 0; i < iMax ; i++)  {
        motorMove(2400);
      }
    }

    // SPIN SHOOT
    if (str.toInt() >= 4 && str.toInt() <= 32) {
      BTserial.println("Shooting started!");
      if (shtdly > 10) {
        BTserial.print("Waiting shutter delay... (");
        BTserial.print(shtdly / 1000);
        BTserial.println(" sec)");
      }
      delay(shtdly);
      NormalShoot(str.toInt());
    }

    // INCREMENTAL SHOOT
    if (str.compareTo("i") == 0) {
      BTserial.println("Incremental shooting.");
      if (shtdly > 10) {
        BTserial.print("Waiting shutter delay... (");
        BTserial.print(shtdly / 1000);
        BTserial.println(" sec)");
      }
      delay(shtdly);
      CustomShot(1, true);
    }

    // LED ON DURING CROSS SHOOT
    if (str.compareTo("cled") == 0) {
      cledon = !cledon;
      BTserial.print("During cross shot LED is ");
      if (cledon) {
        BTserial.println("on.");
      } else {
        BTserial.println("off.");
      }
    }

    // CROSS SHOOT 11.25°
    if (str.substring(0, 2).compareTo("cr") == 0 && (str.substring(2).toInt() > 0 && str.substring(2).toInt() < 4)) {
      BTserial.println("Cross left shooting.");
      if (shtdly > 10) {
        BTserial.print("Waiting shutter delay... (");
        BTserial.print(shtdly / 1000);
        BTserial.println(" sec)");
      }
      delay(shtdly);
      CustomShot(str.substring(2).toInt(), false);
    }

  } // End of BTserial reading
} // End of loop

void CustomShot(unsigned int smx, bool inc) {
  data[0] = 70; // WiFi Flash trigger command
  data[1] = (cledon) ? lp : 0; // WiFi set LED off
  digitalWrite(Dir, (!rvr) ? HIGH : LOW);
  shutter();
  stime = millis();
  unsigned int nss = 50 * smx;
  unsigned int sps = 50 * smx;
  speedB = speedA;
  for (int i = 1; i < mts + 1 ; i++)  {
    switch (i) {
      case 1:
        sps = (inc) ? 400 : 50 * smx;
        data[1] = 0;
        break;
      case 400:
        sps = (inc) ? 200 : 400;
        data[1] = (cledon) ? lp : 0;
        break;
      case 800:
        sps = (inc) ? 100 : 50 * smx;
        data[1] = 0;
        break;
      case 1200:
        sps = (inc) ? 50 : 400;
        data[1] = (cledon) ? lp : 0;
        break;
    }

    if (i == nss) {
      nrf24.send(data, sizeof(data));
      nss = nss + sps;
    }
    speedB = (i > 1400) ? speedB + 10 : speedB;
    motorMove(speedB);
  }
  // Camera curtain close
  shutter();
  BTserial.print(" Duration: ");
  BTserial.print((millis() - stime) / 1000.00);
  BTserial.print(" sec. ");
  delay(1000);
  motorrvr360();
  BTserial.println("Ready for new shoot!");
}

void NormalShoot(unsigned int ts) {
  data[0] = 0;
  data[1] = 0;
  nrf24.send(data, sizeof(data));
  data[0] = 70;
  digitalWrite(Dir, (!rvr) ? HIGH : LOW);
  unsigned int nss = 0; // Next shhoting step
  unsigned int sc = 1; // Shot counter
  unsigned int sps = mts / ts; // Steps per shot
  speedB = speedA;
  shutter();
  stime = millis();
  for (int i = 1; i < mts + 1 ; i++)  {
    nss = sps * sc;
    speedB = (i > 1400) ? speedB + 10 : speedB;
    if (i == nss) {
      nrf24.send(data, sizeof(data));
      sc = sc + 1;
    }
    motorMove(speedB);
  }
  // Camera curtain close
  shutter();
  BTserial.print(" Duration: ");
  BTserial.print((millis() - stime) / 1000.00);
  BTserial.print(" sec. ");
  delay(1000);
  motorrvr360();
  BTserial.println("Ready for new shoot!");
}

void motorMove(unsigned int s) {
  digitalWrite(Pulse, HIGH);
  delayMicroseconds(s);
  digitalWrite(Pulse, LOW);
  delayMicroseconds(s);
}

void motorrvr360() {
  data[0] = 0;
  data[1] = (modeling && lp > 0) ? lp : 0;
  nrf24.send(data, sizeof(data));
  speedB = speedA;
  digitalWrite(Dir, (!rvr) ? LOW : HIGH);
  for (int i = 1; i < mts + 1; i++)  {
    speedB = (i > 1400) ? speedB + 10 : speedB;
    motorMove(speedB);
  }
}

void shutter() {
  digitalWrite(Focus, HIGH);
  delay(10);
  digitalWrite(Shutter, HIGH);
  delay(50);
  digitalWrite(Focus, LOW);
  digitalWrite(Shutter, LOW);
}

