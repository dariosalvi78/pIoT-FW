<img src="http://openclipart.org/people/Scout/Chick.svg" width="10%" height="10%"/> pIoT-FW
===========================================================================================


pIoT is an open source pico/personal framework for the Internet of Things (IoT).
It includes a hardware design for low-cost, low-power, Arduino compatible boards, a C++ library for programming the board and a simple server application that stores data and offers web visualization.


This repo contains the code instantiated on the boards.

Requirements
------------

1.  it must work as an Arduino library
2.  must implement the functionalities of the radio module
3.  must include control of sleeping modes for power consumption control
4.  must include functionalities for communicating with the server


Design decisions
----------------

*  the library contains C++ parts, but the API is modelled as C functions
*  the communication among devices is done through *messages*, each type of message defined by an identifier. The library does not go into the detail of the content of the message, this is left to the user to be defined (for example by defining a struct)
*  the communication with the server PC is done through JSON messages
*  the library is split into four parts:
  1. nRF24 manages the nRF24L01+ module,
  2. pIoT_Protocol implements a simple protocol on top of the radio module, 
  3. pIoT_Energy implements power control in the microcontroller, 
  4. pIoT_JSON implements the communication between a device and the server computer


Usage
-----

The typical network configuration includes a PC with the server program installed (see pIoT-SW or pIoT-JS) that is connected via USB and a serial adapter (see pIoT-HW) to a node called *base* which is a gateway between the server and the rest of the nodes.
The other nodes communicate with the server by sending and receiving messages to the base.
The base will only have to translate the messages to and from the computer to which it is attached using JSON.

PC <--USB--> base <--RF--> node

JSON messages follow a predefined format like this one: `{ 'dataName': { a JSON message } }`, where dataName is the type of data is beign exchanged. For example: `{ 'lightMessage': { 'intensity': 300, 'on': true }}`. The dataName is neede to understand how to interpret the rest of the message.

In the Arduino IDE, copy the content of this directory into the *libraries* directory that is contained in the directory where all your Arduino sketches are (if it's not there, create one).
Be careful, the Arduino IDE will not accept pIoT-FW as a valid directory name, you will have to rename it to some other name (for example just pIoT will do).

On a node different than the base:

* include nRF24.h to be able to use the radio module
* include pIoT_Energy.h to manage power on the MCU
* include pIoT_Protocol.h for being able to send/receive messages, you will also need to include SPI.h and nRF24.h

On the base:

* also include pIoT_JSON.h for parsing JSON messages coming from the server

Brief API description
---------------------

*  `startRadio(byte chipEnablePin, byte chipSelectPin, byte irqpin, long myAddress)` is used to initialize the radio module
*  `stopRadio()` powers down the radio module
*  `send(boolean broadcast, long destination, unsigned int msgType, byte* data, int len)` for sending packets, note the identifier of the message: msgType
*  `receive(unsigned int timeoutMS, void (*f)(boolean broadcast, long sender, unsigned int msgType, byte* data, int len))` is used for receiving messages. The function waits until the timeoutMS has expired or a packed has been received
*  `sleepUntil(int seconds, int pinsN, ...)` is used to sleep for a certain number of seconds and/or a pin changes state
* `readSerial(int millis, void (*f)(char* dataName, char* msg))` reads the serial port and waits until a message has been received or millis have passed. When a message is received, it is passed to the function f
* `JSONtoStringArray(char* line, char** arr, int* len)` is used to parse JSON arrays
* `JSONsearchDataName(char* line, char* dataname)` given a JSON string in *line, searches a property with a certain certain name 
* `JSONtoLong(char* line, char* dataName)` searches for a property with a certain name and converts the value to a long
* `JSONtoULong(char* line, char* dataName)` searches for a property with a certain certain name and converts the value to an unsigned long
* `JSONtoDouble(char* line, char* dataName)` searches for a property with a certain certain name and converts the value to a double
* `JSONtoBoolean(char* line, char* dataName)` searches for a property with a certain certain name and converts the value to a boolean

For details, see the header files or look at the [examples folder](https://github.com/dariosalvi78/pIoT-FW/tree/master/examples) for example sketches.

Examples
--------

The library comes with its examples, in your Arduin IDE, see under File->Examples->pIoT

* nRF24SendReceive: a small sketch that makes use only of the nRF24 library. It is indicated for testing the radio modules and for finding the best configuration parameters for radio transmission.
* Sensor: an example of node that acts as a light sensor
* Actuator: an example Arduino sketch for a node that acts as an actuator
* Base: a sketch to be loaded on the base

