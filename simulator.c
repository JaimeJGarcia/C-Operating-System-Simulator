#include "simulator.h"

/*
 * Function Name: addLogToFile
 * Algorithm: addLogToFile creates a linked list of log strings to eventually
              be fputs to a file
 * Preconditon: needs the linked list of log strings and the next log string
 * Postcondition: returns generated linked list of log strings
 * Exceptions: none
 * Notes: none
 */
StringToFile *addLogToFile( StringToFile *logToFilePtr, char *logString )
  {
    // loops while the linked list is not null
    if(logToFilePtr == NULL)
    {
        // access memory for new link/node
            // function: malloc
        logToFilePtr = (StringToFile *)malloc(sizeof( StringToFile ));

        // assigns the log string to the received log string
        // assign next pointer to null
            // function: copyString
        copyString(logToFilePtr->logString, logString);
        logToFilePtr->nextNode = NULL;

        // return current pointer
        return logToFilePtr;
    }

    // assume end of list not found yet
    // assign recursive function to current's next link
        // function: addNode
    logToFilePtr->nextNode = addLogToFile(logToFilePtr->nextNode, logString);

    // return current pointer
    return logToFilePtr;
  }

/*
 * Function Name: addPCBNode
 * Algorithm: loops through the meta data by starting at the head pointer
 *            checks to see if each op code contains app and start, if it
 *            does it increases the amount of appStartTotal
 * Preconditon: given pointers to the heads of the PCB, metadata, and
                config, as well as the current process count
 * Postcondition: a generated linked list is returned
 * Exceptions: none
 * Notes: none
 */
PCB *addPCBNode(PCB *localHeadPtr, OpCodeType *metaDataPtr, int processCount,
                                                     ConfigDataType *configPtr )
   {
     // local files for accessing metadata and the config data
     OpCodeType *localMetaDataPtr = metaDataPtr;
     ConfigDataType *localConfigPtr = configPtr;

     // check for local pointer assigned to null
     if( localHeadPtr == NULL )
       {
        // access memory for new link/node
          // function: malloc
        localHeadPtr = (PCB *)malloc(sizeof( PCB ) );

        // assigns the values of the PCB to what they need to be
        // pid -> int, currState -> state, processStart -> app start location
        // processRunTime -> get the run time of the PCB, nextNode-> NULL
        localHeadPtr->pid = processCount;
        localHeadPtr->currState = NEW;
        localHeadPtr->processStart = metaDataPtr;
        localHeadPtr->processRunTime = calculateRunTime(localMetaDataPtr,
                                                                localConfigPtr);
        localHeadPtr->nextNode = NULL;

        // now that the PCB has been created we can set its state equal to
        // ready
        localHeadPtr->currState = READY;

        // return current PCB
        return localHeadPtr;
       }

     // moves onto the nextPCB to create it the next PCB
     localHeadPtr->nextNode = addPCBNode(localHeadPtr->nextNode,
                                localMetaDataPtr, processCount, localConfigPtr);

     return localHeadPtr;
   }

/*
 * Function Name: addPCBToTraversedPCBs
 * Algorithm: loops through the generated array of ints that keeps track of the
              PCBs that have been traversed, used in SRJ and potentially other
              CPU scheduling codes
 * Preconditon: needs the amount of PCBs, the PCB that is going to be added to
                the list and the int array of traversed PCBs
 * Postcondition: returns the int array with added PCB
 * Exceptions: none
 * Notes: none
 */
void addPCBToTraversedPCBs(int currentPCBPID, int *traversedPCB, int amountOfPCBs)
  {
    // local variable referencing the int array
    int *localTraversedPCB = traversedPCB;

    // Index to keep track of the location in the int array
    int index = 0;

    // Variable notSwitched is meant to keep track of if the current pcb
    // has been added to the int array or not
    Boolean notSwitched = True;

    // loops through each index in the traversed PCB array until current PCB
    // is added
    while( notSwitched == True )
      {
        // if the current index in the array is the amountOfPCBs + 1 (Which
        // it will be unless a PID has replaced it as I initialized it to this)
        if( localTraversedPCB[index] == amountOfPCBs + 1)
          {
            // sets the current index equal to the PID of the current PCB
            localTraversedPCB[index] = currentPCBPID;

            // switches the loop flag so it breaks the loop
            notSwitched = False;
          }

        // moves onto the next index in the array
        index = index + 1;
      }
  }

/*
 * Function Name: attemptMemAccess
 * Algorithm: if the base + offset of the memory access is greater than
              the remaining memory, the function returns false
 * Preconditon: The base, offset of the memory access and the current
                availableMemory is given
 * Postcondition: true or false is returned
 * Exceptions: none
 * Notes: none
 */
Boolean attemptMemAccess( int base, int offset, int *availableMemory )
  {
    // combines the base and the offset of the memory access
    int total = base + offset;

    // if the total is greater then the availableMemory then return false
    // otherwise return true
    if( total > *availableMemory)
      {
        return False;
      }

    return True;
  }

/*
 * Function Name: attemptMemAllocation
 * Algorithm: if the base + offset of the memory access is greater than
              the remaining memory, the function returns false, if there is
              enough memory, the function subtracts the total from the
              remaining memory
 * Preconditon: The base, offset of the memory access and the current
                availableMemory is given
 * Postcondition: true or false is returned and the availableMemory is adjusted
 * Exceptions: none
 * Notes: none
 */
Boolean attemptMemAllocation( int base, int offset, int *availableMemory )
  {
    // combine base and offset to get the total
    int total = base + offset;

    // returns false if the total is greater than availableMemory
    if(total > *availableMemory)
      {
        return False;
      }

    // subtracts the total from the availableMemory then returns true
    *availableMemory = *availableMemory - total;
    return True;
  }

/*
 * Function Name: calculateRunTime
 * Algorithm: grabs the run time of each process and multiplies it by the
              correct cycle rate time that we pull from the config file
 * Preconditon: given pointer to current app start and pointer to config
                file
 * Postcondition: returns generated run time of PCB
 * Exceptions: none
 * Notes: none
 */
int calculateRunTime( OpCodeType *localHeadPtr, ConfigDataType *configPtr )
   {
      // temp variable to return time
      int returnTime = 0;

      // loops through entire meta data list
      while(localHeadPtr != NULL)
         {
              // if the meta data process is ever app end, break out of the
              // loop and return returnTime.
              if( compareString( localHeadPtr->command, "app" ) == STR_EQ
                   && compareString( localHeadPtr->strArg1, "end" ) == STR_EQ )
                {
                  return returnTime;
                }

              // if the process is cpu, multiply process by procCycleRate
              if( compareString(localHeadPtr->command, "cpu") == STR_EQ)
                {
                  returnTime = returnTime + (localHeadPtr->intArg2 *
                                                      configPtr->procCycleRate);
                }
              //if the process is dev, multiply process by ioCycleRate
              else if( compareString(localHeadPtr->command, "dev") == STR_EQ)
                {
                  returnTime = returnTime + (localHeadPtr->intArg2 *
                                                        configPtr->ioCycleRate);
                }
           // moves onto the next process
           localHeadPtr = localHeadPtr->nextNode;
         }

      // returns the completed PCB run time
      return returnTime;
   }

/*
 * Function Name: checkMaximumMemory
 * Algorithm: The maximum memory was specified in the requirements document
              this function checks to see if the memory is greater than that
              size and sets it to it
 * Preconditon: ConfigDataType is given to get the memAvailable
 * Postcondition: returns new memory amount
 * Exceptions: none
 * Notes: none
 */
int checkMaximumMemory( ConfigDataType *configPtr )
  {
    // maximumMemory is based off of the requirements document
    int maximumMemory = 102400;

    // local pointer to get the memAvailable from config file
    ConfigDataType *localConfigPtr = configPtr;

    // returns maximumMemory if it is less than memAvailable
    if( maximumMemory < localConfigPtr->memAvailable)
      {
        return maximumMemory;
      }

    // returns memAvailable if its smaller than maximumMemory
    return localConfigPtr->memAvailable;
  }

/*
 * Function Name: clearPCBList
 * Algorithm: loops through PCB linked list, clearing and freeing
              the PCBs
 * Preconditon: given head pointer of PCB linked list
 * Postcondition: PCB linked list is cleared and freed
 * Exceptions: none
 * Notes: none
 */
PCB *clearPCBList(PCB *pcbPtr)
   {
     // as long as the pcbPtr is not null continue
     if( pcbPtr != NULL)
        {
          // call the nextNode of pcbPtr recursively
          clearPCBList( pcbPtr->nextNode );

          // frees the memory allocated for the PCB
          free(pcbPtr);

          // sets the pcbPtr = NUll
          pcbPtr = NULL;
        }

      // returns NULL as the PCB should be NULL now
      return NULL;
   }

/*
 * Function Name: clearStringToFileList
 * Algorithm: loops through the StringToFile linked list, clearing and freeing
              StringToFile along the way
 * Preconditon: given head pointer to the StringToFile linked list
 * Postcondition: returns freed and null linked list
 * Exceptions: none
 * Notes: none
 */
StringToFile *clearStringToFileList(StringToFile *stringToFilePtr)
   {
     // as long as the stringToFilePtr is not null continue
     if( stringToFilePtr != NULL)
        {

          // call the nextNode of stringToFilePtr recursively
          clearStringToFileList( stringToFilePtr->nextNode );

          // frees the memory allocated for the log string
          free(stringToFilePtr);

          // sets the stringToFilePtr = NUll
          stringToFilePtr = NULL;
        }

      // returns NULL as the stringToFilePtr should be NULL now
      return NULL;
   }

/*
 * Function Name: countAppStart
 * Algorithm: loops through the meta data by starting at the head pointer
 *            checks to see if each op code contains app and start, if it
 *            does it increases the amount of appStartTotal
 * Preconditon: given head pointer to meta data
 * Postcondition: total amount of app starts are returned
 * Exceptions: none
 * Notes: none
 */
int countAppStart( OpCodeType *localPtr )
    {
      // local variables
      int appStartTotal = 0;

      // loop through meta data list to check for 'app' and 'start' in same
      // op code
      while( localPtr != NULL )
         {
           if( compareString( localPtr->command, "app") == STR_EQ
              && compareString( localPtr->strArg1, "start") == STR_EQ)
                 {
                   appStartTotal++;
                 }

           localPtr = localPtr->nextNode;
         }

      return appStartTotal;
    }

/*
 * Function Name: displayOpCode
 * Algorithm: grabs the current process from the PCB and runs checks to
              determine what the process command, inOutArg, and strArg1
              are to display them
 * Preconditon: given head pointer to PCB, config, Booleans to track
                log output and the file to log to
 * Postcondition: the correct print statements have been executed
 * Exceptions: none
 * Notes: none
 */
void displayOpCode( PCB *pcbPtr, ConfigDataType *configPtr,
                   Boolean logToMonitor, Boolean logToFile, int *memoryAvailable,
                                                    StringToFile *logFilePtr)
   {
     // local variables for function use
     // variables are used for accessTimer
     char timeStr[MAX_STR_LEN];
     char tempStr[MAX_STR_LEN];
     double tempTime = accessTimer( LAP_TIMER, timeStr );

     // keeps track of Segmentation Fault
     Boolean memBoolean = False;

     // Grabs StringToFile linked list and generates temp log string(outputLine)
     StringToFile *logToFilePtr = logFilePtr;
     char outputLine[MAX_STR_LEN];
     outputLine[0] = '\0';

     // Allocates a local memory availableMemory
     int *localMemoryAvailable = memoryAvailable;

     // runTime is used for pthreads, pthread is used for running time
     int runTime = 0;
     pthread_t threadID;

     // each op code has the same beginning, so I print that here.
     if( logToMonitor == True )
       {
          printf("\n%10.6f, Process: %d, ", tempTime, pcbPtr->pid);
       }
     // log to file includes combining the required strings into the outputline
     // then adding it to the log to linked list, then resetting outputLine
     if( logToFile == True )
       {
         sprintf(outputLine, "\n%10.6f, Process: %d, ", tempTime,
                                                              pcbPtr->pid);
         addLogToFile(logToFilePtr, outputLine);
         outputLine[0] = '\0';
       }

     // Check what command the OpCode has, if it is CPU here, continue...
     if( compareString(pcbPtr->processStart->command, "cpu") == STR_EQ )
        {
          // logs that the command is cpu
          if( logToMonitor == True )
            {
              printf("cpu");
            }
          // log to file includes combining the required strings into the outputline
          // then adding it to the log to linked list, then resetting outputLine
          if ( logToFile == True )
            {
              sprintf(outputLine,  "cpu");
              addLogToFile(logToFilePtr, outputLine);
              outputLine[0] = '\0';
            }

          // if the argument of the op code is process continue...
          if( compareString(pcbPtr->processStart->strArg1, "process") == STR_EQ)
            {
              // logs the process operation start
              if( logToMonitor == True )
                {
                  printf(" process operation start");
                }
              // log to file includes combining the required strings into the outputline
              // then adding it to the log to linked list, then resetting outputLine
              if ( logToFile == True )
                {
                  sprintf(outputLine, " process operation start");
                  addLogToFile(logToFilePtr, outputLine);
                  outputLine[0] = '\0';
                }

               // creates runTime based off of the cycle rate and the time val
               runTime = pcbPtr->processStart->intArg2*configPtr->procCycleRate;
               // creates pthread and joins it for accurate pthread usage
               pthread_create(&threadID, NULL, threadIOProcess, (void*)&(runTime));
               pthread_join(threadID, NULL);

               // now that the process has finished 'running', the process
               // has now ended so we print that it ended
               tempTime = accessTimer( LAP_TIMER, timeStr );
               if( logToMonitor == True )
                 {
                   printf("\n%10.6f, Process: %d, cpu process operation end",
                                                         tempTime, pcbPtr->pid);
                 }
               // log to file includes combining the required strings into the outputline
               // then adding it to the log to linked list, then resetting outputLine
               if ( logToFile == True )
                 {
                   sprintf(outputLine, "\n%10.6f, Process: %d, cpu \
process operation end", tempTime, pcbPtr->pid);
                   addLogToFile(logToFilePtr, outputLine);
                   outputLine[0] = '\0';
                 }
            }
        }

      // if the process is not CPU we then check to see if it is a dev
      else if( compareString(pcbPtr->processStart->command, "dev") == STR_EQ )
        {
          // if the function is a dev, we need to check if it is in or out
          // if this is in we continue
          if( compareString(pcbPtr->processStart->inOutArg, "in" ) == STR_EQ)
              {
                /*
                 * the next set of if, else if statements check to see what
                 * kind of dev input we need to display, when we find it
                 * we copy it to a tempStr for printing
                 */
                if( compareString(pcbPtr->processStart->strArg1, "hard drive")
                                                                     == STR_EQ)
                  {
                    copyString(tempStr, "hard drive");
                  }
                else if( compareString(pcbPtr->processStart->strArg1, "monitor")
                                                                      == STR_EQ)
                  {
                    copyString(tempStr, "monitor");
                  }
                else if( compareString(pcbPtr->processStart->strArg1,
                                                     "sound signal") == STR_EQ )
                  {
                    copyString(tempStr, "sound signal");
                  }
                else if( compareString(pcbPtr->processStart->strArg1,
                                                              "usb") == STR_EQ )
                  {
                    copyString(tempStr, "usb");
                  }
                else if( compareString(pcbPtr->processStart->strArg1,
                                                     "video signal") == STR_EQ )
                  {
                    copyString(tempStr, "video signal");
                  }
                else if( compareString(pcbPtr->processStart->strArg1,
                                                         "keyboard") == STR_EQ )
                  {
                   copyString(tempStr, "keyboard");
                  }
                else if( compareString(pcbPtr->processStart->strArg1,
                                                           "serial") == STR_EQ )
                  {
                   copyString(tempStr, "serial");
                  }
                // finish looking for dev operation

                // log what dev input opertion we have found and started
                if( logToMonitor == True )
                  {
                    printf("%s input operation start", tempStr);
                  }
                // log to file includes combining the required strings into the outputline
                // then adding it to the log to linked list, then resetting outputLine
                if ( logToFile == True )
                  {
                    sprintf(outputLine, "%s input operation start",tempStr);
                    addLogToFile(logToFilePtr, outputLine);
                    outputLine[0] = '\0';
                  }

                // creates runTime based off of the cycle rate and the time val
                runTime = pcbPtr->processStart->intArg2*configPtr->ioCycleRate;
                // creates pthread and joins it for accurate pthread usage
                pthread_create(&threadID, NULL, threadIOProcess, (void*)&(runTime));
                pthread_join(threadID, NULL);

                // display that the operation has now ended
                tempTime = accessTimer( LAP_TIMER, timeStr );
                if( logToMonitor == True )
                  {
                    printf("\n%10.6f, Process: %d, %s input operation end",
                                                tempTime, pcbPtr->pid, tempStr);
                  }
                // log to file includes combining the required strings into the outputline
                // then adding it to the log to linked list, then resetting outputLine
                if ( logToFile == True )
                  {
                    sprintf(outputLine, "\n%10.6f, Process: %d, %s input \
operation end", tempTime, pcbPtr->pid, tempStr);
                    addLogToFile(logToFilePtr, outputLine);
                    outputLine[0] = '\0';
                  }
             }

          // earlier we checked if it was an input, now we check for output
          else if( compareString(pcbPtr->processStart->inOutArg, "out")
                                                                     == STR_EQ )
            {
              /*
               * the next set of if, else if statements check to see what
               * kind of dev output we need to display, when we find it
               * we copy it to a tempStr for printing
               */
              if( compareString(pcbPtr->processStart->strArg1, "hard drive")
                                                                      == STR_EQ)
                {
                  copyString(tempStr, "hard drive");
                }
              else if( compareString(pcbPtr->processStart->strArg1, "monitor")
                                                                      == STR_EQ)
                {
                  copyString(tempStr, "monitor");
                }
              else if( compareString(pcbPtr->processStart->strArg1,
                                                     "sound signal") == STR_EQ )
                {
                  copyString(tempStr, "sound signal");
                }
             else if( compareString(pcbPtr->processStart->strArg1, "usb")
                                                                     == STR_EQ )
                {
                  copyString(tempStr, "usb");
                }
             else if( compareString(pcbPtr->processStart->strArg1,
                                                     "video signal") == STR_EQ )
                {
                  copyString(tempStr, "video signal");
                }
             else if( compareString(pcbPtr->processStart->strArg1,
                                                         "keyboard") == STR_EQ )
               {
                 copyString(tempStr, "keyboard");
               }
             else if( compareString(pcbPtr->processStart->strArg1,
                                                           "serial") == STR_EQ )
               {
                 copyString(tempStr, "serial");
               }
              // finish looking for dev operation

              // log what dev output opertion we have found and started
              if( logToMonitor == True )
               {
                  printf("%s output operation start", tempStr);
               }
              // log to file includes combining the required strings into the outputline
              // then adding it to the log to linked list, then resetting outputLine
              if ( logToFile == True )
               {
                 sprintf(outputLine, "%s output operation start", tempStr);
                 addLogToFile(logToFilePtr, outputLine);
                 outputLine[0] = '\0';
               }

              // creates runTime based off of the cycle rate and the time val
              runTime = pcbPtr->processStart->intArg2*configPtr->ioCycleRate;
              // creates pthread and joins it for accurate pthread usage
              pthread_create(&threadID, NULL, threadIOProcess, (void*)&(runTime));
              pthread_join(threadID, NULL);

              // display that the operation has now ended
              tempTime = accessTimer( LAP_TIMER, timeStr );
              if( logToMonitor == True )
                  {
                    printf("\n%10.6f, Process: %d, %s output operation end",
                                                tempTime, pcbPtr->pid, tempStr);
                  }
              // log to file includes combining the required strings into the outputline
              // then adding it to the log to linked list, then resetting outputLine
              if ( logToFile == True )
                  {
                    sprintf(outputLine, "\n%10.6f, Process: %d, %s output\
operation end", tempTime, pcbPtr->pid, tempStr);
                    addLogToFile(logToFilePtr, outputLine);
                    outputLine[0] = '\0';
                  }
            }
        }
        // after CPU and DEV the only remaining processStart is MEM
        else if( compareString(pcbPtr->processStart->command, "mem") == STR_EQ )
          {
            // the two different mem operations are allocate and access
            if( compareString(pcbPtr->processStart->strArg1, "allocate" ) == STR_EQ)
                {
                  // prints and adds to the log string linked list attempting
                  // mem allocate request
                  tempTime = accessTimer( LAP_TIMER, timeStr );
                  if( logToMonitor == True )
                      {
                        printf("attempting mem allocate request");
                      }

                  if ( logToFile == True )
                      {
                        sprintf(outputLine, "attempting mem allocate request");
                        addLogToFile(logToFilePtr, outputLine);
                        outputLine[0] = '\0';
                      }

                  // memBoolean is set to the result of the attempt at mem
                  // allocation
                  memBoolean = attemptMemAllocation( pcbPtr->processStart->intArg2,
                    pcbPtr->processStart->intArg3, localMemoryAvailable);

                  // timer is called after mem allocation attempt
                  tempTime = accessTimer( LAP_TIMER, timeStr );

                  // prints/ adds successful logString to the linked list if the
                  // memBoolean is true, meaning the allocation worked
                  if(memBoolean == True)
                    {
                      if( logToMonitor == True )
                          {
                            printf("\n%10.6f, Process: %d, successful mem allocate request",
                                                        tempTime, pcbPtr->pid);
                          }

                      if ( logToFile == True )
                          {
                            sprintf(outputLine, "\n%10.6f, Process: %d, successful mem allocate request",
                                                        tempTime, pcbPtr->pid);
                            addLogToFile(logToFilePtr, outputLine);
                            outputLine[0] = '\0';
                          }
                    }
                  // prints/ adds failure logString to the linked list if the
                  // memBoolean is true, meaning the allocation worked
                  else if(memBoolean == False)
                    {
                      // The allocation failed, so the process is set to EXIT
                      // which causes a Segmentation fault
                      pcbPtr->currState = EXIT;
                      if( logToMonitor == True )
                          {
                            printf("\n%10.6f, Process: %d, failed mem allocate request",
                                                        tempTime, pcbPtr->pid);
                          }

                      if ( logToFile == True )
                          {
                            sprintf(outputLine, "\n%10.6f, Process: %d, failed mem allocate request",
                                                        tempTime, pcbPtr->pid);
                            addLogToFile(logToFilePtr, outputLine);
                            outputLine[0] = '\0';
                          }
                    }
                }
            // the other mem operation is access, follows allocation closely
            else if( compareString(pcbPtr->processStart->strArg1, "access")
                                                                       == STR_EQ )
              {
                // prints and adds to the log string linked list attempting
                // mem access request
                tempTime = accessTimer( LAP_TIMER, timeStr );
                if( logToMonitor == True )
                    {
                      printf("attempting mem access request");
                    }

                if ( logToFile == True )
                    {
                      sprintf(outputLine, "attempting mem access request");
                      addLogToFile(logToFilePtr, outputLine);
                      outputLine[0] = '\0';
                    }

                // memBoolean is set to the result of the attempt at mem
                // access
                memBoolean = attemptMemAccess( pcbPtr->processStart->intArg2, pcbPtr->processStart->intArg3, localMemoryAvailable);

                // timer is called after mem allocation attempt
                tempTime = accessTimer( LAP_TIMER, timeStr );

                // prints/ adds successful logString to the linked list if the
                // memBoolean is true, meaning the access worked
                if(memBoolean == True)
                  {
                    if( logToMonitor == True )
                        {
                          printf("\n%10.6f, Process: %d, successful mem access request", tempTime, pcbPtr->pid);
                        }

                    if ( logToFile == True )
                        {
                          sprintf(outputLine, "\n%10.6f, Process: %d, successful mem access request", tempTime, pcbPtr->pid);
                          addLogToFile(logToFilePtr, outputLine);
                          outputLine[0] = '\0';
                        }
                  }

                // prints/ adds failure logString to the linked list if the
                // memBoolean is true, meaning the access worked
                else if(memBoolean == False)
                  {
                    // The access failed, so the process is set to EXIT
                    // which causes a Segmentation fault
                    pcbPtr->currState = EXIT;
                    if( logToMonitor == True )
                        {
                          printf("\n%10.6f, Process: %d, failed mem access request", tempTime, pcbPtr->pid);
                        }

                    if ( logToFile == True )
                        {
                          sprintf(outputLine, "\n%10.6f, Process: %d, failed mem access request", tempTime, pcbPtr->pid);
                          addLogToFile(logToFilePtr, outputLine);
                          outputLine[0] = '\0';
                        }
                  }
              }
          }
   }

/*
 * Function Name: getNextPCB
 * Algorithm: gets the CPU sched code and calls functions to get the correct
              PCB depending on the CPU sched code
 * Preconditon: needs the pointers to the metadata, config, and PCB linked
                list as well as the master loop index
 * Postcondition: returns the correct next PCB
 * Exceptions: none
 * Notes: none
 */
PCB *getNextPCB(OpCodeType *localMetaDataPtr,
                           ConfigDataType *configLocalPtr, PCB *localPCBHeadPtr,
                      int currentLoopIndex, int *traversedPCB, int amountOfPCBs)
  {
    // get the next process, this is where we check for FCFS, SRTF etc
    // configLocalPtr->cpuSchedCode
    char cpuCode[MAX_STR_LEN];
    PCB *returnNextPCB = NULL;
    PCB *localPCBPtr = localPCBHeadPtr;
    int *localTraversedPCB = traversedPCB;
    configCodeToString( configLocalPtr->cpuSchedCode, cpuCode );

    // if the cpu sched code is FCFS-N continue
    if(compareString(cpuCode, "FCFS-N") == STR_EQ)
      {
        // get the next PCB according to FCFS-N
        returnNextPCB = getNextPCBFCFSN( localPCBPtr, currentLoopIndex );
      }
    // if the cpu sched code is SJF-N continue
    else if(compareString(cpuCode, "SJF-N") == STR_EQ)
      {
        // get the next PCB according to SJF-N
        returnNextPCB = getNextPCBSJFN( localPCBPtr, localTraversedPCB, amountOfPCBs);
      }
    // if the cpu sched code is SRTF-P continue
    else if(compareString(cpuCode, "SRTF-P") == STR_EQ)
      {
        // temp print statement as this is not implemented
        printf("\nSRTF-P not implemented yet.\n");
      }
    // if the cpu sched code is FCFS-P continue
    else if(compareString(cpuCode, "FCFS-P") == STR_EQ)
      {
        // temp print statement as this is not implemented
        printf("\nFCFS-P not implemented yet.\n");
      }
    // if the cpu sched code is RR-P continue
    else if(compareString(cpuCode, "RR-P") == STR_EQ)
      {
        // temp print statement as this is not implemented
        printf("\nRR-P not implemented yet.");
      }

    // returns the next PCB
    return returnNextPCB;
  }

/*
 * Function Name: getNextPCBFCFSN
 * Algorithm: returns the next PCB according to FCFSN protocals
 * Preconditon: given the loop index of the master loop and the
                PCB linked list
 * Postcondition: returns the correct next PCB
 * Exceptions: none
 * Notes: none
 */
PCB *getNextPCBFCFSN( PCB *localPCBHeadPtr, int currentLoopIndex )
   {
     PCB *localPCBPtr = localPCBHeadPtr;
     // returns NULL if the PCB linked list is NULL
     if(localPCBPtr == NULL)
       {
         return NULL;
       }

     // if the current PCB->pid equals the master loop index, we have the right
     // PCB
     if( localPCBPtr->pid == currentLoopIndex)
        {
          return localPCBPtr;
        }

     // call the next PCB in the list
     return getNextPCBFCFSN(localPCBPtr->nextNode, currentLoopIndex);
   }

/*
 * Function Name: getNextPCBFCFSN
 * Algorithm: returns the next PCB according to FCFSN protocals
 * Preconditon: given the loop index of the master loop and the
                PCB linked list
 * Postcondition: returns the correct next PCB
 * Exceptions: none
 * Notes: none
 */
PCB *getNextPCBSJFN( PCB *localPCBHeadPtr, int *traversedPCB, int amountOfPCBs )
   {
     // variables for function use
     PCB *localPCBPtr = localPCBHeadPtr;
     int *localTraversedPCB = traversedPCB;
     int shortestRuntimeIndex = 0;

     // returns NULL if the PCB linked list is NULL
     if(localPCBPtr == NULL)
       {
         return NULL;
       }

     // grabs the PID of the PCB with the shortest runtime
     shortestRuntimeIndex = getShortestRuntime( localPCBPtr, localTraversedPCB, amountOfPCBs);

     // loops through the PCB linked list
     while(localPCBPtr != NULL)
      {
        // the current PCB is equal to the Index with the shortest runtime
        // return that PCB
        if(localPCBPtr->pid == shortestRuntimeIndex)
          {
            return localPCBPtr;
          }
        // calls the next node of the PCB linked list
        localPCBPtr = localPCBPtr->nextNode;
      }

     // no shortest job was found
     return NULL;
   }

/*
 * Function Name: getShortestRuntime
 * Algorithm: loops through PCB linked list, seeing if the each PCB is in the
 *            int traversedPCB array. If its not in the traversedPCB array,
 *            then it checks to see if it is the shortest runtime currently
 *            this will continue until all PCB's have gone through the loop.
 *            Finally, the PCB selected is added to the Traversed PCBs and returned
 * Preconditon: PCB head pointer, traversedPCB array, and the total amount of PCBS
 *              given
 * Postcondition: shortest runtime PCB is added to traversed PCBs and returned
 * Exceptions: none
 * Notes: none
 */
int getShortestRuntime( PCB *localPCBHeadPtr, int *traversedPCB, int amountOfPCBs )
   {
     // local variables for use in function declared
     PCB *localPCBPtr = localPCBHeadPtr;
     int *localTraversedPCB = traversedPCB;

     // Variables declared so that the first PCB not in the traversedPCB will
     // be the shortest run time.
     int currentShortestPCB = amountOfPCBs + 1;
     int currentLowestRuntime = 100000;

     // loops through the entire PCB list
     while( localPCBPtr != NULL )
      {
        // checks to see if the PCB is in the traversedPCB array
        if(pidAlreadyTraversed(localPCBPtr->pid, localTraversedPCB, amountOfPCBs) == False)
         {
           // checks to see if the current PCB has the shortest runtime
           if( localPCBPtr->processRunTime < currentLowestRuntime )
              {
                  // assigns the current PCB to the shortest runtime variables
                  currentLowestRuntime = localPCBPtr->processRunTime;
                  currentShortestPCB = localPCBPtr->pid;
              }
         }
         // moves onto the next node
         localPCBPtr = localPCBPtr->nextNode;
      }
     // adds the found node to the traversedPCB array so that we don't call it
     // again
     addPCBToTraversedPCBs(currentShortestPCB, localTraversedPCB, amountOfPCBs);

     // returns the current PCB so we can loop through its processes
     return currentShortestPCB;
   }

/*
 * Function Name: pidAlreadyTraversed
 * Algorithm: loops through the array to see if the currentPCBPID is in it
 * Preconditon: needs the PID of the current PCB, the array of traversedPCBs
 *              and the total number of pcbs for easy for loop travel
 * Postcondition: returns true if PCB is in the array, false otherwise
 * Exceptions: none
 * Notes: none
 */
Boolean pidAlreadyTraversed( int currentPCBPID, int *traversedPCB, int amountOfPCBs )
  {
    // local variables and temporary index declared
    int sizeOfTraversedPCB = amountOfPCBs;
    int *localTraversedPCB = traversedPCB;
    int index;

    // loops through entire traversed pcb array
    for( index = 0; index < sizeOfTraversedPCB; index++ )
      {
        // checks to see if the PID is in the array
        if( localTraversedPCB[index] == currentPCBPID )
          {
            return True;
          }
      }
    return False;
  }

/*
 * Function Name: runSim
 * Algorithm: master driver for simulator operations;
 *            conducts OS simulation with varying scheduling strategies
 *            and varying numbers of processes
 * Preconditon: given head pointer to config data and meta data
 * Postcondition: simulation is provided,file output is provided as configured
 * Exceptions: none
 * Notes: none
 */
void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr )
   {
      // local variables for function use
      int amountOfPCBs = 0;
      int currentPCBIndex = 0;
      int index = 0;
      int *traversedPCB;
      OpCodeType *localMetaDataPtr = metaDataMstrPtr;
      PCB *currentPCB = NULL;
      ConfigDataType *configLocalPtr = configPtr;
      PCB *localPCBHeadPtr = NULL;

      // initializes the maximum memory and a temp memory for use
      int maximumMemory = checkMaximumMemory( configLocalPtr );
      int memoryAvailable = maximumMemory;

      // flags for use in code
      Boolean atAppEnd = False;
      Boolean logToMonitor = False;
      Boolean logToFile = False;

      // used in accessTimer function
      double currTime;
      char timeStr[MAX_STR_LEN];

      // file pointer for use in code
      FILE *filePointer;

      // I had to initialize the log string linked list with this instead of
      // just null to avoid issues
      StringToFile logHead = {"", NULL};
      StringToFile *logToFilePtr = &logHead;

      // temp string for log string use
      char outputLine[MAX_STR_LEN];
      outputLine[0] = '\0';

      /*
       * Depending on the logToCode from the config file, we then set
       * logToMonitor and logToFile to True or False
       */
      if( configLocalPtr->logToCode == LOGTO_BOTH_CODE)
        {
          logToMonitor = True;
          logToFile = True;
        }

      else if ( configLocalPtr->logToCode == LOGTO_MONITOR_CODE)
        {
          logToMonitor = True;
          logToFile = False;
        }

      else if ( configLocalPtr->logToCode == LOGTO_FILE_CODE)
        {
          logToMonitor = False;
          logToFile = True;
        }

      // Opens the file to log output to
      filePointer = fopen(configLocalPtr->logToFileName, "w");

      // if their is no file, we print error message
      if( filePointer == NULL )
        {
          printf("Log File Failed to Open");
          logToFile = False;
        }

      // Display Simulator Log File Header, this is always called
      logFileToHeader( configLocalPtr, filePointer );

      // closes file, I need to append the rest of the log strings
      fclose(filePointer);

      // if only printing to file, we display this to monitor
      if( logToMonitor == False && logToFile == True )
         {
           printf("Simulator running with output to file");
         }
      // logs title to monitor or file depending on flags
      if( logToMonitor == True )
        {
          printf( "\n================\n" );
          printf( "Begin Simulation\n" );
        }

      if ( logToFile == True )
        {
          sprintf(outputLine, "\n\n================\n");
          addLogToFile(logToFilePtr, outputLine);
          outputLine[0] = '\0';

          sprintf(outputLine, "Begin Simulation\n");
          addLogToFile(logToFilePtr, outputLine);
          outputLine[0] = '\0';
        }


      // grabs the starting time from accessTimer, and displays the message to
      // the monitor or log file depending on flags
      currTime = accessTimer( ZERO_TIMER, timeStr );
      if( logToMonitor == True )
        {
           printf( "\n%10.6f, OS: Simulator start", currTime );
        }
      if ( logToFile == True )
        {
          sprintf(outputLine, "\n%10.6f, OS: Simulator start", currTime);
          addLogToFile(logToFilePtr, outputLine);
          outputLine[0] = '\0';
        }


      // Gather the total amount of processes that should be created
      // by counting total amount of 'app start'
      amountOfPCBs = countAppStart( localMetaDataPtr );

      // Need to create an int array to avoid repeats of PCBs
      traversedPCB = (int*)malloc( amountOfPCBs * sizeof(int));

      // initialize the traversedPCB as amountOfPCBs + 1 so no object in the
      // array is a PID of a PCB we would actually use
      for(index = 0; index < amountOfPCBs; index++)
        {
          traversedPCB[index] = amountOfPCBs + 1;
        }

      // loop through the number of process count and create PCB linked list
      while( currentPCBIndex < amountOfPCBs )
          {
            // loop through the entire meta data file
            if( localMetaDataPtr != NULL)
               {
                // when the meta data pointer is app start, create new PCB
                if( compareString( localMetaDataPtr->command, "app" ) == STR_EQ
                    && compareString( localMetaDataPtr->strArg1, "start" )
                                                                     == STR_EQ )
                      {
                        // creates a new PCB at the app start location
                        localPCBHeadPtr = addPCBNode( localPCBHeadPtr,
                          localMetaDataPtr, currentPCBIndex, configLocalPtr );

                        // gets the current time and displays PCB state status
                        // change from NEW to READY
                        currTime = accessTimer( LAP_TIMER, timeStr );
                        if( logToMonitor == True )
                          {
                             printf("\n%10.6f, OS: Process %d set to READY \
state from NEW state", currTime, currentPCBIndex );
                          }

                        if ( logToFile == True )
                          {
                            sprintf(outputLine, "\n%10.6f, OS: Process %d \
set to READY state from NEW state", currTime, currentPCBIndex);
                            addLogToFile(logToFilePtr, outputLine);
                            outputLine[0] = '\0';
                          }

                        // PCB has been created so we increase the PCB amount
                        currentPCBIndex++;
                      }
               }

            // move on to the next node in the meta data file list
            localMetaDataPtr = localMetaDataPtr->nextNode;
          }

      // reset PCB index and loop through each PCB and print the processes
      // associated with each PCB
      currentPCBIndex = 0;
      while ( currentPCBIndex < amountOfPCBs)
        {
           // atAppEnd keeps track of if we reach the end of a PCB
           // so because we loop through each PCB I reset atAppEnd
           // at the beginning of the loop
           atAppEnd = False;

           // calls getNextPCB, I have a function because it could be different
           // depending on the cpu sched code
           currentPCB = getNextPCB(localMetaDataPtr, configLocalPtr,
                localPCBHeadPtr, currentPCBIndex, traversedPCB, amountOfPCBs );

           // now that we have a new PCB, we display which PCB we selected
           currTime = accessTimer( LAP_TIMER, timeStr );
           if( logToMonitor == True )
             {
                printf("\n%10.6f, OS: Process %d selected with %d ms remaining",
                        currTime, currentPCB->pid, currentPCB->processRunTime );
             }

           if ( logToFile == True )
             {
               sprintf(outputLine, "\n%10.6f, OS: Process %d selected with %d \
ms remaining", currTime, currentPCB->pid, currentPCB->processRunTime);
               addLogToFile(logToFilePtr, outputLine);
               outputLine[0] = '\0';
             }

           // set the current state of the PCB to RUNNING
           currentPCB->currState = RUNNING;

           // display that we are now running the PCB
           currTime = accessTimer( LAP_TIMER, timeStr );
           if( logToMonitor == True )
             {
                printf("\n%10.6f, OS: Process %d set from READY to RUNNING\n",
                                                    currTime, currentPCB->pid );
             }
           if ( logToFile == True )
             {
               sprintf(outputLine, "\n%10.6f, OS: Process %d set from READY to\
RUNNING\n", currTime, currentPCB->pid);
               addLogToFile(logToFilePtr, outputLine);
               outputLine[0] = '\0';
             }

           // Loop through every process of a PCB and display it properly
           // check to see if the process is app end, if app end increaese PCB
           while( atAppEnd == False && currentPCB->currState != EXIT )
              {
                /*
                 * Each PCB initially starts at app start, we don't need to
                 * print this OpCodeType, so we move to the next processStart
                 * after that we want to go to the next process for each loop
                 */
                currentPCB->processStart = currentPCB->processStart->nextNode;

                // if the current process is app end, we need to end the PCB
                if( compareString( currentPCB->processStart->command, "app" )
                                                                      == STR_EQ
                   && compareString( currentPCB->processStart->strArg1, "end" )
                                                                     == STR_EQ )
                     {
                       // prints that we have ended the PCB
                       currTime = accessTimer( LAP_TIMER, timeStr );
                       if( logToMonitor == True )
                         {
                           printf("\n\n%10.6f, OS: Process %d ended", currTime,
                                                              currentPCB->pid );
                         }

                       if ( logToFile == True )
                         {
                           sprintf(outputLine, "\n\n%10.6f, OS: Process %d \
ended", currTime, currentPCB->pid );
                           addLogToFile(logToFilePtr, outputLine);
                           outputLine[0] = '\0';
                         }

                       // sets the current state of PCB to EXIT
                       currentPCB->currState = EXIT;

                       // displays that the proccess was set to EXIT
                       currTime = accessTimer( LAP_TIMER, timeStr );
                       if( logToMonitor == True )
                         {
                           printf("\n%10.6f, OS: Process %d set to EXIT",
                                                    currTime, currentPCB->pid );

                         }
                       if ( logToFile == True )
                         {
                           sprintf(outputLine, "\n%10.6f, OS: Process %d set \
to EXIT", currTime, currentPCB->pid );
                           addLogToFile(logToFilePtr, outputLine);
                           outputLine[0] = '\0';
                         }

                       // moves onto the next PCB and sets atAppEnd to true
                       currentPCBIndex++;
                       atAppEnd = True;
                     }
                // if we are not at app end, we display the process
                else
                  {
                    // calls displayOpCode for the process because each OpCode
                    // can be different
                    displayOpCode( currentPCB, configLocalPtr, logToMonitor,
                                      logToFile, &memoryAvailable,logToFilePtr);

                    // if after the displayOpCode the currstate is EXIT
                    // this means there is a segmentation fault from memory
                    if( currentPCB->currState == EXIT)
                      {
                        // prints segmentation fault issues
                        currTime = accessTimer( LAP_TIMER, timeStr );
                        if( logToMonitor == True )
                          {
                            printf("\n\n%10.6f, OS: Segmentation fault, Process %d ended",
                                                     currTime, currentPCB->pid );

                          }
                        if ( logToFile == True )
                          {
                            sprintf(outputLine, "\n\n%10.6f, OS: Segmentation fault, Process %d ended",
                                                     currTime, currentPCB->pid );
                            addLogToFile(logToFilePtr, outputLine);
                            outputLine[0] = '\0';
                          }

                        currTime = accessTimer( LAP_TIMER, timeStr );
                        if( logToMonitor == True )
                          {
                            printf("\n%10.6f, OS: Process %d set to EXIT",
                                                     currTime, currentPCB->pid );

                          }
                        if ( logToFile == True )
                          {
                            sprintf(outputLine, "\n%10.6f, OS: Process %d set \
 to EXIT", currTime, currentPCB->pid );
                            addLogToFile(logToFilePtr, outputLine);
                            outputLine[0] = '\0';
                          }
                        // moves onto the next PCB
                        currentPCBIndex++;
                      }
                  }
              }
         }

      // frees the int array as I used malloc to allocate space for it
      free(traversedPCB);

      // stops the current system
      currTime = accessTimer( LAP_TIMER, timeStr );
      if( logToMonitor == True )
        {
          printf( "\n%10.6f, OS: System stop", currTime );
        }

      if ( logToFile == True )
        {
          sprintf(outputLine, "\n%10.6f, OS: System stop", currTime );
          addLogToFile(logToFilePtr, outputLine);
          outputLine[0] = '\0';
        }

      // ends the Simulator
      currTime = accessTimer( STOP_TIMER, timeStr );
      if( logToMonitor == True )
        {
          printf( "\n%10.6f, OS: Simulator end\n", currTime );
        }

      if ( logToFile == True )
        {
          sprintf(outputLine, "\n%10.6f, OS: Simulator end\n", currTime );
          addLogToFile(logToFilePtr, outputLine);
          outputLine[0] = '\0';
        }

      // clears the PCB list and frees memory removing mem leaks
      localPCBHeadPtr = clearPCBList(localPCBHeadPtr);

      //end simulation
      if( logToMonitor == True )
        {
          printf( "\nEnd Simulation - Complete\n" );
          printf( "=========================\n" );
        }
      if ( logToFile == True )
        {
          sprintf(outputLine, "\nEnd Simulation - Complete\n" );
          addLogToFile(logToFilePtr, outputLine);
          outputLine[0] = '\0';

          sprintf(outputLine, "=========================\n" );
          addLogToFile(logToFilePtr, outputLine);
          outputLine[0] = '\0';
        }

      // loop through the log string linked list if the logToFile flag is True
      if(logToFile == True)
        {
          // opens file to append information to
          filePointer = fopen(configLocalPtr->logToFileName, "a+");

          // loops through entire log string linked list
          while(logToFilePtr != NULL)
          {
            // puts the log string linked list information into the file
            fputs(logToFilePtr->logString, filePointer);
            logToFilePtr = logToFilePtr->nextNode;
          }

          // closes the file
          fclose(filePointer);
        }

      // free and reset the logToFilePtr linked list
      logToFilePtr = clearStringToFileList(logToFilePtr);
   }


/*
 * Function Name: threadIOProcess
 * Algorithm: runs runTimer with the pthread information
 * Preconditon: runTime is given
 * Postcondition: runTime is run for given time
 * Exceptions: none
 * Notes: none
 */
void *threadIOProcess( void *arg )
  {
    // Convert argument to an integer value to be ran
    int runtime = * (int*) arg;

    // Runs the timer according to the generated runTime
    runTimer(runtime);

    // exit thread
    pthread_exit(NULL);
  }
