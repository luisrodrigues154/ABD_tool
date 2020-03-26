#include <stdlib.h>
#include <stdio.h>




#ifndef loaded_settm
    #define loaded_settm

    typedef struct sett{
        char objOutPath [100];
        char eventsOutPath[100];
    }ABD_SETTINGS;

    static ABD_SETTINGS settings;
    static char * userPath;
    static char * folderPath;
    static char * filePath;
#endif

int writeCurrSettings(FILE * settingsFile);
int loadSettings();
int load(FILE * settingsFile);
int closeSetFile(FILE * file);
FILE * openSetFile();
void buildFilePath();
int buildFolderPath();
int checkFolderHierarchy();
void createDefaults(FILE * settingsFile);