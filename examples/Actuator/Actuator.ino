/**
 * Example actuator Arduino sketch.
 * This sketch shows how an actuator node can be programmed.
 * It waits for incoming messages, parses them and activates
 * a digital pin accordingly.
 */
#include <Arduino.h>
#include <SPI.h>
#include <nRF24.h>
#include <pIoT_Energy.h>
#include <pIoT_JSON.h>
#include <pIoT_Protocol.h>


/** Address of this node.
 */
long nodeAddress = 4321;

/** Time, in seconds, between hello messages.
 */
int helloPeriod = 10;

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

/** Definition of the message that contains
 * the information about the status of a switch.
 */
unsigned int switchMsgType = 101;
struct switchMessage {
  boolean on;
};


void setup() {
  Serial.begin(57600);

  long myAddress = nodeAddress;

  Serial.println("pIoT example, acting as Actuator");

  if (!startRadio(9, 10, myAddress)) Serial.println("Cannot start radio");
}

/** Handles incoming messages from the network.
 * Particularly parses the switch messages, actuates
 * accordingly and sends a status message back for
 * confirmation.
 */
void handleSwitchMessage(boolean broadcast, long sender, unsigned int msgType, byte* data, int len) {
  if ((msgType == switchMsgType) &&
      (len == sizeof(switchMessage))) {
    Serial.print("Received a switch message, status: ");
    switchMessage pkt = *((switchMessage*)data);
    digitalWrite(5, pkt.on);
    Serial.println(pkt.on);
    if (!send(false, BASE_ADDR, switchMsgType, (byte*)&pkt, sizeof(switchMessage))) {
      Serial.println("- Cannot send confirmation message");
    }
  } else {
    Serial.println("Received something that I cannot interpret");
  }
}

void loop() {
  //The loop sends an hello message every now and then
  //and just waits for incoming messages
  Serial.println("Sending hello");
  helloMessage hm;
  hm.internalTemp = getInternalTemperature();
  hm.internalVcc = getInternalVcc();
  hm.operationTime = (millis() / 1000) + getTotalSleepSeconds();
  hm.sentMsgs = getSentCounter();
  hm.unsentMsgs = getUnsentCounter();
  hm.receivedMsgs = getReceivedCounter();
  if (!send(false, BASE_ADDR, helloMsgType, (byte*) &hm, sizeof(helloMessage))) {
    Serial.println("- Cannot send hello message");
  }

  Serial.println("Waiting for a message");
  receive(helloPeriod * 1000, handleSwitchMessage);
}


