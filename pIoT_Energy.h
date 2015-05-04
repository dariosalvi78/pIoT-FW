/** pIoT energy management library.
 * It uses the features of the ATMega chip to power down the MCU
 *
 * Author: Dario Salvi (dariosalvi78 at gmail dot com)
 *
 * Licensed under the GPL license http://www.gnu.org/copyleft/gpl.html
 */
#ifndef pIoT_ENERGY_H_INCLUDED
#define pIoT_ENERGY_H_INCLUDED

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#include <pins_arduino.h>
#endif


/** Resets the MCU.
 */
void reset();

/**
 * Puts all the pincs of the MCU to low
 */
void powerDownAllPins();

/** Put the MCU into sleep mode until wither a certain time has passed or a pin has changed.
 * Powers down everything, radio and MCU.
 * @param seconds the number of seconds after which we want the board to wakeup.
 * 0 means: don't sleep at all
 * a number <1 means don't care, sleep until someone else wakes it up
 * @param pinsN the number of pins, if <=0 the pins arenot considered
 * @param ... a set of pins to be considered if any
 */
void sleepUntil(int seconds, int pinsN, ...);

/** Gives the number of seconds the Sensorino has been sleeping since it has been switched on
 * @return the total number of seconds it has slept
 */
unsigned long getTotalSleepSeconds();

/** Retrieves the value of the alimentation voltage
 * this value i scomputed against an internal reference
 * and might be unprecise
 * @return the value in volts
 */
float getInternalVcc();

/** Gives the internal temperature in chip.
 * The value has an uncertainty of 10 degrees
 * @return the temperature in Celsius
 */
float getInternalTemperature() ;

#endif // pIoT_ENERGY_H_INCLUDED
