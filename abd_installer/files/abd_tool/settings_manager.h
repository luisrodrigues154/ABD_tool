#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <abd_tool/settings_manager_defn.h>

#ifdef __linux__
#include <unistd.h>
#endif

char *getDisplayerPath()
{
    return displayerPath;
}

char *getCommand()
{
    int partialPathSize = strlen(displayerPath);
    int htmlPathSize = strlen("index.html");
    int openSize = 5;
    char *htmlPath;
    #ifdef __APPLE__
    char open[] = "open ";
    openSize = strlen(open);
    htmlPath = (char *)malloc(sizeof(char) * (partialPathSize + htmlPathSize + openSize + 1));
    strcpy(htmlPath, open);
    strcat(htmlPath, displayerPath);
    strcat(htmlPath, "index.html");
    #elif defined __linux__
    char open[] = "nohup xdg-open ";
    char devNull[] = "> /dev/null 2>&1";
    openSize = strlen(open);

    htmlPath = (char *)malloc(sizeof(char) * (partialPathSize + htmlPathSize + openSize + strlen(devNull) + 1));
    strcpy(htmlPath, open);
    strcat(htmlPath, displayerPath);
    strcat(htmlPath, "index.html");
    strcat(htmlPath, devNull);

    #endif

    return htmlPath;
}

char *getJSpath(char *jsFileName)
{
    int partialPathSize = strlen(displayerPath);

    int jsPathSize = strlen("custom/js/");
    jsPathSize += strlen(jsFileName) + 3;
    char *finalPath = (char *)malloc(sizeof(char) * (partialPathSize + jsPathSize + 1));
    finalPath[0] = '\0';
    strcat(finalPath, displayerPath);
    strcat(finalPath, "custom/js/");
    strcat(finalPath, jsFileName);
    strcat(finalPath, ".js");

    return finalPath;
}

int findOverflow(const char *path, const char *str)
{

    char *result = strstr(path, str);
    int position = result - path;
    int overflow = strlen(path) - position;
    return overflow;
}

void mergePaths(const char *path, int oldPathSize)
{
    int overflow = findOverflow(path, "bin/exec/R");

    char append[] = "src/include/abd_tool/displayer/";
    int appendSize = strlen(append);
    int useAppend = 0;
    int displayerPathSize = oldPathSize - overflow + appendSize;
    displayerPath = (char *)malloc(sizeof(char) * displayerPathSize + 1);
    for (int i = 0, j = 0;; i++)
    {
        if (useAppend && j < appendSize)
        {
            displayerPath[i] = append[j];
            j++;
        }
        else if (!useAppend)
        {
            if (i == (oldPathSize - overflow - 1))
                useAppend = 1;
            displayerPath[i] = path[i];
        }
        else
        {
            break;
        }
    }
    displayerPath[displayerPathSize] = '\0';
}
void buildDisplayerPath()
{
    char path[1024];
    uint32_t size = sizeof(path);
    #ifdef __APPLE__

    if (_NSGetExecutablePath(path, &size) == 0)
    {
        //path is valid, and retrieved
        size = strlen(path);
        mergePaths(path, size);
    }

    #elif defined __linux__
    if (readlink("/proc/self/exe", path, size))
    {
        size = strlen(path);
        mergePaths(path, size);
    }
    //
    #endif
}
void saveNewPath(const char *path, int target)
{

    switch (target)
    {
    
    case 1:
        settings->objOutPath[0] = '\0';
        strcat(settings->objOutPath, path);
        strcat(settings->objOutPath, "/objects.json");
        break;
    case 2:
        settings->eventsOutPath[0] = '\0';
        strcat(settings->eventsOutPath, path);
        strcat(settings->eventsOutPath, "/events.json");
        break;
    case 3:
        settings->warnsAndErrs[0] = '\0';
        strcat(settings->warnsAndErrs, path);
        strcat(settings->warnsAndErrs, "/wrnerr.json");
        break;
    case 0:
        settings->eventsOutPath[0] = '\0';
        settings->objOutPath[0] = '\0';
        settings->warnsAndErrs[0] = '\0';
        
        
        strcat(settings->eventsOutPath, path);
        strcat(settings->objOutPath, path);
        strcat(settings->warnsAndErrs, path);

        strcat(settings->eventsOutPath, "/events.json");
        strcat(settings->objOutPath, "/objects.json");
        strcat(settings->warnsAndErrs, "/wrnerr.json");
        break;
    default:
        break;
    }

    FILE *setFile = openSetFile("wb");
    if (setFile == NO_PATH)
    {
        printf("\n\t[ABD_TOOL] Error saving settings...\n");
        return;
    }
    writeCurrSettings(setFile);
    closeSetFile(setFile);
}

Rboolean checkPath(const char *path)
{
    DIR *dir;
    if ((dir = opendir(path)) == NO_PATH)
        return FALSE;

    closedir(dir);
    return TRUE;
}

int checkFolderHierarchy()
{
    DIR *setDir;
    if ((setDir = opendir(folderPath)) == NO_PATH)
    {
        //folder does not exists, create
        if (mkdir(folderPath, 0700))
            //error creating
            return 0;
    }
    else
        closedir(setDir);
    //folder ready
    return 1;
}

int buildFolderPath()
{
    char *login = getlogin();

    if (login == NO_PATH)
        return 0;

    int userPathLen;
    int folderPathLen;

    #ifdef __APPLE__
    userPathLen = strlen(login) + strlen("/Users/");
    folderPathLen = userPathLen + strlen("/Documents/ABD_tool");
    #elif defined __linux__
    userPathLen = strlen(login) + strlen("/home/");
    folderPathLen = userPathLen + strlen("/Documents/ABD_tool");
    #endif

    folderPath = (char *)malloc(folderPathLen * sizeof(char) + 1);
    userPath = (char *)malloc(userPathLen * sizeof(char) + 1);

    memset(userPath, 0, userPathLen * sizeof(char));
    memset(folderPath, 0, folderPathLen * sizeof(char));

    #ifdef __APPLE__
    strcat(userPath, "/Users/");
    #elif defined __linux__
    strcat(userPath, "/home/");
    #endif

    strcat(userPath, login);

    strncat(folderPath, userPath, userPathLen * sizeof(char));
    strcat(folderPath, "/Documents/ABD_tool");

    return 1;
}

void buildFilePath()
{
    int size = strlen(folderPath) + strlen("/settings.dat");

    filePath = (char *)malloc(size * sizeof(char) + 1);

    memset(filePath, 0, size * sizeof(char));

    strcat(filePath, folderPath);
    strcat(filePath, "/settings.dat");
}

FILE *openSetFile(const char *mode)
{
    return fopen(filePath, mode);
}

int closeSetFile(FILE *file)
{
    return fclose(file);
}

int load(FILE *settingsFile)
{
    rewind(settingsFile);
    settings = (ABD_SETTINGS *)malloc(sizeof(ABD_SETTINGS));
    return fread(settings, sizeof(ABD_SETTINGS), 1, settingsFile);
}

int writeCurrSettings(FILE *settingsFile)
{
    rewind(settingsFile);
    return fwrite(settings, sizeof(ABD_SETTINGS), 1, settingsFile);
}

void setLaunchOption(ABD_STATE state)
{
    settings->launchOnStop = state;
    FILE *setFile = openSetFile("wb");
    if (setFile == NO_PATH)
    {
        printf("\n\t[ABD_TOOL] Error saving settings...\n");
        return;
    }

    writeCurrSettings(setFile);
    closeSetFile(setFile);
}

void updateVerboseMode(ABD_STATE newState) {
    if (settings->verbose != newState) {
        settings->verbose = newState;
        FILE *setFile = openSetFile("wb");
        if (setFile == NO_PATH)
        {
            printf("\n\t[ABD_TOOL] Error saving settings...\n");
            return;
        }
        writeCurrSettings(setFile);
        closeSetFile(setFile);
    }
}
ABD_STATE useVerbose() {
    return settings->verbose;
}

ABD_STATE launchOnStop()
{
    return settings->launchOnStop;
}

void createDefaults(FILE *settingsFile)
{
    int eventsOutSize = strlen("/events.json");
    int objOutSize = strlen("/objects.json");
    int folderPathLen = strlen(folderPath);
    settings = (ABD_SETTINGS *)malloc(sizeof(ABD_SETTINGS));
    settings->verbose = ABD_DISABLE;
    settings->launchOnStop = ABD_ENABLE;
    settings->eventsOutPath[0] = '\0';
    settings->objOutPath[0] = '\0';
    settings->warnsAndErrs[0] = '\0';

    strcat(settings->eventsOutPath, folderPath);
    strcat(settings->objOutPath, folderPath);
    strcat(settings->warnsAndErrs, folderPath);

    strcat(settings->eventsOutPath, "/events.json");
    strcat(settings->objOutPath, "/objects.json");
    strcat(settings->warnsAndErrs, "/wrnerr.json");

    writeCurrSettings(settingsFile);
}

void loadSettings()
{
    FILE *settingsFile;
    buildFolderPath();
    buildFilePath();
    buildDisplayerPath();
    if (checkFolderHierarchy())
    {
        printf("\t[ABD_TOOL] Loading settings... ");
        if ((settingsFile = openSetFile("ab+")) != NO_PATH)
        {
            if (!load(settingsFile))
            {
                puts("ERROR");
                printf("\t[ABD_TOOL] Creating default settings... ");
                createDefaults(settingsFile);
            }

            puts("DONE\n");
            closeSetFile(settingsFile);
        }
    }
}

void checkSettings()
{
    messagePrinter("Verifying settings integrity");
    if (settings == NO_PATH)
        loadSettings();


}

void forceDefaults()
{
    FILE *settingsFile;
    buildFolderPath();
    buildFilePath();
    buildDisplayerPath();
    if (checkFolderHierarchy())
    {
        if ((settingsFile = openSetFile("ab+")) != NO_PATH)
        {
            createDefaults(settingsFile);

            closeSetFile(settingsFile);
        }
    }
}
char *getFolderPath()
{
    return folderPath;
}
char *getObjPath()
{
    return settings->objOutPath;
}
char *getEventsPath()
{
    return settings->eventsOutPath;
}
char * getWarningsAndErrorsPath(){
    return settings->warnsAndErrs;
}