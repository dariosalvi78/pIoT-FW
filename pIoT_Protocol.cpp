/** pIoT communication library.
 * This library goes on top of the nRF24L01 and simply adds the sender and a messsage type field.
 * Decisions taken:
 * - pipe 0 is used as broadcast pipe, with shared address and no acks
 * - pipe 1 is used as private address
 * - nodes send their address
 * - addresses are 4 bytes long
 * - messages are identified by a message type field of 2 bytes
 * - CRC is 2 bytes
 * - 2Mbps, 750us ack time, 5 retries
 *
 * Author: Dario Salvi (dariosalvi78 at gmail dot com)
 *
 * Licensed under the GPL license http://www.gnu.org/copyleft/gpl.html
 */
#ifdef __cplusplus
extern "C"
#endif

#include <pIoT_Protocol.h>

//Addresses:
byte broadCastAddress[4] = {BROADCAST_ADDR};
byte baseAddress[4] = {BASE_ADDR};
byte thisAddress[4] = {1,2,3,4}; //just a default address

boolean startRadio(byte chipEnablePin, byte chipSelectPin, byte irqPin, byte myAdd[]) {
    NRF24::configure(chipEnablePin, chipSelectPin, irqPin);
    thisAddress[0] = myAdd[0];
    thisAddress[1] = myAdd[1];
    thisAddress[2] = myAdd[2];
    thisAddress[3] = myAdd[3];

    //Init the nrf24
    nRF24.init();
    if(!nRF24.setChannel(RF_CHANNEL)) return false;
    //set dynamic payload size
    if(!nRF24.setPayloadSize(0, 0)) return false;
    if(!nRF24.setPayloadSize(1, 0)) return false;
    //Set address size to 3
    if(!nRF24.setAddressSize(NRF24::NRF24AddressSize4Bytes)) return false;
    //Set CRC to 2 bytes
    if(!nRF24.setCRC(NRF24::NRF24CRC2Bytes)) return false;
    //Set 2 Mbps, maximum power
    if(!nRF24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPower0dBm)) return false;
    //Configure pipes
    if(!nRF24.setPipeAddress(0, broadCastAddress)) return false;
    if(!nRF24.enablePipe(0)) return false;
    if(!nRF24.setAutoAck(0, false)) return false;
    if(!nRF24.setPipeAddress(1, thisAddress)) return false;
    if(!nRF24.enablePipe(1)) return false;
    if(!nRF24.setAutoAck(1, true)) return false;
    //Configure retries
    if(!nRF24.setTXRetries(3, 5)) return false;
    return true;
}

boolean send(boolean broadcast, byte* destination, unsigned int msgType, byte* data, int len){
    if(broadcast){
        if(!nRF24.setTransmitAddress(broadCastAddress)) return false;
    }
    else{
        if(!nRF24.setTransmitAddress(destination)) return false;
    }
    unsigned int totlen = len + 6;
    byte pkt[totlen];
    pkt[0] = thisAddress[0];
    pkt[1] = thisAddress[1];
    pkt[2] = thisAddress[2];
    pkt[3] = thisAddress[2];

    pkt[4] = msgType & 0xFF ;
    pkt[5] = (msgType >> 8) & 0xFF;

    for(int i=0; i<len; i++){
        pkt[i+6] = data[i];
    }
    return nRF24.send(pkt, totlen, broadcast);
}

boolean receive(unsigned int timeoutMS, void (*f)(boolean broadcast, byte* sender, unsigned int msgType, byte* data, int len)){
    boolean broadcast;
    byte sender[4];
    unsigned int msgType;
    int len;
    byte buffer[NRF24_MAX_MESSAGE_LEN];
    byte totlen;
    byte pipe;
    if(nRF24.waitAvailableTimeout(timeoutMS)){
            if(nRF24.recv(&pipe, buffer, &totlen)){
                broadcast = (pipe == BROADCAST_PIPE);
                sender[0] = buffer[0];
                sender[1] = buffer[1];
                sender[2] = buffer[2];
                sender[3] = buffer[3];
                msgType = (unsigned int)(buffer[5] <<8) + (unsigned int)buffer[4];
                len = totlen - 6;
                byte data[len];
                for(int i=0; i<len; i++)
                    data[i] = buffer[i+6];

                f(broadcast,sender, msgType, data, len);
                return true;
            }
    }
    return false;
}

