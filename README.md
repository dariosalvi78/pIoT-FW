pIoT-FW
=======


pIoT is an open source pico/personal framework for the Internet of Things (IoT).
It includes a hardware design for low-cost, low-power, Arduino compatible boards, a C++ library for programming the board and a simple server application that stores data and offers web visualization.


This repo contains the code instantiated on the boards.

Requirements:
-------------

1.  the code must be an Arduino library to be imported
2.  must implement the functionalities of the radio module
3.  must include control of sleeping modes for power consumption control
4.  shall include functionalities for communicating with the server


Design decisions:
-----------------

1.  the library contains C++ parts, but the API is modelled as C functions


