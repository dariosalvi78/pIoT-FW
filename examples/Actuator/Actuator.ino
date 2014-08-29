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

/** The last time an hello message was sent.
 */
unsigned long lastHelloSent = 0;

/** If true puts the MCU in sleep mode.
 * This allows saving some power, but the radio
 * module will be kept on receving.
 */
boolean sleep = true;

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

  Serial.println("pIoT example, acting as Actuator");

  if (!startRadio(9, 10, nodeAddress)) Serial.println("Cannot start radio");
}

/** Handles incoming messages from the network.
 * Parses the switch messages, actuates
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
  //The loop sends a Hello message every helloPeriod secs
  //and waits for incoming switch messages

  //seconds passed since start
  unsigned long time = (millis() / 1000) + getTotalSleepSeconds();

  if ((time - lastHelloSent) > helloPeriod) {
    //Time to send a hello message
    Serial.println("Sending hello");
    helloMessage hm;
    hm.internalTemp = getInternalTemperature();
    hm.internalVcc = getInternalVcc();
    hm.operationTime = time;
    hm.sentMsgs = getSentCounter();
    hm.unsentMsgs = getUnsentCounter();
    hm.receivedMsgs = getReceivedCounter();
    if (!send(false, BASE_ADDR, helloMsgType, (byte*) &hm, sizeof(helloMessage))) {
      Serial.println("- Cannot send hello message");
    }
    lastHelloSent = time;
  }

  if (sleep) {
    //leave the radio in receive mode before going to sleep
    receive(0, handleSwitchMessage);

    Serial.println("Going to sleep...");
    delay(50); //this delay it's only for allowing the serial complete the message
    
    sleepUntil(helloPeriod, 1, 2); //2 is the used IRQ pin

    //after the sleep a message may have just come, handle it
    receive(0, handleSwitchMessage);
  }
  else {
    //just wait until a message comes or there's a timeout
    receive(helloPeriod, handleSwitchMessage);
  }
}


