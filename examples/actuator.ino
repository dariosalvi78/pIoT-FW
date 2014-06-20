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
long nodeAddress = 123457;

/** Time, in seconds, the sensor will sleep before
 * sending another measurement.
 */
int sleepTime = 10;

/** Definition of the message that contains
 * the information about the status of a switch.
 */
unsigned int switchMsgType = 101;
struct switchPacket{
    boolean status;
};


void setup() {
	Serial.begin(57600);

    long myAddress = nodeAddress;

    Serial.println("pIoT example, acting as Actuator");

    if(!startRadio(9, 10, 12, myAddress)) Serial.println("Cannot start radio");
}

/** Handles incoming messages from the network.
 * Particularly parses the switch messages, actuates
 * accordingly and sends a status message back for
 * confirmation.
 */
void handleSwitchMessage(boolean broadcast, long sender, unsigned int msgType, byte* data, int len){
    if (msgType == switchMsgType){
        Serial.println("Received a switch message");
        switchPacket pkt = *((switchPacket*)data);
        digitalWrite(5, pkt.status);
        send(false, BASE_ADDR, switchMsgType, (byte*)&pkt, sizeof(switchPacket));
    } else{
        Serial.println("Received something that I cannot interpret");
    }
}

void loop() {
    //The loop just waits for incoming messages
    Serial.println("Waiting for a message");
    receive(5000, handleSwitchMessage);
}

