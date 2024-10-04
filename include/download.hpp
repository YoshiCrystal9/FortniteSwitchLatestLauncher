#ifndef _DOWNLOAD_H_
#define _DOWNLOAD_H_

#define ON              1
#define OFF             0

#define SIGPATCHES_URL "https://github.com/YoshiCrystal9/FortPatcher-NX/releases/latest/download/all_patches.zip"
#define DOWNLOAD_PATH "/config/FortLatestLauncher/"
#define SIGPATCHES_FILENAME "/config/FortLatestLauncher/stuff.zip"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <curl/curl.h>

#include <string>
#include <regex>
#include <iostream>

#include <switch.h>


bool downloadFile(const char *url, const char *output, int api);

std::tuple<std::vector<std::string>, std::vector<std::string>> fetchLinks(const char *url);

#endif