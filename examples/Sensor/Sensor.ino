/**
 * Example sensor Arduino sketch.
 * This sketch shows how a sensor node can be programmed.
 * It reads an analog signal, it sends the value on the network
 * then it sleeps for some time.
 */
#include <Arduino.h>
#include <SPI.h>
#include <nRF24.h>
#include <pIoT_Energy.h>
#include <pIoT_Protocol.h>

/** Address of this node.
 */
long nodeAddress = 1234;

/** Time, in seconds, the sensor will sleep before
 * sending another measurement.
 */
int sleepTime = 5;

/** Definition of a "hello message"
 * that contains internal values of the
 * node. Hello message is recommended to
 * be used on all nodes.
 */
unsigned int helloMsgType = 1;
struct helloMessage {
  float internalTemp;
  float internalVcc;
  unsigned long operationTime;
  unsigned long sentMsgs;
  unsigned long unsentMsgs;
  unsigned long receivedMsgs;
};

/** Definition of a message that contains
 * the value of measured light intensity.
 */
unsigned int lightMsgType = 100;
struct lightMessage {
  int intensity;
};


void setup() {
  Serial.begin(57600);
  Serial.println("pIoT example, acting as Sensor");

  if (!startRadio(9, 10, 8, nodeAddress)) Serial.println("Cannot start radio");
}


void loop() {
  //The loop sends an hello message,
  //then reads an analog value and sends it as light
  //intensity in a light message, then goes to sleep for
  //a certain time
  Serial.println("Sending hello");
  helloMessage hm;
  hm.internalTemp = getInternalTemperature();
  hm.internalVcc = getInternalVcc();
  hm.operationTime = (millis() / 1000) + getTotalSleepSeconds();
  hm.sentMsgs = getSentCounter();
  hm.unsentMsgs = getUnsentCounter();
  hm.receivedMsgs = getReceivedCounter();
  if (!send(false, BASE_ADDR, helloMsgType, (byte*) &hm, sizeof(helloMessage))) {
    Serial.println("- Cannot send message");
  }

  Serial.print("Sending light intensity ");
  int intensity = analogRead(0);
  Serial.println(intensity);
  lightMessage lm;
  lm.intensity = intensity;
  if (!send(false, BASE_ADDR, lightMsgType, (byte*) &lm, sizeof(lightMessage))) {
    Serial.println("- Cannot send message");
  }

  Serial.println("going to sleep for some seconds...");
  delay(100); //this delay is to let the serial send the debug message
  stopRadio(); //you have to shut down the radio explicitly
  sleepUntil(sleepTime, 0);
}

