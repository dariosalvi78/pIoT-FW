/**
 * Simple Sketch to demonstrate the radio functions
 * of the nRF24 library.
 * You have to comment the SENDER definition to act as a receiver.
 */
#include <Arduino.h>
#include <SPI.h>
#include <nRF24.h>

//Comment this to act as a receiver
#define SENDER

//digital pin used to powerup the NRF24 module, use -1 if none
byte powerpin = 8;

//Used pipe, chhose 0 to 5
byte pipe = 0;
//RF channel, choose 0 to ?
byte rfchannel = 50;
//the pipe address, any would do
byte pipeAddr[] = {1, 2, 3, 4};
//use of autoacks
boolean autoAcks = true;
//delays between retransmissions, 0 to 15
byte delays = 2;
//retransmissions, 0 to 15
byte retries = 5;
unsigned long sleep = 1000;
//data rate: NRF24DataRate1Mbps, NRF24DataRate2Mbps, NRF24DataRate250kbps
NRF24::NRF24DataRate datarate = NRF24::NRF24DataRate2Mbps;
//transmit power: NRF24TransmitPowerm18dBm, 12dBm, 6dBm, 0dbm
NRF24::NRF24TransmitPower power = NRF24::NRF24TransmitPower0dBm;
//test IRQs? just prints a line when RX IRQ is detected
boolean testIRQ = false;

unsigned int counter = 0;
unsigned int lost = 0;

void setup() {
  Serial.begin(57600);
  Serial.print("SendReceive test, ");
#ifdef SENDER
  Serial.println("acting as sender");
#else
  Serial.println("acting as receiver");
#endif

  nRF24.configure(9, 10, powerpin);
  Serial.print(nRF24.powerUpIdle());
  Serial.print(nRF24.setChannel(rfchannel));
  Serial.print(nRF24.setPayloadSize(pipe, 0));
  Serial.print(nRF24.setAddressSize(NRF24::NRF24AddressSize4Bytes));
  Serial.print(nRF24.setCRC(NRF24::NRF24CRC2Bytes));
  Serial.print(nRF24.setRF(datarate, power));
  Serial.print(nRF24.enablePipe(pipe));
  Serial.print(nRF24.setPipeAddress(pipe, pipeAddr));
  Serial.print(nRF24.setAutoAck(pipe, autoAcks));

  Serial.print(nRF24.setTXRetries(delays, retries));
#ifdef SENDER
  Serial.print(nRF24.setTransmitAddress(pipeAddr));
#else
  if(testIRQ){
    Serial.print(nRF24.setIRQMask(false, true, true));
    attachInterrupt(0, handleIRQ, CHANGE);
  }
#endif

  Serial.println(" <- if all of these are 1, then it's configured");
  delay(500);
}

void handleIRQ(){
 Serial.println("GOT IRQ!!"); 
}

void loop() {

#ifdef SENDER
  counter++;
  Serial.print("Sending packet ");
  Serial.print(counter);
  byte pkt[2];
  pkt[0] = counter & 0xFF ;;
  pkt[1] = (counter >> 8) & 0xFF;
  if (! nRF24.send(pkt, 2, !autoAcks)) lost++;
  Serial.print(", lost ");
  Serial.println(lost);
  nRF24.powerDown();
  delay(sleep);
#else
  nRF24.waitAvailable(); //waits forever, until a packet is received
  byte buff[50];
  byte len;
  if (nRF24.recv(&pipe, buff, &len)) {
    counter ++;
    Serial.print("Received packet n ");
    Serial.print(counter);
    if(len !=2){
      Serial.println(" -- truncated packet!");
      return;
    }
    Serial.print(" of ");
    unsigned int recounter = buff[0] + (buff[1] << 8);
    Serial.print(recounter);
    Serial.print(", lost ");
    Serial.println(recounter - counter);
  } else {
    Serial.println("Wait exited, but no packet available");
  }
#endif

}

