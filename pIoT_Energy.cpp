/** pIoT energy management library.
 * It uses the features of the ATMega chip to power down the MCU
 *
 * Author: Dario Salvi (dariosalvi78 at gmail dot com)
 *
 * Licensed under the GPL license http://www.gnu.org/copyleft/gpl.html
 *
 * For an extended guide on power consumption and saving see
 * http://www.gammon.com.au/forum/?id=11497
 */
#ifdef __cplusplus
extern "C"
#endif

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

#include <pIoT_Energy.h>


void(* resetf) (void) = 0;

void powerDownAllPins(){
  pinMode(0, INPUT); digitalWrite(0, LOW);
  pinMode(1, INPUT); digitalWrite(1, LOW);
  pinMode(2, INPUT); digitalWrite(2, LOW);
  pinMode(3, INPUT); digitalWrite(3, LOW);
  pinMode(4, INPUT); digitalWrite(4, LOW);
  pinMode(5, INPUT); digitalWrite(5, LOW);
  pinMode(6, INPUT); digitalWrite(6, LOW);
  pinMode(7, INPUT); digitalWrite(7, LOW);
  pinMode(8, INPUT); digitalWrite(8, LOW);
  pinMode(9, INPUT); digitalWrite(9, LOW);
  pinMode(10, INPUT); digitalWrite(10, LOW);
  pinMode(11, INPUT); digitalWrite(11, LOW);
  pinMode(12, INPUT); digitalWrite(12, LOW);
  pinMode(13, INPUT); digitalWrite(13, LOW);
  pinMode(A0, INPUT); digitalWrite(A0, LOW);
  pinMode(A1, INPUT); digitalWrite(A1, LOW);
  pinMode(A2, INPUT); digitalWrite(A2, LOW);
  pinMode(A3, INPUT); digitalWrite(A3, LOW);
  pinMode(A4, INPUT); digitalWrite(A4, LOW);
  pinMode(A5, INPUT); digitalWrite(A5, LOW);
}

void reset(){
    resetf();
}


//Used to keep track if we have to keep sleeping or not
volatile boolean keepSleeping = true;

//ISR that wakes up the PIC when a pin changes from LOW to HIGH or viceversa
ISR(PCINT0_vect){
    keepSleeping = false;
}
ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));
ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));

//Returns the interrupt mask of the pin
byte pinToInt(byte pin){
  if(pin <=7)
    return PCINT16 + pin;
  else if(pin >=8 && pin <=13)
    return PCINT0 + (pin-8);
  else return PCINT8 + (pin -14);
}

//Returns the interrupt enable mask of the pin
byte pinToIE(byte pin){
  if(pin <=7)
    return PCIE2;
  else if(pin >=8 && pin <=13)
    return PCIE0;
  else return PCIE1;
}

/** Counter of the wake ups produced by the watchdog.
 */
volatile unsigned long totalSleepCounter;

unsigned long getTotalSleepSeconds(){
    return totalSleepCounter;
}

/** seconds to be waited */
int toWaitSeconds = 0;
volatile int sleptSecondsSinceLastWakeUp =0;

/** ISR of the watchdog */
ISR(WDT_vect) {
    totalSleepCounter++;
    sleptSecondsSinceLastWakeUp ++;
    if(( toWaitSeconds >0) && (sleptSecondsSinceLastWakeUp >= toWaitSeconds)){
        //Time to wake up!
        keepSleeping = false;
        sleptSecondsSinceLastWakeUp = 0;
    } else {
        keepSleeping = true;
    }
}


void sleepUntil(int seconds, int pinsN, ...){
    if(seconds == 0)
        return;

    int pins[pinsN];
    va_list list;
    va_start(list, pinsN);
    for(int i = 0; i<pinsN; i++){
       pins[i] = va_arg(list, int);
    }
    va_end(list);

    //make sure we don't get interrupted before we sleep
    noInterrupts ();

	//power off ADC
	ADCSRA &= ~(1<<ADEN);

    //register pin changes
    if(pinsN >0){
		//clear pins interrupts
		PCIFR &= ~(bit (PCIF0));
		PCIFR &= ~(bit (PCIF1));
		PCIFR &= ~(bit (PCIF2));
		for(int i=0; i< pinsN; i++){
			//set pin mask
			if((pins[i]>=0) && (pins[i]<=19)){//pin is considered only if >=0 and <=19
				if(pins[i] <=7)
					PCMSK2 |= (1 << pinToInt(pins[i]));
				else if(pins[i] >=8 && pins[i] <=13)
					PCMSK0 |= (1 << pinToInt(pins[i]));
				else PCMSK1 |= (1 << pinToInt(pins[i]));
				//Register the pin change interrupt
				PCICR |= (1 << pinToIE(pins[i]));
			}
		}
	}

    //Activate the watchdog
    toWaitSeconds = seconds;
    // reset status flag
    MCUSR &= ~(1 << WDRF);
    // enable configuration changes
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    // set the prescalar to 0110 (1 second)
    WDTCSR = (0<<WDP0) | (1<< WDP1) | (1 << WDP2) | (0<<WDP3);
    // enable interrupt mode without reset
    WDTCSR |= _BV(WDIE);

    //Set sleep mode power down:
    //In this mode, the external Oscillator is stopped, while the external interrupts, the 2-
    //wire Serial Interface address watch, and the Watchdog continue operating (if enabled). Only an
    //External Reset, a Watchdog System Reset, a Watchdog Interrupt, a Brown-out Reset, a 2-wire
    //Serial Interface address match, an external level interrupt on INT0 or INT1, or a pin change
    //interrupt can wake up the MCU.
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    keepSleeping = true;

    //interrupts allowed now
    interrupts ();
    //Let's sleep !
    while(keepSleeping){
        // turns BOD off, must be done right before sleep
        #ifdef BODS
        MCUCR |= (1<<BODS) | (1<<BODSE);
        MCUCR &= ~(1<<BODSE);
        #endif
        sleep_cpu();  // sleep here
    }

    //Here we wake up
    sleep_disable();

    //deactivate watchdog
    wdt_disable();

    //deactivate pins
    if(pinsN >0) for(int i=0; i< pinsN; i++){
        if((pins[i]>=0) && (pins[i]<=19)){
            PCICR &= ~(1 << pinToIE(pins[i]));
        }
    }


	//restore ADC
	ADCSRA |= (1<<ADEN);  // adc on

    power_all_enable();
}


float getInternalVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  float retval = (float)result / 1000;
  return retval;
}

//From: http://playground.arduino.cc/Main/InternalTemperatureSensor
float getInternalTemperature() {
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN);
  delay(20);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  return ((float)ADCW * 0.9873) - 330.12;
}
