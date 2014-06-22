/** Trivial JSON messages parser for pIoT base.
 * The name of the first object identifies the type of message.
 *  A common fields is "address".
 *
 * Author: Dario Salvi (dariosalvi78 at gmail dot com)
 *
 * Licensed under the GPL license http://www.gnu.org/copyleft/gpl.html
 */
#ifdef __cplusplus
extern "C"
#endif

#include <pIoT_JSON.h>

#define JSON_STRING_BUFFER_LEN 150

void JSONtoStringArray(char* line, char** arr, int* len) {
    *len = 0;
    if(line == NULL) return;
    char* substrstart = line;
    int level = 0;
    int arridx = 0;
    for(int i=0; i<strlen(line); i++)
    {
        char* ptr = line +i;
        if(ptr == NULL) return;
        if(*ptr == '['){
            if(level == 0){
                arr[arridx] = ptr+1;
                arridx++;
            }
            level ++;
        }
        if((*ptr == ',') && (level == 1)){
            arr[arridx] = ptr+1;
            arridx++;
        }
        if(*ptr == ']') {
            if(level == 0) break;
            level --;
        }
    }
    *len = arridx;
}

char* JSONsearchDataName(char* line, char* dataname)
{
    char* dataptr = strstr(line, dataname);
    char* ptr = dataptr + strlen(dataname);
			
    for(int i=0; i<strlen(dataptr); i++) {
        ptr += i;
        if(*ptr == ':') {
            return ptr+1;
        }
    }
    return NULL;
}

unsigned long JSONtoULong(char* line, char* dataName)
{
    char* dataptr = JSONsearchDataName(line, dataName);
    if (dataptr != NULL) {
        return strtoul(dataptr, NULL, 10);
    }
    return 0;
}

long JSONtoLong(char* line, char* dataName)
{
	char* dataptr = JSONsearchDataName(line, dataName);
    if (dataptr != NULL) {
        return strtol(dataptr, NULL, 10);
    }
    return 0; 
}

double JSONtoDouble(char* line, char* dataName) {
    char* dataptr = JSONsearchDataName(line, dataName);
    if (dataptr != NULL)
    {
        return strtod(dataptr, NULL);
    }
    return 0;
}

boolean JSONtoBoolean(char* line, char* dataName) {
    char* dataptr = JSONsearchDataName(line, dataName);
    if (dataptr != NULL) {
        if((toupper(dataptr[0]) == 'T')&&
                (toupper(dataptr[1]) == 'R')&&
                (toupper(dataptr[2]) == 'U')&&
                (toupper(dataptr[3]) == 'E'))
            return true;
        else return false;
    }
    return false;
}

static char buffer[JSON_STRING_BUFFER_LEN];
static int buffPtr = 0;
static int level =0;
static boolean inQuotes = false;

static char firstWordBuff[20];
static int firstWordBuffPtr = 0;
static boolean inFirstWord = false;

void readSerial(int mis, void (*f)(char* dataName, char* msg)) {
    long now=  millis();

    while ((millis()-now < mis) || (Serial.available() >0)){
        if(!Serial.available()) continue;
        char b = Serial.read();
        if(b=='"'){
            if(inQuotes)  //not in quote anymore
            {
                inQuotes = false;
                inFirstWord = false;
            }
            else     //in quote
            {
                inQuotes = true;
                if(firstWordBuffPtr == 0){
                    inFirstWord = true;
                }
            }
        }
        if(b=='{'){
            level++;
        }

        if((level>0) && ((inQuotes) || ((b!=' ')&&(b!='\n')&&(b!='\r')&&(b!='\t')))){
            buffer[buffPtr] = b;
            buffPtr++;
            if(inFirstWord)  //Fill the first word buffer
            {
                firstWordBuff[firstWordBuffPtr] = b;
                firstWordBuffPtr++;
            }
        }

        if(b=='}'){
            if(level == 1){
			
                //The message is complete, use it
                char msg[buffPtr+1];
                for(int i=0; i<buffPtr; i++)
                    msg[i] = buffer[i];
                msg[buffPtr] = '\0';

                char firstword[firstWordBuffPtr];
                for(int i=0; i<firstWordBuffPtr-1; i++)
                    firstword[i] = firstWordBuff[i+1];
                firstword[firstWordBuffPtr-1] = '\0';

                //reset buffers
                buffPtr = 0;
                firstWordBuffPtr = 0;

                f(firstword, msg);
            }
			level--;
        }
    }
}


