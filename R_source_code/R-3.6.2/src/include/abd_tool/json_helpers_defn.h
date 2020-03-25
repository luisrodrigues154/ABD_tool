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
#include <abd_tool/obj_manager_defn.h>

/*
    The needed constants to clear the code and make "things" more
    abstracted and easy to modify
*/
//event types
#ifndef loaded_jm
  #define loaded_jm

    #define FILE_NOT_FOUND NULL
    #define FILE_OPEN_MODE "w"
    typedef enum{
        INDENT_0 = 0,
        INDENT_1 = 1,
        INDENT_2 = 2,
        INDENT_3 = 3,
        INDENT_4 = 4,
        INDENT_5 = 5,
        INDENT_6 = 6,
        INDENT_7 = 7,
        INDENT_8 = 9,
        INDENT_9 = 9,
        INDENT_10 = 10,
        INDENT_11 = 11,
        INDENT_12 = 12,
        INDENT_13 = 13,
        INDENT_14 = 14
    }JSON_INDENT;
    typedef enum{
        OBJECTS,
        EVENTS
    }ABD_PERSIST;
    char * OBJECTS_FILE_PATH = "/home/luis/Desktop/objects.json";
    char * EVENTS_FILE_PATH = "/home/luis/Desktop/events.json";
#endif



/*
    The prototypes regarding the JSON output are below

*/

FILE * openFile(char * filePath);
int closeFile(FILE * outputFile);
char * getStrFromIndent(JSON_INDENT indent);
void writeObjVector(FILE * out, ABD_VEC_OBJ * vecObj, int id);
void writeObjModsToFile(FILE * out, ABD_OBJECT_MOD * listStart);
void writeObjToFile(FILE * out, ABD_OBJECT * obj);
void saveObjects(FILE * out);
void persistInformation();

void writeArgVector(FILE * out, ABD_VEC_OBJ * vecObj, JSON_INDENT indent);
void writeArgValueToFile(FILE * out, ABD_OBJECT_MOD * value, JSON_INDENT indent);
void saveFuncArgs(FILE * out, ABD_EVENT_ARG * argsList);
void saveFuncEvent(FILE * out, ABD_FUNC_EVENT * funcEvent);
void saveRetEvent(FILE * out, ABD_RET_EVENT * retEvent);
void saveEvents(FILE * out);