/** Trivial JSON messages parser for pIoT base.
 * The name of the first object identifies the type of message.
 *  A common fields is "address".
 *
 * Author: Dario Salvi (dariosalvi78 at gmail dot com)
 *
 * Licensed under the GPL license http://www.gnu.org/copyleft/gpl.html
 */

#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#include <pins_arduino.h>
#endif

/** Separates an array into an array of strings.
 * @param line a pointer to the line to be analysed
 * @param arr a pre-initialized array of char*
 * @param len the length of the resulting array
 */
void JSONtoStringArray(char* line, char** arr, int* len);

/** Looks for the start of a data, given its name
 * example: search for "data" returns a pointer next to the : after "data" has been found
 * @param line the line where to look into
 * @param dataname the name of the data without the ".." and :
 * @return the pointer where the data starts (after the :), NULL if not found
 */
char* JSONsearchDataName(char* line, char* dataname);

/** Converts a JSON property to a unsigned long.
 * @param line the line that contains the property
 * @param dataName the property to be parsed, should not include the \"...\":
 * @return the parsed unsigned long
 */
unsigned long JSONtoULong(char* line, char* dataName);

/** Converts a JSON property to a long.
 * @param line the line that contains the property
 * @param dataName the property to be parsed, should not include the \"...\":
 * @return the parsed long
 */
long JSONtoLong(char* line, char* dataName);

/** Converts a JSON property to a double.
 * @param line the line that contains the property
 * @param dataName the property to be parsed, should not include the \"...\":
 * @return the parsed double
 */
double JSONtoDouble(char* line, char* dataName);

/** Converts a JSON property to boolean.
 * @param line the line to be parsed
 * @param dataName the name of the property
 * @return the boolean value
 */
boolean JSONtoBoolean(char* line, char* dataName);

/** Reads the strings coming from the serial and calls the parsers.
 * It listens for the time specified in millis and does not exit
 * until that time has been reached.
 * Spaces, tabs and new lines are removed automatically.
 * @param millis the time to wait until some data is found
 * @param a function that treats the message:
 * - dataname is the name of the first object
 * - msg is the entire JSON string
 */
void readSerial(int millis, void (*f)(char* dataName, char* msg));

