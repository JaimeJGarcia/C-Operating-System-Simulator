//pre-compiler directive
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

//header files
#include <stdio.h>  // file operations
#include <stdlib.h> // dynamic memory operations
#include "datatypes.h" // for boolean

//create global constans - across files
typedef enum {NO_ERR,
              INCOMPLETE_FILE_ERR,
              INPUT_BUFFER_OVERRUN_ERR } StringManipCode;

extern const int STR_EQ;
extern const int MAX_STR_LEN;
extern const int STD_STR_LEN;
extern const int SUBSTRING_NOT_FOUND;
extern const char SPACE;
extern const char COLON;
extern const char SEMICOLON;
extern const char NULL_CHAR;
extern const char COMMA;
extern const char PERIOD;
extern const Boolean IGNORE_LEADING_WS;

//function prototypes
int compareString( char *oneStr, char *otherStr );
void concatenateString( char *destination, char *source);
void copyString( char *destination, char *source);
int findSubString( char *testStr, char *searchSubStr );
int getLineTo( FILE *filePtr, int bufferSize, char stopChar,
                          char *buffer, Boolean omitLeadingWhiteSpace,
                                        Boolean stopAtNonPrintable);
int getStringLength( char *testStr );
void getSubString( char *destStr, char *sourceStr,
                          int startIndex, int endIndex );
Boolean isEndOfFile( FILE *filePtr );
char setCharToLowerCase( char testChar );
void setStrToLowerCase( char *destStr, char *sourceStr );


#endif // STRING_UTILS_H
