//header files
#include "ProgramFormat.h"

/*
 * Function Name: logFileToHeader
 * Algorithm: displays the correct Simulator log file header to the file
 * Preconditon: given head pointer to congfig data and the opened file
 * Postcondition: code is printed to the log file
 * Exceptions: none
 * Notes: none
 */
void logFileToHeader( ConfigDataType *configPtr, FILE *filePointer )
   {
     //local variables for use in function
     FILE *localFilePointer = filePointer;
     ConfigDataType *localConfigPtr = configPtr;
     char cpuCode[MAX_STR_LEN];

     //get the cpuSchedCode as a string
     configCodeToString( localConfigPtr->cpuSchedCode, cpuCode );

     // Print out the adequate statement for the top of the log file
     fprintf( localFilePointer, "\n===========================================\
=======");
     fprintf( localFilePointer, "\nSimulator Log File Header\n");
     fprintf( localFilePointer, "\nFile Name                       : %s",
                                             localConfigPtr->metaDataFileName );
     fprintf( localFilePointer, "\nCPU Scheduling                  : %s",
                                                                      cpuCode );
     fprintf( localFilePointer, "\nQuantum Cycles                  : %d",
                                                localConfigPtr->quantumCycles );
     fprintf( localFilePointer, "\nMemory Available (KB)           : %d",
                                                 localConfigPtr->memAvailable );
     fprintf( localFilePointer, "\nProcessor Cycle Rate (ms/cycle) : %d",
                                                localConfigPtr->procCycleRate );
     fprintf( localFilePointer, "\nI/O Cycle Rate (ms/cycle)       : %d",
                                                  localConfigPtr->ioCycleRate );
   }

/*
 * Function Name: showProgramFormat
 * Process: displays command line argument requirements for this program
 * method input/parameters: none
 * method output/parameters: none
 * metod output/returned: none
 * device input/keyboard: none
 * device output/monitor: instructions provide as specified
 * dependencies: none
 */
void showProgramFormat()
   {
      // print out command line argument instructions
        // function: printf
      printf( "Program Format:\n" );
      printf( "     sim_0X [-dc] [-dm] [-rs] <config file name>\n" );
      printf( "     -dc [optional] displays configuration data\n" );
      printf( "     -dm [optional] displays meta data\n" );
      printf( "     -rs [optional] runs simulator\n" );
      printf( "     config file name is required\n" );
   }
