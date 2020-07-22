#include <stdlib.h>
#include <stdio.h>
#include <abd_tool/base_defn.h>

#ifndef loaded_settm
#define loaded_settm

typedef struct sett
{
    char objOutPath[100];
    char eventsOutPath[100];
    ABD_STATE verbose;
} ABD_SETTINGS;
#define NO_PATH NULL
static ABD_SETTINGS *settings = NO_PATH;
static char *userPath = NO_PATH;
static char *folderPath = NO_PATH;
static char *filePath = NO_PATH;
static char *displayerPath = NO_PATH;
#endif

int writeCurrSettings(FILE *settingsFile);
void loadSettings();
int load(FILE *settingsFile);
int closeSetFile(FILE *file);
FILE *openSetFile();
void buildFilePath();
int buildFolderPath();
int checkFolderHierarchy();
void createDefaults(FILE *settingsFile);
char *getObjPath();
char *getEventsPath();
void checkSettings();
void buildDisplayerPath();
char *getDisplayerPath();
char *getCommand();
char *getJSpath(char *jsFileName);
