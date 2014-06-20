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
long nodeAddress = 123456;

/** Time, in seconds, the sensor will sleep before
 * sending another measurement.
 */
int sleepTime = 10;

/** Definition of the message that contains
 * the value of measured light intensity.
 */
unsigned int lightMsgType = 100;
struct lightPacket{
    int intensity;
};


void setup() {
	Serial.begin(57600);

    long myAddress = nodeAddress;
    Serial.println("pIoT example, acting as Sensor");
    if(!startRadio(9, 10, 12, myAddress)) Serial.println("Cannot start radio");
}


void loop() {
    //The loop reads an analog value and sends it as light
    //intensity in a light message, then goes to sleep for
    //a certain time
    Serial.print("Sending measure: ");

    int intensity = analogRead(0);
    Serial.println(intensity);
    lightPacket pkt;
    pkt.intensity = intensity;
    if(!send(false, BASE_ADDR, lightMsgType, (byte*) &pkt, sizeof(lightPacket))) Serial.println("Cannot send packet");

    Serial.print("...going to sleep for some seconds");
    delay(100);//this delay is to let the serial send the debug message
    stopRadio();//you have to shut down the radio explicitly
    sleepUntil(sleepTime, 0);
}


