// Preprocessor directive
#ifndef CONFIGOPS_H
#define CONFIGOPS_H

#include "StandardConstants.h"
#include "StringUtils.h"


// GLOBAL CONSTANTS - may be used in other files
typedef enum { CFG_FILE_ACCESS_ERR,
               CFG_CORRUPT_DESCRIPTOR_ERR,
               CFG_DATA_OUT_OF_RANGE_ERR,
               CFG_CORRUPT_PROMPT_ERR,
               CFG_VERSION_CODE,
               CFG_MD_FILE_NAME_CODE,
               CFG_CPU_SCHED_CODE,
               CFG_QUANT_CYCLES_CODE,
               CFG_MEM_DISPLAY_CODE,
               CFG_MEM_AVAILABLE_CODE,
               CFG_PROC_CYCLES_CODE,
               CFG_IO_CYCLES_CODE,
               CFG_LOG_TO_CODE,
               CFG_LOG_FILE_NAME_CODE } ConfigCodeMessages;

typedef enum { CPU_SCHED_SJF_N_CODE,
               CPU_SCHED_SRTF_P_CODE,
               CPU_SCHED_FCFS_P_CODE,
               CPU_SCHED_RR_P_CODE,
               CPU_SCHED_FCFS_N_CODE,
               LOGTO_MONITOR_CODE,
               LOGTO_FILE_CODE,
               LOGTO_BOTH_CODE,
               NON_PREEMPTIVE_CODE,
               PREEMPTIVE_CODE } ConfigDataCodes;

// config data structure
typedef struct ConfigDataType
   {
    double version;
    char metaDataFileName[ LARGE_STR_LEN ];
    int cpuSchedCycles;
    int cpuSchedCode;
    int quantumCycles;
    bool memDisplay;
    int memAvailable;
    int procCycleRate;
    int ioCycleRate;
    int logToCode;
    char logToFileName[ LARGE_STR_LEN ];
   } ConfigDataType;

// function prototypes

/*
Name: clearConfigData
Process: frees dynamically allocated config data structure
         if it has not already been freed
Function Input/Parameters: pointer to config data structure (ConfigDataType *)
Function Output/Parameters: none
Function Output/Returned: NULL (ConfigDataType *)
Device Input/device: none
Device Output/device: none
Dependencies: free
*/
ConfigDataType *clearConfigData( ConfigDataType *configData );


/*
Name: configCodeToString
Process: utility function converts configuration code numbers
Function Output/Parameters: resulting output string (char *)
Function Output/Returned: none
Device Input/device: none
Device Output/monitor: none
Dependencies: copyString
*/
void configCodeToString( int code, char *outString );


/*
Name: displayConfigData
Process: screen dump/display of all config data
Function Input/Parameters: pointer to config data structure (configDataType *)
Function Output/Parameters: none
Function Output/Returned: none
Device Input/device: none
Device Output/monitor: displayed as specified
Dependencies: configCodeToString, printf 
*/
void displayConfigData( ConfigDataType *configData );


/*
Name: getConfigData
Process: driver function for capturing configuration data from a config file
Function Input/Parameters: file name (const char *)
Function Output/Parametrs: pointer to config data pointer (ConfigDataType **),
                           end/result state message pointer (char *)
Function Output/Returned: Boolean result of data access operation (bool)
Device Input/file: config data uploaded
Device Output/device:none
Dependencies: copyString, fopen, getStringToDelimiter, compareString,
              fclose, malloc, free, stripTrailingSpaces, getDataLineCode,
              fscanf, setStrToLowerCase, valueInRange, getCpuSchedCode,
              getLogToCode
*/
bool getConfigData( const char *fileName,
                                ConfigDataType **configData, char *endStateMsg );


/*
Name: getCpuSchedCode
Process: converts cpu schedule string to code (all scheduling possibilites)
Function Input/Parameters: lower case code string (const char *)
Function Output/Parameters: none
Function Output/Returned: cpu schedule code (ConfigDataCodes)
Device Input/device: none
Device Output/device: none
Dependencies: compare string
*/
ConfigDataCodes getCpuSchedCode( const char *lowerCaseCodeStr );


/*
Name: getDataLineCode
Process: converts leader line string to configuration code value
         (all config file leader lines)
Function Input/Parameters: config leader line string (const char *)
Function Output/Parametes: none
Function Output/Returned: configuration code value (ConfigCodeMessages)
Device Input/device: none
Device Output/device: none
Dependencies: compareString
*/
ConfigCodeMessages getDataLineCode( const char *dataBuffer );


/*
Name: getLogToCode
Process: converts "log to" text to configuration data code
         (three log to strings)
Function Input/Parameters: lower case log to string (const char *)
Function Output/Parameters: none
Function Output/Returned: configuration data code value (ConfigDataCodes)
Device Input/device: none
Device Output/device: none
Dependencies: compareString
*/
ConfigDataCodes getLogToCode( const char *lowerCaseLogToStr );


/*
Name: stripTrailingSpaces
Process: removes trailing spaces from input config leader lines
Function Input/Parameters: config leader line string (char *)
Function Output/Parameters: updated config leader line string (char *)
Function Output/Returned: none
Device Input/device: none
Device Output/device: none
Dependencies: getStringLength
*/
void stripTrailingSpaces( char *str );


/*
Name: valueInRange
Process: check for config data value in range, including string values
         (all config data values)
Function Input/Parameters: line code number for specific config value (int),
                           integer value, as needed (int)
                           double value, as needed (double)
                           string value, as needed (const char *)
Function Output/Parameters: none
Function Output/Returned: Boolean result of range test (bool)
Device Input/device: none
Device Output/device: none
Dependencies: compareString
*/
bool valueInRange( int lineCode, int intVal,
                              double doubleVal, const char *lowerCaseStringVal );


#endif  // CONFIGOPS_H
