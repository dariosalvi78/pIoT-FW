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
#ifndef pIoT_PROTOCOL_H_INCLUDED
#define pIoT_PROTOCOL_H_INCLUDED

#include <nRF24.h>

//The pipe used for broadcast messages
#define BROADCAST_PIPE 0

//Default broadcast address
#define BROADCAST_ADDR 16711935

//The pipe used for private messages
#define PRIVATE_PIPE 1

//Default address of the base station
#define BASE_ADDR -2130771712

//Default radio channel
#define RF_CHANNEL 50


/** Configures and starts the radio.
 * init() must be called to initialise the interface and the radio module
 * @param chipEnablePin the Arduino pin to use to enable the chip for transmit/receive
 * @param chipSelectPin the Arduino pin number of the output to use to select the NRF24 before
 * @param powerPin the Arduino pin number used to power up the nRF24 module (-1 if always powered up)
 * @param myAddress the address of this node, expressed as a long (from -2,147,483,648 to 2,147,483,647)
 * @return true on success
 */
boolean startRadio(byte chipEnablePin, byte chipSelectPin, byte powerPin, long myaddress);

/** Shuts the radio module down.
 * To restart it you don't need to call startRadio() explicitly.
 */
boolean stopRadio();

/** Sends a message to another pIoT.
 * @param broadcast true if broadcast
 * @param destination address of the destination
 * @param msgType type of message
 * @param len length of the payload in bytes, it cannot exceed 26 (!)
 * @return true if sent
 */
boolean send(boolean broadcast, long destination, unsigned int msgType, byte* data, int len);

/** Receives a message.
 * @param timeoutMS a time-out in milliseconds
 * @param f a function that treats the message with the following parameters:
 * - broadcast tells if the message was in broadcast
 * - sender the sender address
 * - msgType the message type
 * - data the payload
 * - len the length of the payload
 * @return true if something arrived
 */
boolean receive(unsigned int timeoutMS, void (*f)(boolean broadcast, long sender, unsigned int msgType, byte* data, int len));

/** Returns the number of sent, and received, packets since the node was started.
 */
unsigned long getSentCounter();

/** Returns the number of packets that were sent but not received, since the node was started.
 */
unsigned long getUnsentCounter();

/** Returns the number of received packets since the node was started.
 */
unsigned long getReceivedCounter();

#endif // pIoT_PROTOCOL_H_INCLUDED
