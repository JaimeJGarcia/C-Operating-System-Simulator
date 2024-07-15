//pre-compiler directive
#ifndef PROGRAM_FORMAT_H
#define PROGRAM_FORMAT_H

// header files
#include <stdio.h>
#include "configops.h"

//functions
void logFileToHeader( ConfigDataType *configPtr, FILE *filePointer );
void showProgramFormat();

#endif //PROGRAM_FORMAT_H
