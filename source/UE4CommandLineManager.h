#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <regex>
#include <switch.h>
using namespace std;

std::string RebuildUE4CommandLine(std::unordered_map<string, string> arguments)
{
    std::string commandLine = "../../../FortniteGame/FortniteGame.uproject ";
    for (auto &argument : arguments)
    {

        commandLine += (argument.second != "" ? ("-" + argument.first + "=" + argument.second + " ") : ("-" + argument.first + " "));
    }
    return commandLine;
}

regex argsRegex("-([A-Za-z_]+)(=(.*))?");

std::unordered_map<string, string> ParseUE4CommandLine(const std::string &filePath)
{
    consoleUpdate(NULL);
    std::unordered_map<string, string> arguments;
    FILE *file = std::fopen(filePath.c_str(), "r");

    if (file)
    {
        char line[1024]; // Adjust the buffer size as needed

        while (std::fgets(line, sizeof(line), file))
        {

            char *token = strtok(line, " \t\n");

            while (token)
            {
                std::smatch match;
                consoleUpdate(NULL);
                std::string tokenString(token);
                if (std::regex_match(tokenString, match, argsRegex))
                {
                    arguments[match[1]] = match[3];
                }
                consoleUpdate(NULL);

                token = strtok(NULL, " \t\n");
            }

            token = strtok(NULL, " \t\n");
        }

        std::fclose(file);
    }
    else
    {
        arguments["failedtoopen"] = "true";
        return arguments;
    }

    return arguments;
}

void SaveUE4CommandLine(std::string arguments)
{
    FILE *file = std::fopen("sdmc:/atmosphere/contents/010025400AECE000/romfs/UECommandLine.txt", "w");
    if (file)
    {
        std::fprintf(file, arguments.c_str());
        std::fclose(file);
    }
    else
    {
        std::cerr << "Failed to open the UECommandLine.txt file." << std::endl;
    }
}

void storeOldUE4CommandLine(std::unordered_map<string, string> arguments)
{
    std::string commandLine = RebuildUE4CommandLine(arguments);
    FILE *file = std::fopen("sdmc:/switch/FortLatestLauncher/OldCommandLine.txt", "w");
    if (file)
    {
        std::fprintf(file, commandLine.c_str());
        std::fclose(file);
    }
    else
    {
        std::cerr << "Failed to open the OldCommandLine.txt file." << std::endl;
    }
}
