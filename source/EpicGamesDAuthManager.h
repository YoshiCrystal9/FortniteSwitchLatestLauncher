#include <json.hpp>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <optional>
#include <curl/curl.h>
#include <sstream>
#include <switch.h>
using namespace std;



using json = nlohmann::json;
using OptionalJson = std::optional<json>;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *response)
{
    size_t totalSize = size * nmemb;
    response->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}


json GetDAuth() {
    FILE *file = fopen("sdmc:/switch/FortLatestLauncher/auth.json", "r");

    if (file)
    {
        char line[1024]; // Adjust the buffer size as needed

        while (std::fgets(line, sizeof(line), file))
        {
            fclose(file);
            json j = json::parse(line);
            return j;
        }

       
    }
    else
    {
        fclose(file);
        return json();
    }
}

void SaveDAuth(json j) {
    FILE *file = std::fopen("sdmc:/switch/FortLatestLauncher/auth.json", "w");
    std::string s = j.dump();
    fputs(s.c_str(), file);
    fclose(file);
}

void DeleteDAuth() {
    remove("sdmc:/switch/FortLatestLauncher/auth.json");
}

std::string getClientCredentials()
{
    CURL *curl = curl_easy_init();

    if (curl)
    {
        std::string body;
        curl_easy_setopt(curl, CURLOPT_URL, "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "grant_type=client_credentials");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        //curl_easy_setopt(curl, CURLOPT_USERAGENT, "Fortnite/++Fortnite+Release-13.40-CL-14050091 Android/13");

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Authorization: Basic OThmN2U0MmMyZTNhNGY4NmE3NGViNDNmYmI0MWVkMzk6MGEyNDQ5YTItMDAxYS00NTFlLWFmZWMtM2U4MTI5MDFjNGQ3");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            printf("EpicGames request failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            json j = json::parse(body);
            curl_easy_cleanup(curl);
            return j["access_token"];
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    else
    {
        printf("Failed to initialize libcurl\n");
    }
}

json startDauthProcess(std::string accessToken)
{
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
        std::string body;
        struct curl_slist *headers = NULL;

        curl_easy_setopt(curl, CURLOPT_URL, "https://account-public-service-prod03.ol.epicgames.com/account/api/oauth/deviceAuthorization");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        std::string authHeader = "Authorization: Bearer " + accessToken;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            printf("EpicGames request failed: %s\n", curl_easy_strerror(res));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return json();
        }


        json j = json::parse(body);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return j;
    }

    return json();
}

json pollDauth(std::string deviceCode)
{
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
        std::string body;
        struct curl_slist *headers = NULL;

        curl_easy_setopt(curl, CURLOPT_URL, "https://account-public-service-prod03.ol.epicgames.com/account/api/oauth/token");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        std::string postData = "grant_type=device_code&device_code=" + deviceCode;
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        //curl_easy_setopt(curl, CURLOPT_USERAGENT, "Fortnite/++Fortnite+Release-13.40-CL-14050091 Switch/17");

        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Authorization: Basic OThmN2U0MmMyZTNhNGY4NmE3NGViNDNmYmI0MWVkMzk6MGEyNDQ5YTItMDAxYS00NTFlLWFmZWMtM2U4MTI5MDFjNGQ3");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        consoleUpdate(NULL);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return json(); 
        }

        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode != 200)
        {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return json(); 
        }

        json j = json::parse(body);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return j;
    }

    return json();
}


string getAccessToken(json DauthResponse) {
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
        std::string body;
        struct curl_slist *headers = NULL;

        std::string accountId = DauthResponse["accountId"].get<std::string>();


        std::string url = "https://account-public-service-prod03.ol.epicgames.com/account/api/oauth/token";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        std::string postBody = "grant_type=device_auth&account_id=" + accountId + "&device_id=" + DauthResponse["deviceId"].get<std::string>() + "&secret=" + DauthResponse["secret"].get<std::string>();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postBody.c_str());

        headers = curl_slist_append(headers, "Authorization: Basic OThmN2U0MmMyZTNhNGY4NmE3NGViNDNmYmI0MWVkMzk6MGEyNDQ5YTItMDAxYS00NTFlLWFmZWMtM2U4MTI5MDFjNGQ3");
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            //printf("EpicGames request failed: %s\n", curl_easy_strerror(res));
            json errorJson = json::parse(body);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            if (errorJson["numericErrorCode"].get<int>() == 18031)
            {

                return "INVALID_DEVICE_AUTH";
            }
            return ""; 
        }

        long responseCode;

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode != 200)
        {
            printf("EpicGames request failed: %s\n", curl_easy_strerror(res));
            printf("Response code: %ld\n", responseCode);
            printf("Response body: %s\n", body.c_str());

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return "";
        }

        json j = json::parse(body);

        curl_slist_free_all(headers);

        curl_easy_cleanup(curl);
        return j["access_token"];
    } else {
        return "";
    }
}

string getExchangeCode(json DauthResponse) {
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
        std::string body;
        struct curl_slist *headers = NULL;

        std::string accountId = DauthResponse["accountId"];

        std::string url = "https://account-public-service-prod03.ol.epicgames.com/account/api/oauth/exchange";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        std::string accessToken = getAccessToken(DauthResponse);
        if (accessToken == "INVALID_DEVICE_AUTH")
        {
            return "INVALID_DEVICE_AUTH";
        }
        std::string authBearer = "Authorization: Bearer " + accessToken;
        headers = curl_slist_append(headers, authBearer.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            printf("EpicGames request failed: %s\n", curl_easy_strerror(res));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return ""; 
        }

        long responseCode;

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode != 200)
        {
            printf("EpicGames request failed: %s\n", curl_easy_strerror(res));
            printf("Response code: %ld\n", responseCode);
            printf("Response body: %s\n", body.c_str());

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return ""; 
        }

        json j = json::parse(body);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return j["code"];
    } else {
        return "";
    }

}

json getDeviceAuth(json DAuthResponse) {
    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
        std::string body;
        struct curl_slist *headers = NULL;
        std::string accountId = DAuthResponse["account_id"];

        std::string url = "https://account-public-service-prod03.ol.epicgames.com/account/api/public/account/" + accountId + "/deviceAuth";
        printf("URL: %s\n", url.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        //curl_easy_setopt(curl, CURLOPT_USERAGENT, "Fortnite/++Fortnite+Release-13.40-CL-14050091 Switch/17");

        headers = curl_slist_append(headers, ("Authorization: Bearer " + DAuthResponse["access_token"].get<std::string>()).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        consoleUpdate(NULL);
        res = curl_easy_perform(curl);


        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode != 200)
        {
            printf("EpicGames request failed: %s\n", curl_easy_strerror(res));
            printf("Response code: %ld\n", responseCode);
            printf("Response body: %s\n", body.c_str());

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return json(); 
        }

        json j = json::parse(body);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return j;
    }

    return json();
}

void InitializeAuthProcess()
{
    printf("Initializing auth process...\n");

    std::string clientCredentialsToken = getClientCredentials();

    json dauthInit = startDauthProcess(clientCredentialsToken);

    if (dauthInit.empty())
    {
        printf("Failed to start DAuth process.\n");
        return;
    }


    std::string deviceCode = dauthInit["device_code"];
    std::string verificationUri = dauthInit["verification_uri_complete"];

    printf("Get a device with an internet connection and go to %s to authenticate to your Epic Games account.\n", verificationUri.c_str());
    consoleUpdate(NULL);

    while (true)
    {
        json dauthPoll = pollDauth(deviceCode);

        if (!dauthPoll.empty())
        {
            printf("Authenticated!\n");
            json dauth = getDeviceAuth(dauthPoll);
            if (dauth.empty())
            {
                printf("Failed to get device auth.\n");
                return;
            }
            dauth["displayName"] = dauthPoll["displayName"];
            SaveDAuth(dauth);
            printf("Saved auth info. Welcome %s!\n", dauthPoll["displayName"].get<std::string>().c_str());
            consoleUpdate(NULL);
            sleep(3);
            break;
        }

        consoleUpdate(NULL);
        sleep(1);
    }
}
