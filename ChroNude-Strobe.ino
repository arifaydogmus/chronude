#include <SPI.h>
#include <RH_NRF24.h>
#define Trigger 7
#define Led 9

RH_NRF24 nrf24;

bool is_connected = false;
unsigned int ledPower = 0;

void setup()
{
  Serial.begin(9600);
  //Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  //Serial.println("setChannel failed");
  //Serial.println("setRF failed");
  Serial.println("started");
  nrf24.init();
  nrf24.setChannel(1);
  nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm);
  pinMode(Led, OUTPUT);
  pinMode(Trigger, OUTPUT);
  digitalWrite(Trigger, LOW);
}

void loop()
{
  if (nrf24.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (nrf24.recv(buf, &len))
    {
      
            Serial.print("got request: ");
            Serial.println((char*)buf);
            for (uint8_t j = 0; j < len; ++j)
            {
              Serial.print(buf[j], DEC);
              Serial.print(" ");
            }
            Serial.println("");
      
      switch (buf[1] ) {
        case 0:
        case 48:
          ledPower = 0;
          analogWrite(Led, ledPower);
          break;
        case 49:
          ledPower = 16;
          analogWrite(Led, ledPower);
          break;
        case 50:
          ledPower = 32;
          analogWrite(Led, ledPower);
          break;
        case 51:
          ledPower = 48;
          analogWrite(Led, ledPower);
          break;
        case 52:
          ledPower = 64;
          analogWrite(Led, ledPower);
          break;
        case 53:
          ledPower = 128;
          analogWrite(Led, ledPower);
          break;
        case 54:
          ledPower = 192;
          analogWrite(Led, ledPower);
          break;
        case 55:
          ledPower = 224;
          analogWrite(Led, ledPower);
          break;
        case 56:
          ledPower = 255;
          analogWrite(Led, ledPower);
          break;
      }

      if (buf[0] == 70) {
        digitalWrite(Trigger, HIGH);
        delayMicroseconds(400);
        digitalWrite(Trigger, LOW);
        //Serial.println("Fired");
      }
    }
    else
    {
      //Serial.println("recv failed");
    }
  }
}

