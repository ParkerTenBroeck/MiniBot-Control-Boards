/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.

    rf24ping85.ino by tong67 ( https://github.com/tong67 )
    This is an example of how to use the RF24 class to communicate with ATtiny85 and other node.
    Write this sketch to an ATtiny85. It will act like the 'transmit' mode of GettingStarted.ino
    Write GettingStarted.ino sketch to UNO (or other board or RPi) and put the node in 'receiver' mode.
    The ATtiny85 will transmit a counting number every second starting from 1.
    The ATtiny85 uses the tiny-core by CodingBadly (https://code.google.com/p/arduino-tiny/)
    When direct use of 3v3 does not work (UNO boards have bad 3v3 line) use 5v with LED (1.8V ~ 2.2V drop)
    For low power consumption solutions floating pins (SCK and MOSI) should be pulled high or low with eg. 10K

    ** Hardware configuration **
    ATtiny25/45/85 Pin map with CE_PIN 3 and CSN_PIN 4
                                 +-\/-+
                   NC      PB5  1|o   |8  Vcc --- nRF24L01  VCC, pin2 --- LED --- 5V
    nRF24L01  CE, pin3 --- PB3  2|    |7  PB2 --- nRF24L01  SCK, pin5
    nRF24L01 CSN, pin4 --- PB4  3|    |6  PB1 --- nRF24L01 MOSI, pin7
    nRF24L01 GND, pin1 --- GND  4|    |5  PB0 --- nRF24L01 MISO, pin6
                                 +----+

    ATtiny25/45/85 Pin map with CE_PIN 3 and CSN_PIN 3 => PB3 and PB4 are free to use for application
    Circuit idea from http://nerdralph.blogspot.ca/2014/01/nrf24l01-control-with-3-attiny85-pins.html
    Original RC combination was 1K/100nF. 22K/10nF combination worked better.
	For best settletime delay value in RF24::csn() the timingSearch3pin.ino scatch can be used.
    This configuration is enabled when CE_PIN and CSN_PIN are equal, e.g. both 3
    Because CE is always high the power consumption is higher than for 5 pins solution
                                                                                            ^^
                                 +-\/-+           nRF24L01   CE, pin3 ------|              //
                           PB5  1|o   |8  Vcc --- nRF24L01  VCC, pin2 ------x----------x--|<|-- 5V
                   NC      PB3  2|    |7  PB2 --- nRF24L01  SCK, pin5 --|<|---x-[22k]--|  LED
                   NC      PB4  3|    |6  PB1 --- nRF24L01 MOSI, pin6  1n4148 |
    nRF24L01 GND, pin1 -x- GND  4|    |5  PB0 --- nRF24L01 MISO, pin7         |
                        |        +----+                                       |
                        |-----------------------------------------------||----x-- nRF24L01 CSN, pin4
                                                                       10nF

    ATtiny24/44/84 Pin map with CE_PIN 8 and CSN_PIN 7
	Schematic provided and successfully tested by Carmine Pastore (https://github.com/Carminepz)
                                  +-\/-+
    nRF24L01  VCC, pin2 --- VCC  1|o   |14 GND --- nRF24L01  GND, pin1
                            PB0  2|    |13 AREF
                            PB1  3|    |12 PA1
                            PB3  4|    |11 PA2 --- nRF24L01   CE, pin3
                            PB2  5|    |10 PA3 --- nRF24L01  CSN, pin4
                            PA7  6|    |9  PA4 --- nRF24L01  SCK, pin5
    nRF24L01 MOSI, pin7 --- PA6  7|    |8  PA5 --- nRF24L01 MISO, pin6
                                  +----+
*/

// CE and CSN are configurable, specified values for ATtiny85 as connected above
//#  define DI   6   // PA6 - Physical pin 7 - USI Data Input pin - Connect to radio MISO
//# define DO   5   // PA5 - Physical pin 8 - USI Data Output pin - Connect to radio MOSI
//# define USCK 4   // PA4 - Physical pin 9 - USI Clock pin - Connect to radio SCK
//# define SS   3   // (not used by USI)

//#  define DI   4   // PA6
//# define DO   5   // PA5
//# define USCK 6   // PA4

#define NUMBER_OF_SHIFT_CHIPS   1
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8
#define PULSE_WIDTH_USEC   5
#define POLL_DELAY_MSEC   1

#define ploadPin       1  // Connects to Parallel load pin the 165
#define clockPin       2 // Connects to the Clock pin the 165
#define dataPin        3 // Connects to the Q7 pin the 165

#define CE_PIN 9
#define CSN_PIN 10
//#define CSN_PIN 3 // uncomment for ATtiny85 3 pins solution

#include "RF24.h"
#include <Servo.h>

RF24 radio(CE_PIN, CSN_PIN);

const byte address[6] = "00001";

#define led_pin 3

struct PacketData {  //data comming in
  int left;
  int right;
  uint8_t led;
};

PacketData data;


void setup() {

  uint8_t mSpeed = readBoard();
  mSpeed = 255;

  pinMode(ploadPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);

  analogWrite(8, 128);


  analogWrite(7, 0);
  analogWrite(8, 0);
  radio.begin();
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MAX);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();              //This sets the module as receiver
  delay(1000);
}
void resetRadio() {
  //asm volatile ("  jmp 0"); //for now just reset arduino
}

void loop() {
  readRadio();
  //uint8_t mSpeed =  readBoard();
  //analogWrite(7, mSpeed);

}
void readRadio() {

  //    if(radio.isChipConnected()){
  //    analogWrite(8, 128);
  //    return;
  //  }else{
  //    analogWrite(8, 0);
  //    return;
  //  }

  uint8_t mSpeed = readBoard();

  long startMicros = micros();
  while (!radio.available()) {
    if ((startMicros + 100000) < micros()) {
      analogWrite(7, 0);
      analogWrite(8, 0);
    }
    if ((startMicros + 200000) < micros()) {
      resetRadio();
    }
  }
  if (radio.available()) {             //Looking for the data.

    radio.read(&data, sizeof(PacketData));    //Reading the data

    data.left = data.left / (255 - (mSpeed - 1));
    data.right = data.right / (255 - (mSpeed - 1));

    data.left = map(data.left, -255, 255, 130, 244);
    data.right = map(data.right, -255, 255, 244, 130);

    analogWrite(7,  data.left);
    analogWrite(8, data.right);
  }

}

uint8_t readBoard()
{
  long bitVal;
  uint8_t bytesVal = 0;

  /* Trigger a parallel Load to latch the state of the data lines,
  */
  digitalWrite(ploadPin, LOW);
  delayMicroseconds(PULSE_WIDTH_USEC);
  digitalWrite(ploadPin, HIGH);


  /* Loop to read each bit value from the serial out line
     of the SN74HC165N.
  */
  for (int i = 0; i < DATA_WIDTH; i++)
  {
    bitVal = digitalRead(dataPin);

    /* Set the corresponding bit in bytesVal.
    */
    bytesVal |= (bitVal << ((DATA_WIDTH - 1) - i));

    /* Pulse the Clock (rising edge shifts the next bit).
    */
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(clockPin, LOW);
  }

  return (bytesVal);
}
