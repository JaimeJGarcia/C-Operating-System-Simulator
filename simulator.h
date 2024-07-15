//pre-compiler directives
#ifndef SIMULATOR_H
#define SIMULATOR_H

//header files
#include <pthread.h>
#include "configops.h"
#include "metadataops.h"
#include "datatypes.h"
#include "StringUtils.h"
#include "simtimer.h"
#include "ProgramFormat.h"

//functions
StringToFile *addLogToFile( StringToFile *logToFilePtr, char *logString );
PCB *addPCBNode(PCB *localHeadPtr, OpCodeType *localPtr, int processCount,
                                                    ConfigDataType *configPtr );
void addPCBToTraversedPCBs( int currentPCBPID, int *traversedPCB, int amountOfPCBs);
Boolean attemptMemAccess( int base, int offset, int *availableMemory );
Boolean attemptMemAllocation( int base, int offset, int *availableMemory );
int calculateRunTime(OpCodeType *localHeadPtr, ConfigDataType *configPtr);
int checkMaximumMemory( ConfigDataType *configPtr );
PCB *clearPCBList(PCB *pcbPtr);
StringToFile *clearStringToFileList(StringToFile *stringToFilePtr);
int countAppStart( OpCodeType *localPtr );
void displayOpCode( PCB *pcbPtr, ConfigDataType *configPtr,
                   Boolean logToMonitor, Boolean logToFile, int *memoryAvailable,
                                                    StringToFile *logFilePtr);
PCB *getNextPCB(OpCodeType *localMetaDataPtr,
                           ConfigDataType *configLocalPtr, PCB *localPCBHeadPtr,
                     int currentLoopIndex, int *traversedPCB, int amountOfPCBs);
PCB *getNextPCBFCFSN( PCB *localPCBHeadPtr, int currentLoopIndex );
PCB *getNextPCBSJFN( PCB *localPCBHeadPtr, int *traversedPCB, int amountOfPCBs );
int getShortestRuntime( PCB *localPCBHeadPtr, int *traversedPCB, int amountOfPCBs );
Boolean pidAlreadyTraversed( int currentPCBPID, int *traversedPCB, int amountOfPCBs );
void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr );
void *threadIOProcess( void *arg );

#endif //SIMULATOR_H
