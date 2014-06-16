/** Trivial JSON messages parser for Sensorino base.
 * The name of the first object identifies the type of message.
 *  A common fields is "address".
 *
 * Author: Dario Salvi (dariosalvi78 at gmail dot com)
 *
 * Licensed under the GPL license http://www.gnu.org/copyleft/gpl.html
 */

#include <pIoT_Protocol.h>

/** Separates an array into an array of strings.
 * @param line a pointer to the line to be analyzed
 * @param arr a pre-initialized array of char*
 * @param len the length of the resulting array
 */
void JSONtoStringArray(char* line, char** arr, int* len);

/** Looks for the start of a data, given its name
 * example: search for "data" returns a pointer next to the : after "data" has been found
 * @param line the line where to look into
 * @param dataname the name of the data without the ".." and :
 * @return the pointer where the data starts (aftert the :), NULL if not found
 */
char* JSONsearchDataName(char* line, char* dataname);

/** Converts a JSON property to a unsigned long.
 * @param line the line that contains the property
 * @param dataName the property to be parsed, should not include the \"...\":
 * @return the parsed unsigned long
 */
unsigned long JSONtoULong(char* line, char* dataName);

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

/** Utility function to parse the "address" field
 * @param address a buffer of 3 bytes where to put the address
 * @param msg the JSON message
 */
void JSONtoAddress(byte* address, char* msg);

/** Utility function to generate the JSON string for the address
 * @param stringbuffer a buffer where the JSON string will be put, consider at least 30 characters
 * @param address the address as byte array
 */
void addressToJSON(char* stringbuffer, byte* address);
