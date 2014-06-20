/**
 * Example base Arduino sketch.
 * This sketch shows how a base node can be programmed.
 * It acts as a bridge between the server and the rest
 * of the nodes.
 */
#include <Arduino.h>
#include <SPI.h>
#include <nRF24.h>
#include <pIoT_Energy.h>
#include <pIoT_JSON.h>
#include <pIoT_Protocol.h>


long nodeAddress = 123456;

/** Definition of the message that contains
 * the value of measured light intensity.
 */
unsigned int lightMsgType = 100;
struct lightPacket{
    int intensity;
};

/** Definition of the message that contains
 * the information about the status of a switch.
 */
unsigned int switchMsgType = 101;
struct switchPacket{
    boolean status;
};


void setup() {
	Serial.begin(57600);

    Serial.println("pIoT example, acting as Base");
    long myAddress = BASE_ADDR;

    if(!startRadio(9, 10, 12, myAddress)) Serial.println("Cannot start radio");
}

/** Function that manages json messages coming to the base from
 * the computer it is attached to (the sever).
 * This function only handles messages that contain an object
 * called "switch" and reads the value of the key called "status"
 * and sends a switch message to the corresponding node.
 */
void handleJson(char* dataname, char* message){
    if(strcasecmp(dataname, "switch")){
        Serial.print("Going to send a switch command.");
        long address = JSONtoULong("destAddress", message);
        switchPacket pkt;
        pkt.status = JSONtoBoolean(message, "status");
        Serial.println(pkt.status);
        if(!send(false, address, switchMsgType, (byte*) &pkt, sizeof(pkt)))! Serial.println("Cannot send packet");
    }
}

/** Function tht manages the messages coming from the other nodes.
 * Two types of messages are supported, light and switch.
 * The function parses the message and generates a corresponding JSON
 * and sends it to the server.
 */
void handleMessage(boolean broadcast, long sender, unsigned int msgType, byte* data, int len){
    Serial.println("Received a mesage");
    if(msgType == lightMsgType){
        lightPacket pkt = *((lightPacket*) data);
        Serial.print("{ \"LightMessage\": { \"sourceAddress\":");
        Serial.print(sender);
        Serial.print(", \"intensity\":");
        Serial.print(pkt.intensity);
        Serial.print(" }}");
    }
    else if (msgType == switchMsgType){
        switchPacket pkt = *((switchPacket*) data);
        Serial.print("{ \"switch\": { \"sourceAddress\":");
        Serial.print(sender);
        Serial.print(", \"status\":");
        Serial.print(pkt.status);
        Serial.print(" }}");

    } else {
        Serial.println("Received something that I cannot interpret");
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

