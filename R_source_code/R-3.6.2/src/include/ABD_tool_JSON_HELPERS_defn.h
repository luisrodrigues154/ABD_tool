/*
    In this file are declared the methods definitions as well as needed macros
    to export the ABD_tool collected data to JSON files
    The implementation of the below declared methods is implemented at
    the file: 
        ABD_tool_JSON_HELPERS.h



*/

/* 
    The needed libraries to this work
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    The needed constants to clear the code and make "things" more
    abstracted and easy to modify
*/
#define JSON_INDENT_0 ""
#define JSON_INDENT_1 "\t"
#define JSON_INDENT_2 "\t\t"
#define JSON_INDENT_3 "\t\t\t"
#define JSON_INDENT_4 "\t\t\t\t"
#define JSON_INDENT_5 "\t\t\t\t\t"

#define PERSIST_OBJECTS 1
#define PERSIST_CODE_FLOW 2
#define OBJECTS_FILE_PATH "/home/luis/Desktop/objects.json"
#define FILE_OPEN_MODE "w+"


/*
    The prototypes regarding the JSON output are below

*/

FILE * openFile(char * filePath);
int closeFile(FILE * outputFile);
void initializeJSONstructure(FILE * outputFile, char * indentation);
void finalizeJSONstructure(FILE * outputFile, char * indentation);
void writeObjectModificationToFile(FILE * outputFile, ABD_OBJECT_MOD * modification);
void writeObjectToFile(FILE * outputFile, ABD_OBJECT * obj);
void persistObjects(FILE * outputFile, ABD_OBJECT * objectsRegistry);
void persistInformation(int structureToStore, ABD_OBJECT * objectsRegistry);