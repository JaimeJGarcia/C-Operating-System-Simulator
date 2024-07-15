// header files
#include "SimDriver.h"

int main( int argc, char **argv )
   {
      // Initialize program

         // initialize variables
         Boolean programRunFlag = False;
         Boolean configDisplayFlag = False;
         Boolean configUploadFlag = False;
         Boolean mdDisplayFlag = False;
         Boolean runSimFlag = False;
         Boolean infoFlag = False;
         int argIndex = 1;
         int lastFourLetters = 4;
         int fileStrLen, fileStrSubLoc;
         char fileName[ STD_STR_LEN ];
         char errorMessage[ MAX_STR_LEN ];
         ConfigDataType *configDataPtr = NULL;
         OpCodeType *metaDataPtr = NULL;

         // show title
           // function: printf
         printf( "\nSimulator Program\n" );
         printf( "=================\n\n" );

      // check for only one arg
      if( argc < 2 )
        {
         // show error message, end program
           //function: showProgramFormat
         showProgramFormat();

         // unset program run flag
         programRunFlag = False;

         // set info flag
         infoFlag = True;
        }

      // initialize file name to empty
      fileName[ 0 ] = NULL_CHAR;

      // loop across args ( starting at 1 ) and proram run flag
      while( programRunFlag == False && argIndex < argc )
        {
         // find lengths to verify file name
           // function: getStringLength, findSubString
         fileStrLen = getStringLength( argv[ argIndex ] );
         fileStrSubLoc = findSubString( argv[ argIndex ], ".cnf" );

         // check for -dc ( display config flag )
           // function: compareString
         if( compareString( argv[ argIndex ], "-dc" ) == STR_EQ )
           {
            // set config upload flag
            configUploadFlag = True;

            // set config display flag
            configDisplayFlag = True;
           }

         // otherwise, check for -dm ( display metadata flag )
           //function: compareString
         else if( compareString( argv[ argIndex ], "-dm" ) == STR_EQ )
           {
            // set config upload flag
            configUploadFlag = True;

            // set run simulator flag
            mdDisplayFlag = True;
           }

         // otherwise, check for -rs ( run simulator )
           //function: compareString
         else if( compareString( argv[ argIndex ], "-rs" ) == STR_EQ )
           {
            // set config upload flag
            configUploadFlag = True;

            // set run simulator flag
            runSimFlag = True;
           }

         // otherwise, check for file name, ending in .cnf
         // must be last flag
         else if( fileStrSubLoc != SUBSTRING_NOT_FOUND
                   && fileStrSubLoc == fileStrLen - lastFourLetters )
           {
            // set file name to variable
              //function: copyString
            copyString( fileName, argv[ argIndex ] );

            // set program run flag
            programRunFlag = True;
           }

         // update arg index
         argIndex++;
        }
      // end arg loop

      // check for command line failure
      if( programRunFlag == False && infoFlag == False )
        {
         // show error message, end program
           //function: printf
         printf( "Incorrect argument line format, program aborted\n\n" );

         // provide command line information to user
           //function: showProgramFormat
         showProgramFormat();
        }

      // check for program run and config upload flag
      if( programRunFlag == True && configUploadFlag == True )
        {
         // upload config file, check for success
           //function: getStringLength, getConfigData
         if( getStringLength( fileName ) > 0
             && getConfigData( fileName, &configDataPtr, errorMessage )
                                                                  == True )
           {

            // check config display flag
            if( configDisplayFlag == True )
              {
               // display config data
                 //function: displayConfigData
               displayConfigData( configDataPtr );
              }
           }

         // otherwise, assume config file upload failure
         else
           {
            // show error message, end program
              //function: printf
            printf( "\nConfig Upload Error: %s, program aborted\n\n", errorMessage );

            // unset program run flag
            programRunFlag = False;
           }
        }

      // check for program run flag
      if( programRunFlag == True
                            && (mdDisplayFlag == True || runSimFlag == True ))
        {

         // upload meta data file, check for success
           //function: getMetaData
         if( getMetaData( configDataPtr->metaDataFileName,
                                             &metaDataPtr, errorMessage )
                                                                    == True)
           {

            // check meta data display flag
            if( mdDisplayFlag == True )
              {
               // display meta data
                 //function: displayMetaData
               displayMetaData( metaDataPtr );
              }

            // check run simulator flag
            if( runSimFlag == True )
              {
               // run simulator
                //function: runSim
               runSim( configDataPtr, metaDataPtr );
              }
           }

         // otherwise, assume meta data upload failure
         else
           {
            // show error message, end program
              //function: printf
            printf( "\nMetadata Upload Error: %s, program aborted\n",
                                                               errorMessage );
           }
        }

      // clean up config data as needed
        //function: clearConfigData
      configDataPtr = clearConfigData( configDataPtr );

      // clean up metadata as needed
        //function: clearMetaDataList
      metaDataPtr = clearMetaDataList( metaDataPtr );

      // show program end
      printf( "\nSimulator Program End.\n\n" );

      // return success
      return 0;
   }
