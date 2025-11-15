// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdbool.h>
#include <sys/stat.h>
#include <switch.h>
#include <dirent.h>
#include <algorithm>

#include "EpicGamesDAuthManager.h"
#include "UE4CommandLineManager.h"

// Include the main libnx system header, for Switch development
#include <switch.h>
#include <unistd.h>
#include <curl/curl.h>

#define VERSION "1.0.2" 

#define TRACE(fmt, ...)                                          \
    printf("%s: " fmt "\n", __PRETTY_FUNCTION__, ##__VA_ARGS__); \
    consoleUpdate(NULL);

bool HasConnection()
{
    NifmInternetConnectionStatus status;

    nifmGetInternetConnectionStatus(nullptr, nullptr, &status);

    return status == NifmInternetConnectionStatus_Connected;
}

// lol
std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void CheckForUpdates() {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if(curl) {
        printf("Checking for updates...\n\n");
        consoleUpdate(NULL);
        std::string body;
        curl_easy_setopt(curl, CURLOPT_URL, "https://raw.githubusercontent.com/AntogamerYT/SwitchS13Launcher/main/version");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        consoleUpdate(NULL);
        res = curl_easy_perform(curl);

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if(response_code == 200) {
            int localVersion = atoi(ReplaceAll(VERSION, ".", "").c_str());
            int remoteVersion = atoi(ReplaceAll(body, ".", "").c_str());

            if (localVersion < remoteVersion) {
                printf("There's a new version available! Please download the new release from GitHub.\n\n");
                consoleUpdate(NULL);
            }
        } else {
            printf("Couldn't check for updates. (Request is not okay)\n\n");
            consoleUpdate(NULL);
        }
        curl_easy_cleanup(curl);
    } else {
        printf("Couldn't check for updates. (Curl handle creation failed)\nYou can ignore this, but make sure that this is the latest version of the launcher!\n\n");
        consoleUpdate(NULL);
    }

}

void printDialog(bool actionCancelled, string message = "", bool clear = true) {
    if (clear)
        consoleClear();
    if (actionCancelled)
    {
        printf("Action cancelled.\n\n");
    }

    if (message != "")
    {
        printf("%s\n\n", message.c_str());
    }

    printf("-----------------FORTNITE LATEST LAUNCHER-----------------\n");
    printf("Version %s\n", VERSION);
    printf("Made by thisisanto and edited to work with latest by YoshiCrystal <3\n\n");

    printf("A - Manage your account\nB - Launch Fortnite\nX - Restore CommandLine arguments\n+ - Exit\n\n");
    consoleUpdate(NULL);
}


int main(int argc, char* argv[])
{
    consoleInit(NULL);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);


    PadState pad;
    padInitializeDefault(&pad);

    socketInitializeDefault();
    nifmInitialize(NifmServiceType_User);

    if (!HasConnection())
    {
        printf("You need an internet connection to use this launcher.\n");
        printf("Press + to exit.\n");
        while (appletMainLoop())
        {
            padUpdate(&pad);
            u64 kDown = padGetButtonsDown(&pad);
            if (kDown & HidNpadButton_Plus)
                break;
            consoleUpdate(NULL);
        }
        socketExit();
        nifmExit();
        consoleExit(NULL);
        return 0;
    }
    DIR *dir = opendir("sdmc:/switch/FortLatestLauncher");
    if(!dir) {
        mkdir("sdmc:/switch/FortLatestLauncher", 0777);
    }
    closedir(dir);

    FILE *file = fopen("sdmc:/atmosphere/contents/010025400AECE000/romfs/UECommandLine.txt", "r");

    if (!file) {
        fclose(file);
        file = fopen("sdmc:/atmosphere/contents/010025400AECE000/romfs/UECommandLine.txt", "w");
        fprintf(file, "../../../FortniteGame/FortniteGame.uproject -skippatchcheck");
    }
    fclose(file);

    //CheckForUpdates();

    printDialog(false, "", false);

    while (appletMainLoop())
    {
        padUpdate(&pad);

        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break; 

        if (kDown & HidNpadButton_A) {
            json dauth = GetDAuth();

            if(dauth.empty()) {
                printf("You're not authenticated to the Epic Games services yet. Do you want to authenticate?\nA - Yes\nB - No\n\n");
                printf("NOTE: This will require you to have access to another device with an internet connection.\n\n");
                consoleUpdate(NULL);
                while (appletMainLoop())
                {
                    padUpdate(&pad);

                    u64 kDown = padGetButtonsDown(&pad);

                    if (kDown & HidNpadButton_Plus)
                        break; 

                    if (kDown & HidNpadButton_A) {
                        InitializeAuthProcess();
                        printDialog(false);
                        break;
                    }

                    if (kDown & HidNpadButton_B) {
                        consoleClear();
                        printDialog(true);
                        break;
                    }

                    consoleUpdate(NULL);
                }
            } else {
                printf("You're currently authenticated as %s.\n\n", dauth["displayName"].get<std::string>().c_str());
                printf("A - Reauthenticate\nB - Delete Credentials\n+ - Exit\n\n");
                consoleUpdate(NULL);
                while (appletMainLoop())
                {
                    padUpdate(&pad);

                    u64 kDown = padGetButtonsDown(&pad);

                    if (kDown & HidNpadButton_Plus) {
                        printDialog(false);
                        break; 
                    }

                    if (kDown & HidNpadButton_A) {
                        InitializeAuthProcess();
                        printDialog(false);
                        break;
                    }

                    if (kDown & HidNpadButton_B) {
                        printf("Are you sure you want to delete your credentials?\nA - Yes\nB - No\n\n");

                        while (appletMainLoop())
                        {
                            padUpdate(&pad);

                            u64 kDown = padGetButtonsDown(&pad);

                            if (kDown & HidNpadButton_Plus)
                                break; 

                            if (kDown & HidNpadButton_A) {
                                DeleteDAuth();
                                printDialog(false, "Credentials deleted.");
                                break;
                            }

                            if (kDown & HidNpadButton_B) {
                                consoleClear();
                                printDialog(true);
                                break;
                            }

                            consoleUpdate(NULL);
                        }
                        break;
                    }

                    consoleUpdate(NULL);
                }
            }
        }

        if (kDown & HidNpadButton_B) {
            json dauth = GetDAuth();
            if (dauth.empty()) {
                printDialog(false, "You need to authenticate first.");
                continue;
            }

            std::unordered_map<string, string> arguments = ParseUE4CommandLine("sdmc:/atmosphere/contents/010025400AECE000/romfs/UECommandLine.txt");

            if (arguments["AUTH_TYPE"] != "exchangecode") 
                storeOldUE4CommandLine(arguments);

            printf("Authenticating with Epic Games services...\n\n");
            consoleUpdate(NULL);
            std::string exchangeCode = getExchangeCode(dauth);

            if (exchangeCode == "INVALID_DEVICE_AUTH") {
                printDialog(false, "Your credentials are invalid. Please reauthenticate.");
                continue;
            }

            arguments["AUTH_PASSWORD"] = exchangeCode;
            arguments["AUTH_LOGIN"] = "unused";
            arguments["AUTH_TYPE"] = "exchangecode";
            arguments["AuthClient"] = "3f69e56c7649492c8cc29f1af08a8a12";
            arguments["AuthSecret"] = "b51ee9cb12234f50a69efa67ef53812e";

            std::string commandLine = RebuildUE4CommandLine(arguments);

            SaveUE4CommandLine(commandLine);

            printf("Launching Fortnite, you will now be prompted to select a user...\n\n");

            consoleUpdate(NULL);

            sleep(2);

            appletRequestLaunchApplication(0x010025400AECE000, NULL);
        }

        if (kDown & HidNpadButton_X) {
            std::unordered_map<string, string> arguments = ParseUE4CommandLine("sdmc:/switch/FortLatestLauncher/OldCommandLine.txt");

            if (arguments["failedtoopen"].empty())
            {

                SaveUE4CommandLine(RebuildUE4CommandLine(arguments));

                remove("sdmc:/switch/FortLatestLauncher/OldCommandLine.txt");

                printDialog(false, "CommandLine arguments restored.");
            }
            else
            {
                printDialog(false, "You don't have any old CommandLine arguments to restore.");
            }

        }

        consoleUpdate(NULL);
    }


    socketExit();
    nifmExit();
    consoleExit(NULL);
    return 0;
}
