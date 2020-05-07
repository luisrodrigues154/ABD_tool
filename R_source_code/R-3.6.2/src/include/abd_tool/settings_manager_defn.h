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

static ABD_SETTINGS *settings = NULL;
static char *userPath = NULL;
static char *folderPath = NULL;
static char *filePath = NULL;
#endif

int writeCurrSettings(FILE *settingsFile);
int loadSettings();
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