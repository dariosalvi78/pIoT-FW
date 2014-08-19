/**
 * Example base Arduino sketch.
 * This sketch shows how a base node can be programmed.
 * It acts as a bridge between the server and the rest
 * of the nodes.
 * To send a message to the switch actuator try writing
 * { "SwitchSet": { "destAddress": 4321, "on": TRUE }}
 * on the serial monitor.
 */
#include <Arduino.h>
#include <SPI.h>
#include <nRF24.h>
#include <pIoT_Energy.h>
#include <pIoT_JSON.h>
#include <pIoT_Protocol.h>


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
  unsigned int unsentMsgs;
};

/** Definition of the message that contains
 * the value of measured light intensity.
 */
unsigned int lightMsgType = 100;
struct lightMessage {
  int intensity;
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
  Serial.println("pIoT example, acting as Base");

  if (!startRadio(9, 10, 12, BASE_ADDR)) Serial.println("Cannot start radio");
}

/** Function that manages json messages coming to the base from
 * the computer it is attached to (the sever).
 * This function only handles messages that contain an object
 * called "switch" and reads the value of the key called "status"
 * and sends a switch message to the corresponding node.
 */
void handleJson(char* dataname, char* message) {
  if (strcasecmp(dataname, "SwitchSet") == 0) { //use strcasecmp to compare strings ignoring their cases
    Serial.print("Going to send a switch set command to ");
    long address = JSONtoLong(message, "destAddress");
    Serial.print(address);
    switchMessage sm;
    Serial.print(" on? ");
    sm.on = JSONtoBoolean(message, "on");
    Serial.println(sm.on);
    if (!send(false, address, switchMsgType, (byte*) &sm, sizeof(switchMessage)))! Serial.println("Cannot send packet");
  } else {
    Serial.print("Received a messages with incomprehensible dataname \"");
    Serial.print(dataname);
    Serial.println("\"");
  }
}

/** Function tht manages the messages coming from the other nodes.
 * Two types of messages are supported, light and switch.
 * The function parses the message and generates a corresponding JSON
 * and sends it to the server.
 */
void handleMessage(boolean broadcast, long sender, unsigned int msgType, byte* data, int len) {
  Serial.print("Received a mesage of type ");
  Serial.print(msgType);
  Serial.print(" from node ");
  Serial.println(sender);
  if ((msgType == helloMsgType) &&
      (len == sizeof(helloMessage))) {
    helloMessage hm = *((helloMessage*) data);
    Serial.print("{ \"Hello\": { \"sourceAddress\":");
    Serial.print(sender);
    Serial.print(", \"temperature\":");
    Serial.print(hm.internalTemp);
    Serial.print(", \"Vcc\":");
    Serial.print(hm.internalVcc);
    Serial.print(", \"operationTime\":");
    Serial.print(hm.operationTime);
    Serial.print(", \"unsentMessages\":");
    Serial.print(hm.unsentMsgs);
    Serial.println(" }}");
  }
  else if ((msgType == lightMsgType) &&
           (len == sizeof(lightMessage))) {
    lightMessage lm = *((lightMessage*) data);
    Serial.print("{ \"LightState\": { \"sourceAddress\":");
    Serial.print(sender);
    Serial.print(", \"intensity\":");
    Serial.print(lm.intensity);
    Serial.println(" }}");
  }
  else if ((msgType == switchMsgType) &&
           (len == sizeof(switchMessage))) {
    switchMessage sm = *((switchMessage*) data);
    Serial.print("{ \"SwitchState\": { \"sourceAddress\":");
    Serial.print(sender);
    Serial.print(", \"on\":");
    Serial.print(sm.on);
    Serial.println(" }}");

  } else {
    Serial.print("- Cannot interpret message type ");
    Serial.println(msgType);
  }
}


void loop() {
  //The loop only reds the serial port
  //and receives data continuously
  //make sure to put no wait seconds, otherwise
  //data will be lost !
  readSerial(0, handleJson);
  receive(0, handleMessage);
}


