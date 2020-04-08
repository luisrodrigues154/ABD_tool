#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <abd_tool/settings_manager_defn.h>

int checkFolderHierarchy(){
    DIR * setDir;
    if((setDir = opendir(folderPath)) == NULL){
        //folder does not exists, create
        if(mkdir(folderPath, 0700))
            //error creating
            return 0;
    }else
        closedir(setDir);
    //folder ready
    return 1;
}

int buildFolderPath(){
    char * login = getlogin();
    
    if(login == NULL)
        return 0;

    int userPathLen;
    int folderPathLen;
    
    #ifdef __APPLE__
        userPathLen = strlen(login) + strlen("/Users/");
        folderPathLen = userPathLen + strlen("/Documents/ABD_tool");
    #elif defined __LINUX__
        userPathLen = strlen(login) + strlen("/home/");
        folderPathLen = userPathLen + strlen("/Documents/ABD_tool");
    #endif


    folderPath = (char *) malloc(folderPathLen * sizeof(char) + 1);
    userPath = (char *) malloc(userPathLen * sizeof(char) + 1);

    memset(userPath, 0, userPathLen * sizeof(char));
    memset(folderPath, 0, folderPathLen * sizeof(char));
   
    #ifdef __APPLE__
        strncat(userPath, "/Users/", strlen("/Users/")*sizeof(char));
    #elif defined __LINUX__
        strncat(userPath, "/home/", strlen("/home/")*sizeof(char));
    #endif
    
    strncat(userPath, login, strlen(login)*sizeof(char));

    strncat(folderPath, userPath, userPathLen * sizeof(char));
    strncat(folderPath, "/Documents/ABD_tool", (folderPathLen - userPathLen) * sizeof(char));

    return 1;
}

void buildFilePath(){
    int size = strlen(folderPath) + strlen("/settings.dat");
    
    filePath = (char *) malloc(size * sizeof(char) + 1);
   
    memset(filePath, 0, size * sizeof(char));
    
    strncat(filePath, folderPath, strlen(folderPath) * sizeof(char));
    strncat(filePath, "/settings.dat", strlen("/settings.dat") * sizeof(char));
    
}

FILE * openSetFile(){
    return fopen(filePath, "ab+");
}

int closeSetFile(FILE * file){
   return fclose(file);
}

int load(FILE * settingsFile){
    rewind(settingsFile);
    settings = (ABD_SETTINGS *) malloc(sizeof(ABD_SETTINGS));
    return fread(settings, sizeof(ABD_SETTINGS),1, settingsFile);
}

int writeCurrSettings(FILE * settingsFile){
    rewind(settingsFile);
    return fwrite(settings, sizeof(ABD_SETTINGS), 1, settingsFile);
}

void createDefaults(FILE * settingsFile){
    int eventsOutSize = strlen("/events.json");
    int objOutSize = strlen("/objects.json");
    int folderPathLen = strlen(folderPath);
    settings = (ABD_SETTINGS *) malloc(sizeof(ABD_SETTINGS));

    settings->eventsOutPath[0] = '\0';
    settings->objOutPath[0] = '\0';

    strncat(settings->eventsOutPath, folderPath, strlen(folderPath) * sizeof(char));
    strncat(settings->objOutPath, folderPath, strlen(folderPath) * sizeof(char));

    strncat(settings->eventsOutPath, "/events.json" , eventsOutSize * sizeof(char));
    strncat(settings->objOutPath, "/objects.json" , objOutSize * sizeof(char));

    writeCurrSettings(settingsFile);
}

int loadSettings(){
    FILE * settingsFile;
    buildFolderPath();
    buildFilePath();
    if(checkFolderHierarchy()){
        printf("Loading ABD_tool settings... ");
        if((settingsFile = openSetFile(filePath)) != NULL){
            if(!load(settingsFile)){
                puts("ERROR...");
                printf("Creating default settings... ");
                createDefaults(settingsFile);
            }    
            
            puts("DONE...\n");
            closeSetFile(settingsFile);
        }
    }
}

void checkSettings(){
    if(settings == NULL)
        loadSettings();
}

char * getObjPath(){
    return settings->objOutPath;
}
char * getEventsPath(){
    return settings->eventsOutPath;
}