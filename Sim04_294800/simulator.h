// Protect from multiple compiling
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include<stdio.h>
#include "configops.h"
#include "metadataops.h"
#include <pthread.h>
#include "simtimer.h"

// enum for process states
typedef enum { NEW,
               READY,
               RUNNING,
               EXITING,
               BLOCKED } processState;

// log File linked list meant to hold each line
typedef struct logFileType
   {
    char logContents[ 1000 ];      // holds current line of commands
    struct logFileType *nextNode;   // points to next node in list
   } logFileType;

// PCB data structure for processes
typedef struct pcbType
   {
    struct OpCodeType *opCommand;  // op code command from meta data pointer
    int pid;                       // process ID
    struct pcbType *nextNode;      // points to null or next process pcbType
    processState pcbState;         // process state from enum processState
    int timeRemaining;             // time remaining for process in ms
    int priority;
   } pcbType;

// interruptNode meant to be queued and released. pid is tied to the process
// it has blocked
typedef struct interruptType
   {
    bool interruptBool;              // flags intterupts and dequeueing
    pcbType *pcbNode;                // pointer to a pcbNode
    char interruptString[ 100 ];     // device input/output operation string
    int cylcleTimeLeft;              // cpu cycles the dev op is blocking
    struct interruptType *nextNode;  // points to the next list node
    struct interruptType *prevNode;  // points to the previous list node
   } interruptType;

// function prototypes

// attempts to acces memory, returns success or failure
bool accessMemory( int memoryBase, int memoryOffset, int *availableMemory );

// adds new log node to log file linked list
logFileType *addLogNode( logFileType *headPointer, logFileType *newNode );

// adds new process node to PCB linked list
pcbType *addProcess( int processNumber, OpCodeType *metaPointer,
                     pcbType *headNode, ConfigDataType *configPointer );

// attempts to allocate memory, returns failure or success
bool allocationAttempt( int min, int max, int *availableMemory );

// checks if the schedule code is preemptive
bool checkPreemption( char *scheduleCode );

interruptType *clearInterruptQueue( interruptType *headNode );

// frees allocated log linked list memory
logFileType *clearLogFileList( logFileType *logFileListPointer );

// frees allocated PCB linked list memory
pcbType *clearPCBList( pcbType *pcbPointer );

void dequeueInterrupt( interruptType **interruptHead,
                       interruptType *interruptNode );

// finds next FCFS-P processes for execution
pcbType *getFCFSPNode( pcbType *pcbHead );

// finds next FCFS-N process for execution
pcbType *getFCFSNNode( pcbType *pcbHead );

// picks next process for execution based on scheduling code
pcbType *getNextProcess( pcbType *pcbHead, char *scheduleCode);


// Checks to see if the memory a process requests is less than the maximum
// memory allowed. If the memory is less than the max memory, the requested
// memory will be returned.
int getMaxMemory( ConfigDataType *configPointer);

// find the a process's time remaining
int getPCBTime( OpCodeType *metaPointer, ConfigDataType *configPointer );

// count meta data "app start" to get the number of processes
int getProcessCount( OpCodeType *configPointer );

// returns process time by checking meta data cycle rates
int getProcessTime( ConfigDataType *configPointer, OpCodeType *metaPointer );

pcbType *getShortAlgoNode( pcbType *pcbHead, bool preempted );

int getShortestJob( pcbType *pcbHead, bool preempted );

void *handleThreadIO( void *timeParameter );

pcbType *populatePCBList( OpCodeType *metaPointer,
                          ConfigDataType *configPointer,
                          int processCount );

void runSim( ConfigDataType *configPtr, OpCodeType *metaPointer );

void queueInterruptNode( interruptType **interruptHead,
                         pcbType *pcbNode,
                         char *interruptString,
                         int cyclesLeft );

void showPCBCommands( pcbType *headNode,
                     ConfigDataType *configPointer,
                     bool toMonitorFlag,
                     bool toFileFlag,
                     int *availableMemory,
                     int *processesExited,
                     bool memDisplayFlag,
                     logFileType *logHead,
                     logFileType *logNode,
                     bool preemptionFlag,
                     interruptType **interruptHead );

void writeToLogFile( ConfigDataType *configPointer,
                     logFileType *logListPointer );

#endif  // SIMULATOR_H
