#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <abd_tool/settings_manager_defn.h>

#ifdef __LINUX__
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
    char open[] = "open ";
    char *htmlPath = (char *)malloc(sizeof(char) * (partialPathSize + htmlPathSize + openSize + 1));
    strcpy(htmlPath, open);
    strcat(htmlPath, displayerPath);
    strcat(htmlPath, "index.html");
    return htmlPath;
}
char *getJSpath(char *jsFileName)
{
    int partialPathSize = strlen(displayerPath);
    int jsPathSize = strlen("custom/js/");
    jsPathSize += strlen(jsFileName) + 2;
    char *finalPath = (char *)malloc(sizeof(char) * (partialPathSize + jsPathSize + 1));
    strcat(finalPath, displayerPath);
    strcat(finalPath, "custom/js/");
    strcat(finalPath, jsFileName);
    strcat(finalPath, ".js");
    return finalPath;
}
void mergePaths(const char *path, int oldPathSize)
{
    int overflow = strlen("bin/exec/R");
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
#ifdef __APPLE__
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
    {
        //path is valid, and retrieved
        size = strlen(path);
        mergePaths(path, size);
    }

#elif __LINUX__
    //
#endif
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
#elif defined __LINUX__
    userPathLen = strlen(login) + strlen("/home/");
    folderPathLen = userPathLen + strlen("/Documents/ABD_tool");
#endif

    folderPath = (char *)malloc(folderPathLen * sizeof(char) + 1);
    userPath = (char *)malloc(userPathLen * sizeof(char) + 1);

    memset(userPath, 0, userPathLen * sizeof(char));
    memset(folderPath, 0, folderPathLen * sizeof(char));

#ifdef __APPLE__
    strncat(userPath, "/Users/", strlen("/Users/") * sizeof(char));
#elif defined __LINUX__
    strncat(userPath, "/home/", strlen("/home/") * sizeof(char));
#endif

    strncat(userPath, login, strlen(login) * sizeof(char));

    strncat(folderPath, userPath, userPathLen * sizeof(char));
    strncat(folderPath, "/Documents/ABD_tool", (folderPathLen - userPathLen) * sizeof(char));

    return 1;
}

void buildFilePath()
{
    int size = strlen(folderPath) + strlen("/settings.dat");

    filePath = (char *)malloc(size * sizeof(char) + 1);

    memset(filePath, 0, size * sizeof(char));

    strncat(filePath, folderPath, strlen(folderPath) * sizeof(char));
    strncat(filePath, "/settings.dat", strlen("/settings.dat") * sizeof(char));
}

FILE *openSetFile()
{
    return fopen(filePath, "ab+");
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

void createDefaults(FILE *settingsFile)
{
    int eventsOutSize = strlen("/events.json");
    int objOutSize = strlen("/objects.json");
    int folderPathLen = strlen(folderPath);
    settings = (ABD_SETTINGS *)malloc(sizeof(ABD_SETTINGS));
    settings->verbose = ABD_ENABLE;
    settings->eventsOutPath[0] = '\0';
    settings->objOutPath[0] = '\0';

    strncat(settings->eventsOutPath, folderPath, strlen(folderPath) * sizeof(char));
    strncat(settings->objOutPath, folderPath, strlen(folderPath) * sizeof(char));

    strncat(settings->eventsOutPath, "/events.json", eventsOutSize * sizeof(char));
    strncat(settings->objOutPath, "/objects.json", objOutSize * sizeof(char));

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
        printf("[ABD_TOOL] Loading settings... ");
        if ((settingsFile = openSetFile()) != NO_PATH)
        {
            if (!load(settingsFile))
            {
                puts("ERROR...");
                printf("[ABD_TOOL] Creating default settings... ");
                createDefaults(settingsFile);
            }

            puts("DONE...\n");
            closeSetFile(settingsFile);
        }
    }
}

void checkSettings()
{
    if (settings == NO_PATH)
        loadSettings();
}

char *getObjPath()
{
    return settings->objOutPath;
}
char *getEventsPath()
{
    return settings->eventsOutPath;
}