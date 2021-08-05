#include <RF24.h>
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.

#define analogPin1 7
#define analogPin2 8

struct PacketData {  //data comming in
  int left;
  int right;
  uint8_t led;
};

PacketData data;

int middleX = 0;
int middleY = 0;


void setup() {
  //Serial.begin(9600);
  radio.begin();                  //Starting the Wireless communication
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MAX);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();          //This sets the module as transmitter

  delay(100);
  for (int i = 0; i < 50; i++) {
    middleY += analogRead(analogPin1);
    delay(1);
  }
  middleY = middleY / 50;

  for (int i = 0; i < 50; i++) {
    middleX += analogRead(analogPin2);
    delay(1);
  }
  middleX = middleX / 50;
}

void loop()
{
  int y = analogRead(analogPin1);
  int x = analogRead(analogPin2);

  y = map(y, middleY - 390, middleY + 390, -255, 255);
  if (y > 255) {
    y = 255;
  } else if (y < -255) {
    y = -255;
  }

  x = map(x, middleX - 390, middleX + 390, -255, 255) / 2;
  if (x > 255) {
    x = 255;
  } else if (x < -255) {
    x = -255;
  }





  int deadzoneXRadius = 30;
  int deadzoneYRadius = 50;
  if (x > -deadzoneXRadius && x < deadzoneXRadius) {
    x = 0;
  }
  if (y > -deadzoneYRadius && y < deadzoneYRadius) {
    y = 0;
  }

  data.left = y + x;
  data.right = y - x;

  if (data.left > 255) {
    data.left = 255;
  } else if (data.left < -255) {
    data.left = -255;
  }
  if (data.right > 255) {
    data.right = 255;
  } else if (data.right < -255) {
    data.right = -255;
  }
    //Serial.print(data.left);
    //Serial.print(" ");

    //Serial.print(radio.isChipConnected());
    //Serial.print(" ");

    //Serial.print(data.right);
    //Serial.println(" ");

    radio.write(&data, sizeof(PacketData));  //Sending the message to receiver
    //delay(20);
}
