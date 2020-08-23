#include <stdlib.h>
#include <stdio.h>
#include <abd_tool/base_defn.h>

#ifndef loaded_settm
#define loaded_settm

typedef struct sett
{
    ABD_STATE launchOnStop;
    ABD_STATE verbose;
    char objOutPath[214];
    char eventsOutPath[214];
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
char *getFolderPath();
Rboolean checkPath(const char *path);
void saveNewPath(const char *path, int target);
void setLaunchOption(ABD_STATE state);
ABD_STATE launchOnStop();
void forceDefaults();
ABD_STATE useVerbose();
void updateVerboseMode(ABD_STATE newState);