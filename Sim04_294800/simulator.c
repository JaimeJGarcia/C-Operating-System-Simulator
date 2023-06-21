// header files
#include "simulator.h"

/*
Name: runSim
Process: primary simulation driver
Funciton Input/Parameters: configuration data (ConfigDataType *),
                           metadata (OpCodeType *)
Function Ouput/Parameters: none
Function Output/Returned: none
Device Input/device: none
Device Output/device: none
Dependencies: tbd
*/
void runSim( ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr )
   {
    // initialize variables
    int segFaults = 0;
    int clearedProcesses = 0;
    int processCount = 0;
    int processesExited = 0;
    int processesBlocked = 0;
    int activeProcesses = 0;
    char blankString[MAX_STR_LEN] = " ";

    // parameters assigned to local variables
    ConfigDataType *configPointer = configPtr;
    OpCodeType *metaPointer = metaDataMstrPtr;

    // pcbType pointers
    pcbType *pcbHead = NULL;
    pcbType *pcbLoopPointer = NULL;
    pcbType *pcbNodePointer = NULL;

    // interuptType pointers
    interruptType *interruptHead = NULL;
    interruptType *interruptLoopPointer = interruptHead;


    // schedule code variables
    char scheduleCode[ SMALL_STR_LEN ];
    configCodeToString( configPointer -> cpuSchedCode, scheduleCode );

    // cpu variables
    int cpuQuantum = configPointer -> quantumCycles;
    int quantumCounter = cpuQuantum;

    // initialize log Nodes. firstLogNode is meant to initialize linked list
    // while logNode is meant to be overwritten like a buffer. logHead is meant
    // to point to the fistLogNode and be the log node linked list head pointer
    logFileType *firstLogNode = NULL;
    logFileType *logNode = NULL;
    logFileType *logHead = NULL;
    // string variable to hold log line contents
    char logString[ 1000 ];
    // make logString equal to null char constant
    logString[0] = NULL_CHAR;

    // timer variables
    double timeDouble;
    char timeStamp[ SMALL_STR_LEN ];

    // boolean flags
    bool toFileFlag = false;
    bool toMonitorFlag = false;
    bool segFaultFlag = false;
    bool preemptionFlag;
    bool quantumTimeOutFlag = false;


    // memory variables
    int maxMemory = getMaxMemory( configPointer );
    int availableMemory = maxMemory;

    // use config file to set memory dislpay flag
    bool memDisplayFlag = configPointer -> memDisplay;

    // variables for keeing count of the number of proceses
    processCount = getProcessCount( metaPointer );

    activeProcesses = processCount;

    // Begin operations

    // check for preemption
    preemptionFlag = checkPreemption( scheduleCode );

    // Check for log to file, display to monitor, or both
    if( configPtr->logToCode == LOGTO_BOTH_CODE )
       {
        toMonitorFlag = true;
        toFileFlag = true;
       }
    else if( configPtr->logToCode == LOGTO_FILE_CODE )
       {
        toFileFlag = true;
       }
    else
       {
        toMonitorFlag = true;
       }

    // allocate memory to log file structs and variables
    if( toFileFlag )
       {
        // allocate memory for log file nodes
        logNode = (logFileType *)malloc(sizeof(logFileType ) );
        firstLogNode = (logFileType *)malloc(sizeof(logFileType));

        // populate first Log Node fields
        copyString( firstLogNode -> logContents, blankString );
        firstLogNode -> nextNode = NULL;

        // make logHead point to fistLogNode, creating linked list.
        logHead = firstLogNode;
       }

    // simulator run

    // display sim run to
      if( toMonitorFlag )
         {
          // print sim run to monitor
          printf("Simulator Run\n");
          printf("-------------\n\n");
         }
      // display simulator is output to log file only
      else
         {
          // print message to let user know sim is writing to file
          printf( "\nSimulator running for output to file only\n" );
         }

    // add log file header to log file linked list
    if( toFileFlag )
       {
         // add header beginning
         sprintf( logString,
                  "\n==================================================\n");
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         sprintf( logString, "Simulator Log File Header\n\n");
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         // add file name
         sprintf( logString,
                  "File Name                       : %s\n",
                  configPointer -> metaDataFileName );
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         // add cpu scheduling algorithm
         sprintf( logString,
                  "CPU Scheduling                  : %s\n",
                  scheduleCode );
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         // add quantum cycles
         sprintf( logString,
                  "Quantum Cycles                  : %d\n",
                  configPointer -> quantumCycles );
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         // add Memory available in kilabytes
         sprintf( logString,
                  "Memory Available (KB)           : %d\n",
                  configPointer -> memAvailable );
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         // add processor cycle rate in milliseconds per cycle
         sprintf( logString,
                  "Processor Cycle Rate (ms/cycle) : %d\n",
                  configPointer -> procCycleRate );
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         // add i/o cycle rate in milliseconds per cycle
         sprintf( logString,
                  "I/O Cycle Rate (ms/cycle)       : %d\n",
                  configPointer -> ioCycleRate );
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;

         // end of header file
         sprintf( logString, "================\nBegin Simulation\n\n");
         copyString( logNode -> logContents, logString );
         addLogNode( logHead, logNode );
         logString[0] = NULL_CHAR;
       } // end log file header conditional

    // assigns start time to time double
    timeDouble = accessTimer( ZERO_TIMER, timeStamp );

    // display to monitor sim start
    if( toMonitorFlag )
       {
          printf("  %10.6f, OS: Simulator start\n", timeDouble);
         }

    // log to file linked List sim start
    if( toFileFlag )
       {
          // move string to logString
          sprintf( logString, "  %10.6f, OS: Simulator start\n", timeDouble );
          copyString( logNode -> logContents, logString );
          // add logString as a node to log file type linked list
          addLogNode( logHead, logNode );
          // clear string
          logString[0] = NULL_CHAR;
         }

    // get the number of processes
    processCount = getProcessCount( metaPointer );

    // loop through meta data to populate pcb list
    pcbHead = populatePCBList( metaPointer, configPointer, processCount );

    // assign pcb loop pointer to pcbHead to loop through each process
    pcbLoopPointer = pcbHead;

    // ready each process in the pcb linked list
    while( pcbLoopPointer != NULL )
       {
        // set pcb node's state to running
        pcbLoopPointer -> pcbState = READY;

        // get current sim time
        timeDouble = accessTimer( LAP_TIMER, timeStamp );

        // display process ready to monitor
        if( toMonitorFlag )
           {
            printf( "  %10.6f, OS: Process %d set to READY state from"
                    " NEW state\n", timeDouble, pcbLoopPointer -> pid );
           }

        // store process ready message to log node
        if( toFileFlag )
        {
         // copy string to logString variable
         sprintf( logString,
                  "  %10.6f, OS: Process %d set to READY state from"
                  " NEW state\n",
                  timeDouble, pcbLoopPointer -> pid );
         copyString( logNode -> logContents, logString );
         // add logString variable as a node to our log file linked list
         addLogNode( logHead, logNode );
         // clear logString variable
         logString[0] = NULL_CHAR;
        }

        // traverse to next node in pcb linked list
        pcbLoopPointer = pcbLoopPointer -> nextNode;
      } // end process ready loop

    // update timer
    timeDouble = accessTimer( LAP_TIMER, timeStamp );

    // memory initialization
    if( toMonitorFlag && memDisplayFlag )
       {
        printf( "--------------------------------------------------\n" );
        printf( "After memory initialization\n" );
        printf( "0 [ Open, p#: x, 0-0 ] %d\n", maxMemory);
        printf( "--------------------------------------------------\n" );
       }

    // set pcb node pointer to the current pcbHead node
    pcbNodePointer = pcbHead;

    // simulator main loop
    while( processesExited < processCount )
       {
        pcbNodePointer = pcbHead;
        // reset segmentation fault flag for new process
        segFaultFlag = false;

        // get next (or first) pcb process
        pcbNodePointer = getNextProcess( pcbNodePointer, scheduleCode );

        if( !quantumTimeOutFlag && pcbNodePointer != NULL )
        {
         // change pcb state to running
         pcbNodePointer -> pcbState = RUNNING;

         // update timer
         timeDouble = accessTimer( LAP_TIMER, timeStamp );

         // display ready status and state change
         if( toMonitorFlag )
            {

            printf( "  %10.6f, OS: Process %d selected with %d ms remaining\n",
                    timeDouble, pcbNodePointer -> pid,
                    pcbNodePointer -> timeRemaining);

            printf( "  %10.6f, OS: Process %d set from READY to RUNNING\n\n",
                    timeDouble, pcbNodePointer -> pid );
           }

         // Add ready status and state change as a log node
         if( toFileFlag )
            {
            // new process selected

            // copy string to logString variable
            sprintf( logString,
                     "  %10.6f, OS: Process %d selected with %d ms remaining\n",
                     timeDouble, pcbNodePointer -> pid,
                     pcbNodePointer-> timeRemaining );
            copyString( logNode -> logContents, logString );
            // add logString variable as a node to our log file linked list
            addLogNode( logHead, logNode );
            // clear logString variable
            logString[0] = NULL_CHAR;

            // Process set from ready to running

            // copy string to logString variable
            sprintf( logString,
                     "  %10.6f, OS: Process %d set from READY to RUNNING\n\n",
                     timeDouble, pcbNodePointer -> pid );
            copyString( logNode -> logContents, logString );
            // add logString variable as a node to our log file linked list
            addLogNode( logHead, logNode );
            // clear logString variable
            logString[0] = NULL_CHAR;
           }
        }
        // reset interupt node pointer
        if( preemptionFlag )
           {
            interruptLoopPointer = interruptHead;
           }

        // cpu simulation with quantum cycles
        while ( quantumCounter > 0
                && ( pcbNodePointer -> pcbState != EXITING )
                && (pcbNodePointer -> pcbState != BLOCKED ) )
           {
            // check for interrupt
            if( preemptionFlag && interruptLoopPointer != NULL )
               {
                // loop through interrupt queue to find a blocked process
                // ready to be released
                while( interruptLoopPointer -> cylcleTimeLeft > 0
                       && interruptLoopPointer -> nextNode != NULL )
                   {
                    interruptLoopPointer = interruptLoopPointer -> nextNode;
                   }
                // if a node in the interrupt queue is marked for removal
                if( interruptLoopPointer -> cylcleTimeLeft < 1 )
                   {
                    // get new time for cpu interrupt
                    timeDouble = accessTimer( LAP_TIMER, timeStamp );

                    if( toMonitorFlag )
                       {
                        printf( "  %10.6f OS: Interrupted by Process %d, %s\n",
                                timeDouble,
                                interruptLoopPointer -> pcbNode -> pid,
                                interruptLoopPointer -> interruptString );
                       }
                    if( toFileFlag )
                       {
                        // copy string to logString variable
                        sprintf( logString,
                                 "  %10.6f OS: Interrupted by Process %d, %s\n",
                                 timeDouble,
                                 interruptLoopPointer -> pcbNode -> pid,
                                 interruptLoopPointer -> interruptString );
                        // copy string to localLogNode member from logString
                        copyString( logNode -> logContents, logString );
                        // add logString variable as a node to our log file
                        // linked list
                        addLogNode( logHead, logNode );
                        // clear logString variable
                        logString[0] = NULL_CHAR;
                       }

                    // set blocked node from BLOCKED to READY
                    interruptLoopPointer -> pcbNode -> pcbState = READY;

                    // get new time for state change
                    timeDouble = accessTimer( LAP_TIMER, timeStamp );

                    if( toMonitorFlag )
                       {
                        printf( "  %10.6f OS: Process %d set from BLOCKED "
                                " to READY\n",
                                timeDouble,
                                interruptLoopPointer -> pcbNode -> pid );
                       }
                    if( toFileFlag )
                       {
                        // copy string to logString variable
                        sprintf( logString,
                                 "  %10.6f OS: Process %d set from"
                                 " BLOCKED to READY\n",
                                 timeDouble,
                                 interruptLoopPointer -> pcbNode -> pid );
                        // copy string to localLogNode member from logString
                        copyString( logNode -> logContents, logString );
                        // add logString variable as a node to our log file
                        // linked list
                        addLogNode( logHead, logNode );
                        // clear logString variable
                        logString[0] = NULL_CHAR;
                       }
                    // remove node from queue
                    dequeueInterrupt( &interruptHead, interruptLoopPointer );
                   }
               }

            // reset interrupt pointer to the head of the queue
            if( preemptionFlag )
               {
                interruptLoopPointer = interruptHead;
               }

            // preemptive algorithms only
            // skip app start and check to see if our current cpu process is done
            if( preemptionFlag )
               {
                // skip  app start and go to the next node
                if( ( compareString( pcbNodePointer -> opCommand -> command,
                                   "app") == STR_EQ )
                      || pcbNodePointer -> opCommand -> intArg2 < 1 )
                   {
                    pcbNodePointer -> opCommand =
                                        pcbNodePointer -> opCommand -> nextNode;
                   }
               }

            // if there is no premption then we don't have to worry about
            // cycles app start gets skipped as well
            else
               {
                pcbNodePointer -> opCommand =
                                        pcbNodePointer -> opCommand -> nextNode;
               }

            // reset quantumTimeOut flag
            quantumTimeOutFlag = false;

            // find app end exit the process state
            if( compareString( pcbNodePointer -> opCommand -> command,
                               "app" ) == STR_EQ
                && compareString( pcbNodePointer -> opCommand -> strArg1,
                                  "end" ) == STR_EQ  )
               {
                // get time for app end
                timeDouble = accessTimer( LAP_TIMER, timeStamp );

                // display app end
                if( toMonitorFlag )
                   {
                    printf( "\n  %10.6f, OS: Process %d ended\n",
                            timeDouble, pcbNodePointer -> pid );
                   }

                // log app end
                if( toFileFlag )
                   {
                    // copy string to logString variable
                    sprintf( logString, "\n  %10.6f, OS: Process %d ended\n",
                             timeDouble, pcbNodePointer -> pid );

                    // copy log strings to logContenets in logNode
                    copyString( logNode -> logContents, logString );

                    // add new log node to linked list
                    addLogNode( logHead, logNode );

                    // clear logString variable
                    logString[0] = NULL_CHAR;
                   }

             // set process to exiting.
             pcbNodePointer -> pcbState = EXITING;
             processesExited++;

             // get time for process exiting
             timeDouble = accessTimer( LAP_TIMER, timeStamp );
            } // end app end conditional

            // all other meta commands
            else
               {
                // extra line to show processes start after quantuming out
                if( quantumTimeOutFlag )
                   {
                    // display to monitor
                    if( toMonitorFlag )
                       {
                        printf( "\n" );
                       }
                    // log to file
                    if( toFileFlag )
                       {
                        // copy string to logString variable
                        sprintf( logString, "\n");
                        // copy string to localLogNode member from logString
                        copyString( logNode -> logContents, logString );
                        // add logString variable as a node to our log file
                        // linked list
                        addLogNode( logHead, logNode );
                        // clear logString variable
                        logString[0] = NULL_CHAR;
                       }
                   }

                // showOpcommand function
                showPCBCommands( pcbNodePointer,
                                 configPointer,
                                 toMonitorFlag,
                                 toFileFlag,
                                 &availableMemory,
                                 &processesExited,
                                 memDisplayFlag,
                                 logHead,
                                 logNode,
                                 preemptionFlag,
                                 &interruptHead );
                // extra line to show process start after quantuming out
                if( quantumTimeOutFlag )
                   {
                    // display to monitor
                    if( toMonitorFlag )
                       {
                        printf( "\n" );
                       }
                    // log to file
                    if( toFileFlag )
                       {
                        // copy string to logString variable
                        sprintf( logString, "\n");
                        // copy string to localLogNode member from logString
                        copyString( logNode -> logContents, logString );
                        // add logString variable as a node to our log file
                        // linked list
                        addLogNode( logHead, logNode );
                        // clear logString variable
                        logString[0] = NULL_CHAR;
                       }
                   }

                // reset quantumTimeOutFlag
                quantumTimeOutFlag = false;

                // in case of segmentation fault
                if( pcbNodePointer -> pcbState == EXITING )
                   {
                    segFaultFlag = true;
                    // get segmentation fault time
                    timeDouble = accessTimer( LAP_TIMER, timeStamp );

                    // display segmentation fault
                    if( toMonitorFlag )
                       {
                        printf( "\n  %10.6f, OS: Segmentation Fault, ",
                                                               timeDouble );
                        printf( "Process %d ended\n", pcbNodePointer -> pid );
                       }

                    // log segmentation fault
                    if( toFileFlag )
                           {
                            // copy string to logString variable
                            sprintf( logString,
                                     "\n  %10.6f, OS: Segmentation Fault, "
                                     "Process %d ended\n",
                                     timeDouble, pcbNodePointer -> pid );

                            // copy logString contents to logNode's logContents
                            copyString( logNode -> logContents, logString );

                            // add new log node to linked list
                            addLogNode( logHead, logNode );

                            // clear logString variable
                            logString[0] = NULL_CHAR;
                           }

                    // get time for process exiting display and logging
                    timeDouble = accessTimer( LAP_TIMER, timeStamp );
                   } // end seg fault condition

               } // end non app end commands conditional
            // exiting process display
            if( toMonitorFlag && pcbNodePointer -> pcbState == EXITING )
               {
                // memory clearing after process Success
                if( memDisplayFlag )
                   {
                    printf( "---------------------------"
                            "-----------------------\n" );

                    printf( "After clear process %d ",
                            pcbNodePointer -> pid );
                    // display process failure
                    if( segFaultFlag )
                       {
                        printf( "failure\n" );
                       }
                    // display process success
                    else
                       {
                        printf( "success\n" );
                       }

                    printf( "0 [ Open, p#: x, 0-0 ] %d\n", maxMemory);
                    printf( "---------------------------"
                            "-----------------------\n" );
                   }

                printf( "  %10.6f, OS: Process %d set to EXIT\n",
                        timeDouble, pcbNodePointer -> pid );
               } // end print process exit

            // interrupt handler section
            // keep track of blocked and exited processes
            if( pcbNodePointer -> pcbState == BLOCKED )
               {
                processesBlocked++;
                activeProcesses--;
               }
            if( pcbNodePointer -> pcbState == EXITING )
               {
                activeProcesses--;
               }
            // cpu idling in preemption algorithms
            if( preemptionFlag && processesBlocked == activeProcesses )
               {
               // get new time for cpu interrupt
               timeDouble = accessTimer( LAP_TIMER, timeStamp );

               // display cpu idling to monitor
               if( toMonitorFlag )
                  {
                   printf( "  %10.6f OS: CPU idle, "
                           "all active processes are blocked\n", timeDouble);
                  }

               // log cpu idling
               if( toFileFlag )
                  {
                   // copy string to logString variable
                   sprintf( logString, "  %10.6f OS: CPU idle, "
                           "all active processes are blocked\n", timeDouble);
                   // copy string to localLogNode member from logString
                   copyString( logNode -> logContents, logString );
                   // add logString variable as a node to our log file
                   // linked list
                   addLogNode( logHead, logNode );
                   // clear logString variable
                   logString[0] = NULL_CHAR;
               }

            // make sure loop pointer is pointing to head
            interruptLoopPointer = interruptHead;

            // decrement blocked processes i/o cycles till a process is
            // unblocked. Simulates cpu idle
            while( processesBlocked == activeProcesses )
               {
                // decrement each io cycle by 1
                while( interruptLoopPointer != NULL
                       && processesBlocked == activeProcesses)
                   {
                    interruptLoopPointer -> cylcleTimeLeft =
                            interruptLoopPointer -> cylcleTimeLeft - 1;

                    if( interruptLoopPointer -> cylcleTimeLeft < 1 )
                       {
                        // decrement blocked cycles to break out of the loop
                        processesBlocked--;
                       }
                    else
                       {
                        // traverse queue
                        interruptLoopPointer = interruptLoopPointer -> nextNode;
                       }
                   }
                // reset queue
                if( processesBlocked == activeProcesses)
                   {
                    interruptLoopPointer = interruptHead;
                   }
               }

            // end cpu idling
            // get new time for cpu interrupt
            timeDouble = accessTimer( LAP_TIMER, timeStamp );

            if( toMonitorFlag )
               {
                printf( "  %10.6f OS: Interrupted by Process %d, %s\n",
                        timeDouble,
                        interruptLoopPointer -> pcbNode -> pid,
                        interruptLoopPointer -> interruptString );
               }
             if( toFileFlag )
                {
                 // copy string to logString variable
                 sprintf( logString, "  %10.6f OS: Interrupted "
                          "by Process %d, %s\n",
                          timeDouble,
                          interruptLoopPointer -> pcbNode -> pid,
                          interruptLoopPointer -> interruptString );
                 // copy string to localLogNode member from logString
                 copyString( logNode -> logContents, logString );
                 // add logString variable as a node to our log file
                 // linked list
                 addLogNode( logHead, logNode );
                 // clear logString variable
                 logString[0] = NULL_CHAR;
                }

             // set blocked node from BLOCKED to READY
             interruptLoopPointer -> pcbNode -> pcbState = READY;

             // get new time for state change
             timeDouble = accessTimer( LAP_TIMER, timeStamp );

             // display blocked to ready state change to monitor
             if( toMonitorFlag )
                {
                 printf( "  %10.6f OS: Process %d set from BLOCKED to READY\n",
                         timeDouble, interruptLoopPointer -> pcbNode -> pid );
                }
              // log blocked to ready state change
              if( toFileFlag )
                 {
                  // copy string to logString variable
                  sprintf( logString, "  %10.6f OS: Process %d set from "
                           "BLOCKED to READY\n",
                           timeDouble,
                           interruptLoopPointer -> pcbNode -> pid );
                  // copy string to localLogNode member from logString
                  copyString( logNode -> logContents, logString );
                  // add logString variable as a node to our log file
                  // linked list
                  addLogNode( logHead, logNode );
                  // clear logString variable
                  logString[0] = NULL_CHAR;
                 }

            // remove node from queue
            dequeueInterrupt( &interruptHead, interruptLoopPointer );
           }
            // if cpu isn't idling decrement io cycles in blocked queue to
            // simulate concurency
            // on next cpu cycle the cpu will check for an interrupt via
            // looping through the blocked que.
            else if( preemptionFlag )
               {
                while( interruptLoopPointer != NULL)
                   {
                    // decrement one cycle from the queue
                    interruptLoopPointer -> cylcleTimeLeft =
                                     interruptLoopPointer -> cylcleTimeLeft - 1;

                    // traverse blocked queue
                    interruptLoopPointer = interruptLoopPointer -> nextNode;
                   }
               }
            // Reduce Quantum cycle
            quantumCounter--;
           } // end cpu loop
        // update timer for quantum time out message
        timeDouble = accessTimer( LAP_TIMER, timeStamp );

        // Quantum time out with preemptive schedule codes
        if( preemptionFlag )
           {
            if ( pcbNodePointer -> pcbState != EXITING )
               {
                // display quantum time out to monitor
                if( toMonitorFlag )
                   {

                    printf( "\n  %10.6f, OS: Process %d quantum time out, "
                            "cpu process operation end\n\n",
                            timeDouble, pcbNodePointer -> pid );
                   }

                // log quantum time out
                if( toFileFlag )
                   {
                    // copy string to logString variable
                    sprintf( logString, "\n  %10.6f, OS: Process %d quantum "
                             "time out, cpu process operation end\n\n",
                             timeDouble,
                             pcbNodePointer -> pid );
                    // copy string to localLogNode member from logString
                    copyString( logNode -> logContents, logString );
                    // add logString variable as a node to our log file
                    // linked list
                    addLogNode( logHead, logNode );
                    // clear logString variable
                    logString[0] = NULL_CHAR;
                   }
               }
           }
        // reset quantum counter if it reaches 0, otherwise let the next
        // process use up the rest of the quantum cycles
        if( quantumCounter == 0 )
           {
             quantumCounter = cpuQuantum;

             if( pcbNodePointer -> pcbState != EXITING )
                {
                  quantumTimeOutFlag = true;
                }
           }
       } // end OS master loop

    // point to pcbHead for easy freeing
    pcbNodePointer = pcbHead;
    pcbLoopPointer = pcbHead;

    // free pcb linked list pointers
    pcbHead = clearPCBList( pcbHead );

    // get sim end time
    timeDouble = accessTimer( STOP_TIMER, timeStamp );

    // end simulation

    // display end simulation message to monitor
    if ( toMonitorFlag )
       {
        // mem Display On; display closing message to monitor
        if( memDisplayFlag )
           {
            printf( "--------------------------------------------------\n" );
            printf("After clear ");
            // celeberate no seg faults
            if( segFaults == 0 )
               {
                printf( "all process success\n" );
               }
            // celeberate completed processes
            else
               {
                clearedProcesses = processCount - segFaults;
                printf( "%d processes successful\n", clearedProcesses );
               }

            // last mem display message
            printf( "No memory configured\n" );
            printf( "--------------------------------------------------\n" );
           }
        // displays last message
        printf( "  %10.6f, OS: Simulation end\n", timeDouble );
       }

    // add end simulation message to log linked list
    // write to log file and free linked list
    if ( toFileFlag )
       {
        // copy string to logString variable
        sprintf( logString,
                 "  %10.6f, OS: Simulation end\n", timeDouble );
        copyString( logNode -> logContents, logString );
        // add logString variable as a node to our log file linked list
        addLogNode( logHead, logNode );
        // clear logString variable
        logString[0] = NULL_CHAR;

        // write all log file linked list contents to the log file
        writeToLogFile( configPtr, logHead );

        // free log Node buffer
        free( logNode );

        // free log File linked list memory
        logHead = clearLogFileList( logHead );
       }

    // void function, no return
   }

// attempts to acces memory, returns success or failure
bool accessMemory( int memoryBase, int memoryOffset, int *availableMemory )
   {
    int memoryLocation = memoryBase + memoryOffset;

    // checks if the memory location is valid
    if( memoryLocation > *availableMemory )
       {
        // return access memory failure
        return false;
       }

    // return accessMemory success
    return true;
   }

// adds new log node to log file linked list
logFileType *addLogNode( logFileType *headPointer, logFileType *newNode )
   {
    // if current node is empty
    if( headPointer == NULL )
       {
        // allocate memory for new node
        headPointer = (logFileType *)malloc(sizeof( logFileType ));

        // populate node fields
        copyString( headPointer->logContents, newNode -> logContents );
        headPointer -> nextNode = NULL;

        // return pointer
        return headPointer;
       }

    // traverse through linked list
    headPointer -> nextNode = addLogNode( headPointer -> nextNode, newNode );

    // return node
    return headPointer;
   }

// adds new process node to PCB linked list
pcbType *addProcess( int processNumber, OpCodeType *metaPointer,
                     pcbType *currentNode, ConfigDataType *configPointer )
   {
    // initialize local variables
    OpCodeType *localMetaPointer = metaPointer;
    pcbType *localHeadNode = currentNode;
    ConfigDataType *localConfigPointer = configPointer;

    // check if localPCBPointer is null
    if( localHeadNode == NULL )
       {
        // allocate memory for currentNode
        localHeadNode = (pcbType *) malloc( sizeof( pcbType ) );

        // populate currentNode fields with newNode fields

        // pid is the current process number parameter
        localHeadNode->pid = processNumber;
        // set state to NEW since the processes has just been added
        localHeadNode->pcbState = NEW;
        // assign current opCommand from
        localHeadNode->opCommand = localMetaPointer;
        // get the time remaining for the current process from meta data
        localHeadNode->timeRemaining = getPCBTime( localMetaPointer,
                                                 localConfigPointer );
        // Set next node to null since list is empty
        localHeadNode->nextNode = NULL;

        localHeadNode->priority = 0;

        // return current Node in list
        return localHeadNode;
       }

    // the local head node is not NULL, so we try adding our local head node
    // to our next node reference recursively
    localHeadNode -> nextNode = addProcess( processNumber,
                                            localMetaPointer,
                                            localHeadNode -> nextNode,
                                            localConfigPointer );

     // return current node after breaking out of recursive calls
     return localHeadNode;
   }

// attempts to allocate memory, returns failure or success
bool allocationAttempt( int memoryBase,
                        int memoryOffset,
                        int *availableMemory )
   {
    int totalMemoryRequested = memoryBase + memoryOffset;

    // check if memory is greater than whats available
    if( totalMemoryRequested > *availableMemory )
       {
        // return attempt failure
        return false;
       }

    // return postitive success
    return true;
   }

// checks if the schedule code is preemptive
bool checkPreemption( char *scheduleCode )
   {
    // check for preemptive algorithms
    if( compareString(scheduleCode, "FCFS-P") == STR_EQ )
       {
        return true;
       }
    else if( compareString(scheduleCode, "SRTF-P") == STR_EQ )
       {
        return true;
       }
    else if( compareString(scheduleCode, "RR-P") == STR_EQ )
       {
        return true;
       }

    // return false, no preemptive algorithms
    return false;
   }

interruptType *clearInterruptQueue( interruptType *headNode )
   {
    // if linked list is not empty
    if( headNode != NULL )
       {
        // recursively clear next pointer
        clearInterruptQueue( headNode -> nextNode );

        // free current node after recursive function call headNode isn't null
        if( headNode != NULL )
           {
            free( headNode );
           }

        // set pointer to NULL
        headNode = NULL;
       }

    // return null for empty linked list
    return NULL;
   }

// frees allocated log linked list memory
logFileType *clearLogFileList( logFileType *logFileListPointer )
   {
    // if linked list is not empty
    if( logFileListPointer != NULL )
       {
        // recursively clear next pointer
        clearLogFileList( logFileListPointer -> nextNode );

        // free current node after recursive function call
        if( logFileListPointer != NULL)
           {
            free( logFileListPointer );
           }

        // set node to NULL
        logFileListPointer = NULL;
       }

    // return NULL when list is clear
    return NULL;
   }

// frees allocated PCB linked list memory
pcbType *clearPCBList( pcbType *pcbPointer )
   {
    //check to see if pcb pointer is NULL
    if( pcbPointer != NULL )
       {
        // recursive function to next pointer
        clearPCBList( pcbPointer->nextNode );

        // after traversal/recursion free node
        free( pcbPointer );

        // set node to NULL
        pcbPointer = NULL;
       }
    // return NULL because list is cleared now
    return NULL;
   }

void dequeueInterrupt( interruptType **interruptHead,
                       interruptType *interruptNode )
   {
    interruptType *localHead = *interruptHead;

    // search for local head position
    while( &localHead != interruptHead )
       {
        localHead = localHead -> nextNode;
       }

    // no previous node
    if( localHead -> prevNode == NULL)
       {
        // this node is the only node in the list
        if( localHead -> nextNode == NULL)
           {
            free( interruptNode );

            interruptNode = NULL;
           }
        // this node is the head node linked to another node
        else
           {
            // assign the next node as the new headNode of the linked list
            localHead = interruptNode -> nextNode;

            // break the link
            interruptNode -> nextNode -> prevNode = NULL;

            // free node memory
            free( interruptNode );

            // set node to NULL
            interruptNode = NULL;
           }

       }

    // this node is at the end of the list
    else if( localHead -> nextNode == NULL )
       {
        // remove previous node link
        interruptNode -> prevNode -> nextNode = NULL;

        // free memory
        free( interruptNode );

        // set interruptNode to null;
        interruptNode = NULL;
       }
    // connnect previous and next nodes
    else
       {
        // connnect previous to next node
        interruptNode -> prevNode -> nextNode = interruptNode -> nextNode;

        // connect next node to previous
        interruptNode -> nextNode -> prevNode = interruptNode -> prevNode;

        // free memory
        free( interruptNode );

        // set node to null
        interruptNode = NULL;
       }
    // void function, no return
   }

// finds next FCFS-N process for execution
pcbType *getFCFSNNode( pcbType *pcbHead )
   {
    // current pcb node is empty
    if( pcbHead == NULL )
       {
        return NULL;
       }

    // if current process is exiting, go to the next process
    if( pcbHead -> pcbState == EXITING)
       {
        return getFCFSNNode( pcbHead -> nextNode );
       }

    // return current pcbHead, because it's not exiting or null
    return pcbHead;
   }

// finds next FCFS-P processes for execution
pcbType *getFCFSPNode( pcbType *pcbHead )
   {
    // current pcb node is empty
    if( pcbHead == NULL )
       {
        return NULL;
       }

    // if current processes is exiting OR blocked, go to the next processes
    if( pcbHead -> pcbState == EXITING || pcbHead -> pcbState == BLOCKED )
       {
        return getFCFSPNode( pcbHead -> nextNode );
       }

    // return current pcbHead, because it's not blocked or exiting
    return pcbHead;
   }

pcbType *getRRPNode( pcbType *pcbHead )
   {
     //local variable
     pcbType *pcbLooper = pcbHead;
     int priority;

     // current pcb node is empty
     if( pcbHead == NULL )
        {
         return NULL;
        }

     // get lowest priority
     while( pcbLooper != NULL && pcbLooper -> pcbState != BLOCKED )
        {
          if( pcbLooper-> pid == 0 )
             {
              priority = pcbLooper -> priority;
             }
          else if( pcbLooper -> priority < priority )
             {
              priority = pcbLooper -> priority;
             }
         pcbLooper = pcbLooper -> nextNode;
        }

     // if current processes is exiting OR blocked, go to the next processes
     if( pcbHead -> pcbState == EXITING || pcbHead -> pcbState == BLOCKED )
        {
         if(pcbHead -> priority == priority )

         return getFCFSPNode( pcbHead -> nextNode );
        }

     // return current pcbHead, because it's not blocked or exiting
     return pcbHead;
   }

// gives back max memory from config file. Memory has to be less than the
// specified max or the max will be returned
int getMaxMemory( ConfigDataType *configPointer)
   {
    // specified max memory
    int memoryMax = 102400;
    int memoryRequested = configPointer -> memAvailable;

    // check to see if memory request is too large
    // return memoryMax
    if( memoryRequested > memoryMax )
       {
        return memoryMax;
       }

    return memoryRequested;
   }

// picks next process for execution based on scheduling code
pcbType *getNextProcess( pcbType *pcbHead, char *scheduleCode )
   {
    // FCFS-P scheduling code
    if( compareString( scheduleCode, "FCFS-P" ) == STR_EQ )
       {
        return getFCFSPNode( pcbHead );
       }

    // SJF-N scheduling code
    if( compareString(scheduleCode, "SJF-N" ) == STR_EQ )
       {
        // the false parameter tells our function there is no preemption
        // getShortestJobAlgoNode() will perform SJF-N scheduling
        return getShortAlgoNode( pcbHead, false );
       }

    // SRTF-P scheduling code
    if( compareString(scheduleCode, "SRTF-P" ) == STR_EQ )
       {
        // the true parameter tells our function there is preemption
        // getShortestJobAlgoNode() will perform SRTF-P scheduling
        return getShortAlgoNode( pcbHead, true );
       }

    // RR-P scheduling code
    // FCFS-P scheduling code
    if( compareString( scheduleCode, "RR-P" ) == STR_EQ )
       {
        return getRRPNode( pcbHead );
       }


    // FCFS-N by default if not SJF-N, FCFS-P, SRTF-P, or RR-P
    return getFCFSNNode( pcbHead );
   }

// find the a process's time remaining
int getPCBTime( OpCodeType *metaPointer, ConfigDataType *configPointer )
   {
    // initiallize variables
    int sum = 0;
    OpCodeType *localMetaPointer = metaPointer;
    ConfigDataType *localConfigPointer = configPointer;

    // loop through process to add up time
    while( localMetaPointer != NULL )
       {
        // stops counting up the time at app end
        if( compareString( localMetaPointer -> command, "app" ) == STR_EQ
            && compareString( localMetaPointer -> command, "end" ) == STR_EQ )
           {
            return sum;
           }

        // CPU cycles
        // multiply process time by processor cycle rate
        if( compareString( localMetaPointer -> command, "cpu" ) == STR_EQ )
           {
            sum += ( localMetaPointer -> intArg2 *
                     localConfigPointer -> procCycleRate );
           }
        // io cyclces
        // mulitply i/o time by i/o cycle rate
        if( compareString( localMetaPointer -> command, "dev" ) == STR_EQ )
           {
            sum += ( localMetaPointer -> intArg2 *
                     localConfigPointer -> ioCycleRate );
           }

        // traverse the array
        localMetaPointer = localMetaPointer -> nextNode;
       } // end of looping through process

    // return sum for timeRemaining
    return sum;
   }

// count meta data "app start" to get the number of processes
int getProcessCount( OpCodeType *metaPointer )
   {
    int appStartCounter = 0;

    // loop through all meta data
    while( compareString( metaPointer->command, "sys" ) != STR_EQ ||
           compareString( metaPointer->strArg1, "end" ) != STR_EQ )
       {
        // check for app start
        if( compareString( metaPointer->command, "app" ) == STR_EQ &&
            compareString( metaPointer->strArg1, "start" ) == STR_EQ )
           {
            // increment appStartCounter
            appStartCounter++;
           }

        // Move through linked list to next Node
        metaPointer = metaPointer->nextNode;
       }

    // return process number
    return appStartCounter;
   }

// returns process time by checking meta data cycle rates
int getProcessTime( ConfigDataType *configPointer, OpCodeType *metaPointer )
   {
    // initialize variable sum to 0
    int sum = 0;

    // I/O cycle
    if( compareString( metaPointer -> command, "dev" ) == STR_EQ )
       {
        sum = configPointer -> ioCycleRate * metaPointer -> intArg2 ;
       }

    // CPU cycle
    if( compareString( metaPointer -> command, "cpu" ) == STR_EQ )
       {
        sum = configPointer -> procCycleRate * metaPointer -> intArg2;
       }

    return sum;
   }

int getShortestJob( pcbType *pcbHead, bool preempted )
   {
    // holds shortest time
    double shortestTime;
    // holds pid of the shortest job. Initialized as an INVALID_INDEX in case
    // there is no shortest job left
    int minPID = INVALID_INDEX;

    // loop through pcb linked List. Avoid null and exiting nodes.
    while( pcbHead != NULL && pcbHead -> pcbState != EXITING )
       {
        // if preemption flag is true, check if the current node is BLOCKED
        // if it is blocked, move on to the next process
        if( preempted && pcbHead -> pcbState == BLOCKED )
           {
            // Current Node is blocked, move on to next node
            pcbHead = pcbHead -> nextNode;
           }
        // make the first process the shortest job by default
        else if( minPID == INVALID_INDEX )
           {
            shortestTime = pcbHead -> timeRemaining;
            minPID = pcbHead -> pid;
            // traverse linked list
            pcbHead = pcbHead -> nextNode;
           }
        // Current Node process has the same time or greater than the
        // current minimum job, we'll traverse over it.
        else if( shortestTime <= pcbHead -> timeRemaining )
           {
            // traverse linked list
            pcbHead = pcbHead -> nextNode;
           }
        // pid hasn't been used so update shortestTime and minPID variables to
        // current pcbHead node's timeRemaining and pid.
        else
           {
            // update min values
            shortestTime  = pcbHead -> timeRemaining;
            minPID = pcbHead -> pid;

            // traverse linked List
            pcbHead = pcbHead -> nextNode;
           }
       } // end loop through pcb linked list

    // return min process pid
    return minPID;
   }


// used for SJF-P when preempted is false and does SRTF-P when preempted is true
// returns next node for each algorithm
pcbType *getShortAlgoNode( pcbType *pcbHead, bool preempted )
   {
    // local variable
    int shortestJobIndex;

    // return localHead if pcbHead is empty
    if( pcbHead == NULL )
       {
        return pcbHead;
       }

    // find shortest job pid
    shortestJobIndex = getShortestJob( pcbHead, preempted );

    // check for valid index returned for shortest job Node
    if( shortestJobIndex != INVALID_INDEX)
       {
        // traverse through pcb node linked list to shortest job index with pid
        while( pcbHead != NULL )
          {
           // check if shortest job index matches a pcb of a process
           if( shortestJobIndex == pcbHead -> pid )
              {
               // shorstest job found, return localHead process
               return pcbHead;
              }

           // traverse to the next pcb Node in linked list
           pcbHead = pcbHead -> nextNode;
          } // end while loop searching for matching pid
       } // end valid index conditional

    // return null for empty pcbNode list
    return NULL;
   }


void *handleThreadIO( void *timeParameter )
   {
    // initializing runtime by converting timeParameter (milliseconds)
    // into an int
    int ioTime =  * (int *) timeParameter;

    // run timer with runtime arg
    runTimer(ioTime);

    // Kill thread
    pthread_exit(NULL);
   }

pcbType *populatePCBList( OpCodeType *metaPointer,
                          ConfigDataType *configPointer,
                          int processCount )
   {
    // initialize variables

    // starts at 0 processes (process ID)
    int processNumber = 0;

    // pcbType Node pointers
    pcbType *headNode = NULL;

    // go through all meta data

    // loop goes while we don't go over the number of processes we have
    while( processNumber < processCount )
       {
        // check for app start to add new pcb process node
        if( compareString( metaPointer->command, "app" ) == STR_EQ &&
            compareString( metaPointer->strArg1, "start" ) == STR_EQ )
           {
            // add new pcb node
            headNode = addProcess( processNumber, metaPointer,
                                       headNode, configPointer );

            // increment processNumber for next process
            processNumber++;
           }

        // Move through the meta data list to the next node
        metaPointer = metaPointer->nextNode;
       }

    // return head node of the PCB linked list
    return headNode;
   }

void queueInterruptNode( interruptType **interruptHead,
                         pcbType *pcbNode,
                         char *interruptString,
                         int cyclesLeft )
   {

    // local variables
    interruptType *localHead = *interruptHead;

    // allocate memory for new node
    interruptType *newNode = (interruptType *)malloc(sizeof( interruptType ));


    // populate node fields, except for prevNode
    newNode -> interruptBool = false;
    newNode -> pcbNode = pcbNode;
    copyString( newNode -> interruptString, interruptString );
    newNode -> cylcleTimeLeft = cyclesLeft;
    newNode -> nextNode = NULL;

    // check if list is completely empty and make this node head of the list
    if( *interruptHead == NULL )
       {
        newNode -> prevNode = NULL;
        // assign current node as the head node of the linked list
        *interruptHead = newNode;
       }
    else
       {
        // find last node in list
        while( localHead -> nextNode != NULL )
           {
            localHead = localHead -> nextNode;
           }
        // make the new node last in the list
        localHead -> nextNode = newNode;
        // make localHead newNode's prevNode
        newNode -> prevNode = localHead;
       }
    // void function, no return
   }

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
                     interruptType **interruptHead )
   {
    // initialize local variables
    pcbType *localNode = headNode;
    ConfigDataType *localConfigPointer = configPointer;
    int *localAvailableMemory = availableMemory;
    int maxMemory = *availableMemory;
    logFileType *localLogHead = logHead;
    logFileType *localLogNode = logNode;
    char logString[ 30000 ];
    logString[0] = NULL_CHAR;

    // pthread ID  for thread operations
    pthread_t pthreadID;

    // cpu values for time management
    int cpuTime;
    int cpuCycleRate;
    int pcbCommandTime;


    int ioTime;
    char ioString[ STD_STR_LEN ];

    // memory variables
    int currentMemoryAvailable = *localAvailableMemory;
    int memoryUsed = 0;
    int memRequestBase = 0;
    int memLocationEnd = 0;
    int memRequestOffset = 0;
    // used for testing allocation and access requests
    bool memFlag;

    // timer local variables
    char localTimeStamp[ MAX_STR_LEN ];
    double localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

    // display to monitor the beginning of the process message
    if( toMonitorFlag )
       {
        printf( "  %10.6f, Process: %d, ",
                localTimeDouble, localNode -> pid );
       }

    // log to file the beginning of the process message
    if( toFileFlag )
       {
        // copy string to logString variable
        sprintf( logString,
                 "  %10.6f, Process: %d, ", localTimeDouble, localNode -> pid );
        // copy string to localLogNode member from logString
        copyString( localLogNode -> logContents, logString );
        // add logString variable as a node to our log file linked list
        addLogNode( localLogHead, localLogNode );
        // clear logString variable
        logString[0] = NULL_CHAR;
       }

    // check for cpu command
    if( compareString( localNode -> opCommand -> command,
                       "cpu") == STR_EQ )
       {
        // print to monitor cpu beginning
        if( toMonitorFlag )
           {
            printf( "cpu ");
           }

        // log cpu beggining
        if( toFileFlag )
           {
             // copy string to logString variable
             sprintf( logString, "cpu " );
             // copy string to localLogNode member from logString
             copyString( localLogNode -> logContents, logString );
             // add logString variable as a node to our log file linked list
             addLogNode( localLogHead, localLogNode );
             // clear logString variable
             logString[0] = NULL_CHAR;
           }

        // process continue
        if( compareString( localNode -> opCommand -> strArg1,
                           "process" ) == STR_EQ )
           {
            // display to monitor process start
            if( toMonitorFlag )
               {
                printf( "process operation start\n" );
               }

            // log to monitor process start
            if( toFileFlag )
               {
                 // copy string to logString variable
                 sprintf( logString, "process operation start\n" );
                 // copy string to localLogNode member from logString
                 copyString( localLogNode -> logContents, logString );
                 // add logString variable as a node to our log file linked list
                 addLogNode( localLogHead, localLogNode );
                 // clear logString variable
                 logString[0] = NULL_CHAR;
               }

            // assign values to multiply together for cpuTime
            pcbCommandTime = localNode -> opCommand -> intArg2;
            cpuCycleRate = localConfigPointer -> cpuSchedCycles;

            // get cpu time
            cpuTime = pcbCommandTime * cpuCycleRate;

            // pthread operation
            pthread_create( &pthreadID, NULL,
                            handleThreadIO, (void *)&(cpuTime) );
            pthread_join( pthreadID, NULL );

            // reduce time cpuTime from node time remaining
            localNode -> timeRemaining = localNode -> timeRemaining - cpuTime;

            // if preemptive algorithm, reduce op command cpu quantum
            if( preemptionFlag )
               {
                localNode -> opCommand -> intArg2 =
                                          localNode -> opCommand -> intArg2 - 1;
               }

            // get cpu process end time
            localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

            // shows process if cpu process finishes before next quantum
            if( !preemptionFlag ||
                (preemptionFlag && localNode -> opCommand -> intArg2 == 0 ) )
               {
                // display process end to monitor
                if( toMonitorFlag )
                   {
                    printf("  %10.6f, Process: %d, cpu processes end\n",
                              localTimeDouble, localNode -> pid );
                   }
                // log process end
                if( toFileFlag )
                   {
                     // copy string to logString variable
                     sprintf( logString,
                              "  %10.6f, Process: %d, cpu process end\n",
                              localTimeDouble, localNode -> pid );
                     // copy string to localLogNode member from logString
                     copyString( localLogNode -> logContents, logString );
                     // add logString variable as a node to our log file
                     // linked list
                     addLogNode( localLogHead, localLogNode );
                     // clear logString variable
                     logString[0] = NULL_CHAR;
                   }
               } // end non preemption and premption before quantum conditional

           } // ends cpu "process" check
       }// end cpu command check

    // memory operations
    else if ( compareString( localNode -> opCommand -> command,
                              "mem" ) == STR_EQ )
       {
        // check for allocate memory command
        if( compareString( localNode -> opCommand -> strArg1,
                           "allocate" ) == STR_EQ )
           {
            // assign value to mem request max and min
            memRequestBase = localNode -> opCommand -> intArg2;
            memRequestOffset = localNode -> opCommand -> intArg3;

            // get time for memory allocate attempt
            localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

            // display to monitor mem allocate request
            if( toMonitorFlag )
               {
                printf( "mem allocate request ( %d, %d)\n",
                        memRequestBase, memRequestOffset );
               }

            // logs mem allocate request
            if( toFileFlag )
               {
                 // copy string to logString variable
                 sprintf( logString,
                          "mem allocate request ( %d, %d )\n",
                          memRequestBase, memRequestOffset );
                 // copy string to localLogNode member from logString
                 copyString( localLogNode -> logContents, logString );
                 // add logString variable as a node to our log file linked list
                 addLogNode( localLogHead, localLogNode );
                 // clear logString variable
                 logString[0] = NULL_CHAR;
               }

            // try to allocate memory
            memFlag = allocationAttempt( memRequestBase,
                                                memRequestOffset,
                                                localAvailableMemory );

            // get time after memory allocation attempt
            localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

            // allocation success
            if( memFlag )
               {
                // display successful mem allocation request to monitor
                if( toMonitorFlag )
                   {
                    printf( "  %10.6f, Process: %d, ",
                            localTimeDouble, localNode -> pid );
                    printf( "succesful mem allocate request ( %d, %d)\n",
                            memRequestBase, memRequestOffset );

                    // second half of range
                    memLocationEnd = memRequestBase + memRequestOffset;
                    // memory that has been allocated
                    memoryUsed = memLocationEnd - memRequestBase;
                    // memory available after process mem allocation
                    currentMemoryAvailable -= memoryUsed;

                    if( memDisplayFlag )
                       {
                        printf( "--------------------------"
                                "------------------------\n" );

                        printf( "After allocation success\n" );

                        printf( "0 [ Used, P#: %d, %d-%d ] %d\n",
                                localNode -> pid,
                                memRequestBase,
                                memLocationEnd,
                                memoryUsed );
                        printf( "%d [ Open, P#: x, 0-0 ] %d\n",
                                currentMemoryAvailable,
                                maxMemory );
                        printf( "-------------------------"
                                "-------------------------\n" );
                       } // end mem display on conditional
                   } // end display to monitor conditional

                // log successful mem allocation request to monitor
                if( toFileFlag )
                   {
                     // copy string to logString variable
                     sprintf( logString,
                              "  %10.6f, Process: %d, "
                              "successful mem allocate request ( %d, %d )\n",
                              localTimeDouble, localNode -> pid,
                              memRequestBase, memRequestOffset );
                     // copy string to localLogNode member from logString
                     copyString( localLogNode -> logContents, logString );
                     // add logString variable as a node to our log file
                     // linked list
                     addLogNode( localLogHead, localLogNode );
                     // clear logString variable
                     logString[0] = NULL_CHAR;
                   } // end successful mem allocate request logging

               } // end allocation success conditional

            // allocation failure
            else
               {
                // set process state to exiting
                localNode -> pcbState = EXITING;
                processesExited++;


                // display to monitor failed mem allocation request
                if( toMonitorFlag )
                   {
                     printf( "  %10.6f, Process: %d, ",
                             localTimeDouble, localNode -> pid );
                     printf( "failed mem allocate request\n" );

                     // second half of range
                     memLocationEnd = memRequestBase + memRequestOffset;
                     // memory that has been allocated
                     memoryUsed = memLocationEnd - memRequestBase;
                     // memory available after process mem allocation
                     currentMemoryAvailable -= memoryUsed;

                     // display access failure to monitor
                     if( memDisplayFlag )
                        {
                          printf( "--------------------------"
                                  "------------------------\n" );

                          printf( "After access failure\n" );

                          printf( "0 [ Used, P#: %d, %d-%d ] %d\n",
                                  localNode -> pid,
                                  memRequestBase,
                                  memLocationEnd,
                                  memoryUsed );
                          printf( "%d [ Open, P#: x, 0-0 ] %d\n",
                                  currentMemoryAvailable,
                                  maxMemory );
                          printf( "-------------------------"
                                  "-------------------------\n" );
                        } // end mem failure display to monitor

                   } // end display mem access failure to monitor

                // log failed mem allocation request
                if( toFileFlag )
                   {
                     // copy string to logString variable
                     sprintf( logString,
                              "  %10.6f, Process: %d, "
                              "failed mem allocate request\n",
                              localTimeDouble, localNode -> pid );
                     // copy string to localLogNode member from logString
                     copyString( localLogNode -> logContents, logString );
                     // add logString variable as a node to our log file linked list
                     addLogNode( localLogHead, localLogNode );
                     // clear logString variable
                     logString[0] = NULL_CHAR;
                   } // end log failed mem allocation request

               } // end failed mem allocation conditional

           } // end mem allocation request conditional

        // command is memory access
        else
           {
            // assign value to mem request max and min
            memRequestBase = localNode -> opCommand -> intArg2;
            memRequestOffset = localNode -> opCommand -> intArg3;

            // get time for memory access attempt
            localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

            // display allocation request message to monitor
            if( toMonitorFlag )
               {
                printf( "mem access request ( %d, %d)\n",
                        memRequestBase, memRequestOffset );
               }

            // log allocation request message
            if( toFileFlag )
               {
                 // copy string to logString variable
                 sprintf( logString, "mem access request ( %d, %d)\n",
                          memRequestBase, memRequestOffset );
                 // copy string to localLogNode member from logString
                 copyString( localLogNode -> logContents, logString );
                 // add logString variable as a node to our log file linked list
                 addLogNode( localLogHead, localLogNode );
                 // clear logString variable
                 logString[0] = NULL_CHAR;
               }

            // try to allocate memory
            memFlag = accessMemory( memRequestBase,
                                     memRequestOffset,
                                     localAvailableMemory );

            // get time after memory allocation attempt
            localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

            // allocation success
            if( memFlag )
               {
                // display succesful mem access request to monitor
                if( toMonitorFlag )
                   {
                    printf( "  %10.6f, Process: %d, ",
                            localTimeDouble, localNode -> pid );
                    printf( "succesful mem access request ( %d, %d)\n",
                            memRequestBase, memRequestOffset );

                    // mem display set to On; show mem allocation
                    if( memDisplayFlag )
                       {
                        printf( "--------------------------"
                                "------------------------\n" );

                        printf( "After access success\n" );

                        printf( "0 [ Used, P#: %d, %d-%d ] %d\n",
                                localNode -> pid,
                                memRequestBase,
                                memLocationEnd,
                                memoryUsed );
                        printf( "%d [ Open, P#: x, 0-0 ] %d\n",
                                currentMemoryAvailable,
                                maxMemory );
                        printf( "-------------------------"
                                "-------------------------\n" );
                       } // end mem display to monitor
                   } // end display allocation success to monitor

                // log succesful mem access request
                if( toFileFlag )
                   {
                    // copy string to logString variable
                    sprintf( logString,
                             "  %10.6f, Process: %d, "
                             "succesful mem access request ( %d, %d)\n",
                             localTimeDouble, localNode -> pid,
                             memRequestBase, memRequestOffset );
                    // copy string to localLogNode member from logString
                    copyString( localLogNode -> logContents, logString );
                    // add logString variable as a node to our log file linked list
                    addLogNode( localLogHead, localLogNode );
                    // clear logString variable
                    logString[0] = NULL_CHAR;
                   } // end log succesful mem acces request

               } // end allocation success conditional

            // allocation failure
            else
               {
                // set process state to exiting
                localNode -> pcbState = EXITING;
                processesExited++;

                // display allocation to monitor
                if( toMonitorFlag )
                   {
                    printf( "  %10.6f, Process: %d, ",
                            localTimeDouble, localNode -> pid );
                    printf( "failed mem access request\n" );

                    // second half of range
                    memLocationEnd = memRequestBase + memRequestOffset;
                    // memory that has been allocated
                    memoryUsed = memLocationEnd - memRequestBase;
                    // memory available after process mem allocation
                    currentMemoryAvailable -= memoryUsed;

                    // mem display set to On
                    if( memDisplayFlag )
                       {
                         printf( "--------------------------"
                                 "------------------------\n" );

                         printf( "After allocate failure\n" );

                         printf( "0 [ Used, P#: %d, %d-%d ] %d\n",
                                 localNode -> pid,
                                 memRequestBase,
                                 memLocationEnd,
                                 memoryUsed );
                         printf( "%d [ Open, P#: x, 0-0 ] %d\n",
                                 currentMemoryAvailable,
                                 maxMemory );
                         printf( "-------------------------"
                                 "-------------------------\n" );
                       } // end mem display On conditional
                   } // end display allocation failure to monitor conditional

                // log allocation failure
                if( toFileFlag )
                   {
                    // copy string to logString variable
                    sprintf( logString,
                             "  %10.6f, Process: %d, "
                             "failed mem access request\n",
                             localTimeDouble, localNode -> pid );
                    // copy string to localLogNode member from logString
                    copyString( localLogNode -> logContents, logString );
                    // add logString variable as a node to our log file linked list
                    addLogNode( localLogHead, localLogNode );
                    // clear logString variable
                    logString[0] = NULL_CHAR;
                   } // end log allocation request

               } // allocattion failure conditional end

           } // access mem condition end

       } // end mem code check end

    // code is dev
    else
       {
        // check for "in" dev command
        if( compareString( localNode -> opCommand -> inOutArg,
                           "in" ) == STR_EQ )
           {
            // find the io device

            // hard drive
            if( compareString( localNode -> opCommand -> strArg1,
                               "hard drive" ) == STR_EQ )
               {
                copyString( ioString, "hard drive" );
               }
            // keyboard
            else if( compareString( localNode -> opCommand -> strArg1,
                               "keyboard" ) == STR_EQ )
               {
                copyString( ioString, "keyboard" );
               }
            // monitor
            else if( compareString( localNode -> opCommand -> strArg1,
                               "monitor" ) == STR_EQ )
               {
                copyString( ioString, "monitor" );
               }
            // serial
            else if( compareString( localNode -> opCommand -> strArg1,
                               "serial" ) == STR_EQ )
               {
                copyString( ioString, "serial" );
               }
            // sound signal
            else if( compareString( localNode -> opCommand -> strArg1,
                               "sound signal" ) == STR_EQ )
               {
                copyString( ioString, "sound signal" );
               }
            // usb
            else if( compareString( localNode -> opCommand -> strArg1,
                               "usb" ) == STR_EQ )
               {
                copyString( ioString, "usb" );
               }
            // video signal
            else if( compareString( localNode -> opCommand -> strArg1,
                               "video signal" ) == STR_EQ )
               {
                copyString( ioString, "video signal" );
               }


            // display i/o operation to monitor
            if( toMonitorFlag )
               {
                printf( "%s input operation start\n", ioString );
               }

            // log i/o operation
            if( toFileFlag )
               {
                // copy string to logString variable
                sprintf( logString,
                         "%s input operation start\n", ioString );
                // copy string to localLogNode member from logString
                copyString( localLogNode -> logContents, logString );
                // add logString variable as a node to our log file linked list
                addLogNode( localLogHead, localLogNode );
                // clear logString variable
                logString[0] = NULL_CHAR;
               }

            // assign values to multiply together for ioTime
            pcbCommandTime = localNode -> opCommand -> intArg2;
            cpuCycleRate = localConfigPointer -> cpuSchedCycles;

            // get io time
            ioTime = pcbCommandTime * cpuCycleRate;

            // pthread operation
            pthread_create( &pthreadID, NULL,
                            handleThreadIO, (void *)&(ioTime) );
            pthread_join( pthreadID, NULL );

            // reduce i/o time from node's field timeRemaining
            localNode -> timeRemaining = localNode -> timeRemaining - ioTime;

            // get io process end time
            localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );


            // block current process for preemptive algorithms
            if( preemptionFlag )
               {
                // get time when blocking is detected
                localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

                // display blocked dev op process to monitor
                if( toMonitorFlag )
                   {
                    printf( "  %10.6f, OS: Process %d blocked "
                            "for %s operation\n",
                            localTimeDouble, localNode -> pid, ioString );
                   }

                // log blocked process
                if( toFileFlag )
                   {
                     // copy string to logString variable
                     sprintf( logString, "  %10.6f, OS: Process %d blocked "
                              "for %s operation\n",
                              localTimeDouble, localNode -> pid, ioString );
                     // copy string to localLogNode member from logString
                     copyString( localLogNode -> logContents, logString );
                     // add logString variable as a node to our log file
                     // linked list
                     addLogNode( localLogHead, localLogNode );
                     // clear logString variable
                     logString[0] = NULL_CHAR;
                   }

                // set process state to blocked
                localNode -> pcbState = BLOCKED;

                // write to string for interrupt node
                sprintf(logString, " %s input operation\n", ioString );

                // add interrupt node to queue
                queueInterruptNode( interruptHead,
                                    localNode,
                                    logString,
                                    pcbCommandTime );

                // clear log string
                logString[0] = NULL_CHAR;

                // get blocked state time
                localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

                // display blocked dev op process to monitor
                if( toMonitorFlag )
                   {
                    printf( "  %10.6f, OS: Process %d set from "
                            "RUNNING to BLOCKED\n",
                            localTimeDouble, localNode -> pid );
                   }

                // log blocked process
                if( toFileFlag )
                   {
                     // copy string to logString variable
                     sprintf( logString, "  %10.6f, OS: Process %d set from "
                              "RUNNING to BLOCKED\n",
                              localTimeDouble, localNode -> pid );
                     // copy string to localLogNode member from logString
                     copyString( localLogNode -> logContents, logString );
                     // add logString variable as a node to our log file
                     // linked list
                     addLogNode( localLogHead, localLogNode );
                     // clear logString variable
                     logString[0] = NULL_CHAR;
                   }
               } // end preemption dev operation

            // execute non preemptive algorithm dev in operatoins
            else
               {
                // display process end
                if( toMonitorFlag )
                   {
                    printf("  %10.6f, Process: %d, %s input operation end\n",
                              localTimeDouble, localNode -> pid, ioString );
                   }

                // log process end
                if( toFileFlag )
                   {
                    // copy string to logString variable
                    sprintf( logString,
                             "  %10.6f, Process: %d, %s input operation end\n",
                             localTimeDouble, localNode -> pid, ioString );
                    // copy string to localLogNode member from logString
                    copyString( localLogNode -> logContents, logString );
                    // add logString variable as a node to our log file
                    // linked list
                    addLogNode( localLogHead, localLogNode );
                    // clear logString variable
                    logString[0] = NULL_CHAR;
                   }
               } // end non preemption dev in processing

           } // end "in" dev command conditional

        // its an out command
        else
           {
            //find the io device

            // hard drive
            if( compareString( localNode -> opCommand -> strArg1,
                               "hard drive" ) == STR_EQ )
               {
                copyString( ioString, "hard drive" );

               }
            // keyboard
            else if( compareString( localNode -> opCommand -> strArg1,
                               "keyboard" ) == STR_EQ )
               {
                copyString( ioString, "keyboard" );
               }
            // monitor
            else if( compareString( localNode -> opCommand -> strArg1,
                               "monitor" ) == STR_EQ )
               {
                copyString( ioString, "monitor" );
               }
            // serial
            else if( compareString( localNode -> opCommand -> strArg1,
                               "serial" ) == STR_EQ )
               {
                copyString( ioString, "serial" );
               }
            // sound signal
            else if( compareString( localNode -> opCommand -> strArg1,
                               "sound signal" ) == STR_EQ )
               {
                copyString( ioString, "sound signal" );
               }
            // usb
            else if( compareString( localNode -> opCommand -> strArg1,
                               "usb" ) == STR_EQ )
               {
                copyString( ioString, "usb" );
               }
            // video signal
            else if( compareString( localNode -> opCommand -> strArg1,
                               "video signal" ) == STR_EQ )
               {
                copyString( ioString, "video signal" );
               }

            // display operation start
            if( toMonitorFlag )
               {
                printf( "%s output operation start\n", ioString );
               }

            if( toFileFlag )
               {
                // copy string to logString variable
                sprintf( logString, "%s output operation start\n", ioString );
                // copy string to localLogNode member from logString
                copyString( localLogNode -> logContents, logString );
                // add logString variable as a node to our log file linked list
                addLogNode( localLogHead, localLogNode );
                // clear logString variable
                logString[0] = NULL_CHAR;
               }


            // assign values to multiply together for ioTime
            pcbCommandTime = localNode -> opCommand -> intArg2;
            cpuCycleRate = localConfigPointer -> cpuSchedCycles;

            // get io time
            ioTime = pcbCommandTime * cpuCycleRate;

            // pthread operation
            pthread_create( &pthreadID, NULL,
                            handleThreadIO, (void *)&(ioTime) );
            pthread_join( pthreadID, NULL );

            // reduce Time from node's field timeRemaining
            localNode -> timeRemaining = localNode -> timeRemaining - ioTime;

            // block current process for preemptive algorithms
            if( preemptionFlag )
               {
                // get time when blocking is detected
                localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

                // display blocked dev op process to monitor
                if( toMonitorFlag )
                   {
                    printf( "  %10.6f, OS: Process %d blocked "
                            "for %s operation\n",
                            localTimeDouble, localNode -> pid, ioString );
                   }

                // log blocked process
                if( toFileFlag )
                   {
                     // copy string to logString variable
                     sprintf( logString, "  %10.6f, OS: Process %d blocked "
                              "for %s operation\n",
                              localTimeDouble, localNode -> pid, ioString );
                     // copy string to localLogNode member from logString
                     copyString( localLogNode -> logContents, logString );
                     // add logString variable as a node to our log file
                     // linked list
                     addLogNode( localLogHead, localLogNode );
                     // clear logString variable
                     logString[0] = NULL_CHAR;
                   }

                // set process state to blocked
                localNode -> pcbState = BLOCKED;

                // write to string for interrupt node
                sprintf(logString, " %s output operation\n", ioString );

                // add interrupt node to queue
                queueInterruptNode( interruptHead,
                                    localNode,
                                    logString,
                                    pcbCommandTime );

                // clear log string
                logString[0] = NULL_CHAR;

                // get blocked state time
                localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

                // display blocked dev op process to monitor
                if( toMonitorFlag )
                   {
                    printf( "  %10.6f, OS: Process %d set from "
                            "RUNNING to BLOCKED\n",
                            localTimeDouble, localNode -> pid );
                   }

                // log blocked process
                if( toFileFlag )
                   {
                     // copy string to logString variable
                     sprintf( logString, "  %10.6f, OS: Process %d set from "
                              "RUNNING to BLOCKED\n",
                              localTimeDouble, localNode -> pid );
                     // copy string to localLogNode member from logString
                     copyString( localLogNode -> logContents, logString );
                     // add logString variable as a node to our log file
                     // linked list
                     addLogNode( localLogHead, localLogNode );
                     // clear logString variable
                     logString[0] = NULL_CHAR;
                   }
               } // end preemption dev operation

            // Process non preemptive io operations
            else
               {
                // get io process end time
                localTimeDouble = accessTimer( LAP_TIMER, localTimeStamp );

                // display process end to monitor
                if( toMonitorFlag )
                   {
                    printf("  %10.6f, Process: %d, %s ouput operation end\n",
                              localTimeDouble, localNode -> pid, ioString );
                   }

                // log process end
                if( toFileFlag )
                   {
                    // copy string to logString variable
                    sprintf( logString,
                             "  %10.6f, Process: %d, %s ouput operation end\n",
                             localTimeDouble, localNode -> pid, ioString );
                    // copy string to localLogNode member from logString
                    copyString( localLogNode -> logContents, logString );
                    // add logString variable as a node to our log file linked list
                    addLogNode( localLogHead, localLogNode );
                    // clear logString variable
                    logString[0] = NULL_CHAR;
                   }

               } // end non preemptive dev processing
           } // end "out" dev command conditional
       }// end dev code condition
   } // end show PCB commands function

// write log linked list contenets to log file
void writeToLogFile( ConfigDataType *configPointer,
                     logFileType *logListPointer )
   {
    // create file with file name from config data
    FILE *filePointer = fopen( configPointer -> logToFileName, "w" );

    // traverse through linked List
    while( logListPointer != NULL )
       {
        // write new line in log file
        fputs( logListPointer -> logContents, filePointer );
        // traverse to next node
        logListPointer = logListPointer -> nextNode;
       }

    // close the log file. No return, void function.
    fclose( filePointer );
   }
