/*
   OpenWeatherOneCall.cpp v3.3.0
   copyright 2020 - Jessica Hershey
   www.github.com/JHershey69

   Open Weather Map - Weather Conditions
   For ESP32 Only
   Viva La Resistance

   REVISION HISTORY
   See User Manual
*/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "OpenWeatherMap.h"

void dateTimeConversion(long _epoch, char *_buffer, int _format)
{

    /*FORMAT RETURNS
    1 M/D/Y 24H
    2 D/M/Y 24H
    3 M/D/Y 12H
    4 D/M/Y 12H
    5/6 TIME ONLY 24H
    7/8 TIME ONLY 12H
    9 DAY SHORTNAME
    */

    time_t rawtime = _epoch;
    struct tm *ptm = localtime(&rawtime);
    switch (_format)
    {

    case 1:
        // M/D/Y 24H
        strftime(_buffer, 20, "%m/%d/%Y %R", ptm);
        break;
    case 2:
        // D/M/Y 24H
        strftime(_buffer, 20, "%d/%m/%Y %R", ptm);
        break;
    case 3:
        // M/D/Y 12H
        strftime(_buffer, 20, "%m/%d/%Y %r", ptm);
        break;
    case 4:
        // D/M/Y 12H
        strftime(_buffer, 20, "%d/%m/%Y %r", ptm);
        break;
    case 5:
    case 6:
        // 24 HOUR TIME ONLY
        strftime(_buffer, 20, "%R", ptm);
        break;
    case 7:
    case 8:
        // 12 HOUR TIME ONLY
        strftime(_buffer, 20, "%r", ptm);
        break;
    case 9:
        // DAY SHORTNAME
        strftime(_buffer, 20, "%a", ptm);
        break;
    default:
        // M/D/Y 24H
        strftime(_buffer, 20, "%m/%d/%Y %R", ptm);
    }
}

OpenWeatherOneCall::OpenWeatherOneCall()
{
}

// For Normal Weather calls *************
#define DS_URL1 "https://api.openweathermap.org/data/2.5/onecall"
char DS_URL2[100];
#define DS_URL3 "&appid="

// For Air Quality calls current *************
#define AQ_URL1 "https://api.openweathermap.org/data/2.5/air_pollution?lat="
#define AQ_URL2 "&lon="
#define AQ_URL3 "&appid="

// For Historical Weather Calls **********
#define TS_URL1 "https://api.openweathermap.org/data/2.5/onecall/timemachine"
#define TS_URL2 "&dt="

// For CITY Id calls
#define CI_URL1 "https://api.openweathermap.org/data/2.5/weather?id="
#define CI_URL2 "&appid="

#define SIZEOF(a) sizeof(a) / sizeof(*a)

// Main Method for Weather API Call and Parsing
int OpenWeatherOneCall::parseWeather(void)
{
    int error_code = 0;

    if (WiFi.status() != WL_CONNECTED)
    {
        return 25;
    }
    error_code = createCurrent();
    error_code = createAQ();
    return error_code;
}

// int OpenWeatherOneCall::parseWeather(char *DKEY, char *GKEY, float SEEK_LATITUDE, float SEEK_LONGITUDE, bool SET_UNITS, int CITY_ID, int API_EXCLUDES, int GET_HISTORY)
// {
//     // Legacy calling Method for versions prior to v3.0.0

//     OpenWeatherOneCall::setOpenWeatherKey(DKEY);
//     if (SET_UNITS)
//     {
//         OpenWeatherOneCall::setUnits(METRIC);
//     }
//     OpenWeatherOneCall::setExcl(API_EXCLUDES);
//     OpenWeatherOneCall::setHistory(GET_HISTORY);
//     if (SEEK_LATITUDE && SEEK_LONGITUDE)
//     {
//         OpenWeatherOneCall::setLatLon(SEEK_LATITUDE, SEEK_LONGITUDE);
//     }
//     else if (CITY_ID)
//     {
//         OpenWeatherOneCall::setLatLon(CITY_ID);
//     }
//     else
//         OpenWeatherOneCall::setLatLon();
//     OpenWeatherOneCall::parseWeather();
// }

// int OpenWeatherOneCall::setLatLon(float _LAT, float _LON)
// {
//     int error_code = 0;

//     if (abs(_LAT) <= 90)
//     {
//         USER_PARAM.OPEN_WEATHER_LATITUDE = _LAT;
//         location.LATITUDE = _LAT; // User copy
//     }
//     else
//         error_code += 1;

//     if (abs(_LON) <= 180)
//     {
//         USER_PARAM.OPEN_WEATHER_LONGITUDE = _LON;
//         location.LONGITUDE = _LON; // User copy
//     }
//     else
//         error_code += 2;

//     if (error_code)
//         return error_code;

//     return (EXIT_SUCCESS);
// }

int OpenWeatherOneCall::setLatLon(int _CITY_ID)
{
    USER_PARAM.CITY_ID = _CITY_ID;
    return (EXIT_SUCCESS);

    int error_code = 0;

    char cityURL[110];
    char *URL1 = "http://api.openweathermap.org/data/2.5/weather?id=";
    char *URL2 = "&appid=";

    sprintf(cityURL, "%s%d%s%s", URL1, _CITY_ID, URL2, USER_PARAM.OPEN_WEATHER_DKEY);
    error_code = parseCityCoordinates(cityURL);
    if (error_code)
        return error_code;

    return (EXIT_SUCCESS);
}

// int OpenWeatherOneCall::setLatLon(void)
// {
//     // IP address of NON-CELLULAR WiFi. Hotspots won't work properly.

//     int error_code = 0;
//     error_code = OpenWeatherOneCall::getIPLocation();
//     if (error_code)
//     {
//         return error_code;
//     }

//     error_code = OpenWeatherOneCall::getIPAPILocation(_ipapiURL);
//     if (error_code)
//     {
//         return error_code;
//     }

//     return 0;
// }

int OpenWeatherOneCall::parseCityCoordinates(char *CTY_URL)
{
    int error_code = 0;

    HTTPClient http;
    http.begin(CTY_URL);
    int httpCode = http.GET();
    if (httpCode > 399)
    {
        if (httpCode == 404)
        {
            http.end();
            error_code += 4;
            return error_code;
        }
        else
        {
            http.end();
            error_code += 5;
            return error_code;
        }
    }
    JsonDocument doc;
    deserializeJson(doc, http.getString());

    if (doc["coord"]["lon"])
    {
        USER_PARAM.OPEN_WEATHER_LONGITUDE = doc["coord"]["lon"]; // -74.2
    }
    else
    {
        error_code += 6;
    }

    if (doc["coord"]["lat"])
    {
        USER_PARAM.OPEN_WEATHER_LATITUDE = doc["coord"]["lat"]; // 39.95
    }
    else
    {
        error_code += 7;
    }

    http.end();

    if (error_code)
    {
        http.end();
        return error_code;
    }

    return (EXIT_SUCCESS);
}

// int OpenWeatherOneCall::getIPLocation()
// {
//     int error_code = 0;

//     HTTPClient http;
//     http.begin("https://api64.ipify.org/ HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");  //https://api.ipify.org

//     int httpCode = http.GET();

//     if (httpCode > 399)
//     {
//         if (httpCode == 404)
//         {
//             http.end();
//             error_code += 8;
//             return error_code;
//         }
//         else
//         {
//             http.end();
//             error_code += 9;
//             return error_code;
//         }
//     }

//     int streamSize = http.getSize();
//     char stringVarout[streamSize + 1];

//     String stringVarin = http.getString();
//     strcpy(stringVarout, stringVarin.c_str());

//     http.end();

//     strncpy(_ipapiURL, "https://ipapi.co/", 38);
//     strcat(_ipapiURL, stringVarout);
//     strcat(_ipapiURL, "/json/");

//     return 0;
// }

// int OpenWeatherOneCall::getIPAPILocation(char *URL)
// {
//     int error_code = 0;
//     HTTPClient http;
//     http.begin(URL);
//     int ipapi_httpCode = http.GET();

//     if (ipapi_httpCode > 399)
//     {
//         if (ipapi_httpCode == 404)
//         {
//             error_code += 10;
//         }
//         else
//         {
//             error_code += 11;
//         }
//         http.end();
//         return error_code;
//     }

//     const size_t capacity = JSON_OBJECT_SIZE(26) + 490;
//     DynamicJsonDocument doc(capacity);

//     deserializeJson(doc, http.getString());

//     strncpy(location.CITY, doc["city"], 60);            // "Lakewood"
//     strncpy(location.STATE, doc["region_code"], 2);     // "NJ"
//     strncpy(location.COUNTRY, doc["country_code"], 3);  // "US"
//     USER_PARAM.OPEN_WEATHER_LATITUDE = doc["latitude"]; // 40.0881
//     location.LATITUDE = doc["latitude"];
//     USER_PARAM.OPEN_WEATHER_LONGITUDE = doc["longitude"]; // -74.1963
//     location.LONGITUDE = doc["longitude"];

//     http.end();

//     return 0;
// }

void OpenWeatherOneCall::initAPI(void)
{
    memset(USER_PARAM.OPEN_WEATHER_DKEY, NULL, 1);
    USER_PARAM.OPEN_WEATHER_LATITUDE = 0.0;
    USER_PARAM.OPEN_WEATHER_LONGITUDE = 0.0;
    USER_PARAM.OPEN_WEATHER_UNITS = 2;
    USER_PARAM.OPEN_WEATHER_EXCLUDES = 0;
    USER_PARAM.OPEN_WEATHER_HISTORY = 0;
}

// int OpenWeatherOneCall::getLocationInfo()
// {
//     int error_code = 0;

//     char locationURL[200];

//     sprintf(locationURL, "https://api.bigdatacloud.net/data/reverse-geocode-client/?latitude=%f&longitude=%f", USER_PARAM.OPEN_WEATHER_LATITUDE, USER_PARAM.OPEN_WEATHER_LONGITUDE);

//     HTTPClient http;
//     http.begin(locationURL); //<------------ Connect to OpenWeatherMap

//     int httpCode = http.GET();
//     if (httpCode > 399) //<- Check for connect errors
//     {
//         if (httpCode == 401)
//         {
//             error_code = 17;
//         }
//         else if (httpCode == 404)
//         {
//             error_code = 18;
//         }
//         else
//             error_code = 19;

//         http.end();
//         return error_code;
//     }

//     // const size_t capacity = JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(2) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 3*JSON_OBJECT_SIZE(6) + 2*JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(14) + 1050;
//     DynamicJsonDocument doc(3072);

//     deserializeJson(doc, http.getString());

//     if (doc["locality"])
//     {
//         strncpy(location.CITY, doc["locality"], 60);
//     }
//     else
//     {
//         http.end();
//         return 20;
//     }

//     if (doc["principalSubdivisionCode"])
//     {
//         strncpy(location.STATE, doc["principalSubdivisionCode"], 10);
//     }
//     else
//     {
//         http.end();
//         return 20;
//     }

//     if (doc["countryCode"])
//     {
//         strncpy(location.COUNTRY, doc["countryCode"], 10);
//     }
//     else
//     {
//         http.end();
//         return 20;
//     }

//     http.end();

//     return 0;
// }

// int OpenWeatherOneCall::createHistory()
// {
//     int error_code = 0;
//     char historyURL[200] = {0};

//     if (USER_PARAM.OPEN_WEATHER_HISTORY > 5)
//     {
//         return 16;
//     }

//     // Gets Timestamp for EPOCH calculation below
//     char tempURL[200];
//     sprintf(tempURL, "https://api.openweathermap.org/data/2.5/onecall?lat=%.6f&lon=%.6f&exclude=minutely,hourly,daily,alerts&units=IMPERIAL&appid=%s", USER_PARAM.OPEN_WEATHER_LATITUDE, USER_PARAM.OPEN_WEATHER_LONGITUDE, USER_PARAM.OPEN_WEATHER_DKEY);

//     HTTPClient http;
//     http.begin(tempURL);
//     int httpCode = http.GET();

//     if (httpCode > 399)
//     {
//         if (httpCode == 401)
//         {
//             http.end();
//             return 22;
//         }
//         else
//         {
//             http.end();
//             return 21;
//         }
//     }

//     DynamicJsonDocument toc(768);
//     deserializeJson(toc, http.getString());

//     JsonObject toc_current = toc["current"];
//     long tempEPOCH = toc_current["dt"]; // 1608323864

//     tempEPOCH = tempEPOCH - (86400 * USER_PARAM.OPEN_WEATHER_HISTORY);

//     http.end();

//     // Timemachine request to OWM
//     sprintf(historyURL, "%s?lat=%.6f&lon=%.6f%s%ld&units=%s%s%s", TS_URL1, USER_PARAM.OPEN_WEATHER_LATITUDE, USER_PARAM.OPEN_WEATHER_LONGITUDE, TS_URL2, tempEPOCH, units, DS_URL3, USER_PARAM.OPEN_WEATHER_DKEY);

//     http.begin(historyURL);
//     httpCode = http.GET();

//     if (httpCode > 399)
//     {
//         if (httpCode == 401)
//         {
//             http.end();
//             return 22;
//         }
//         else
//         {
//             http.end();
//             return 21;
//         }
//     }

//     DynamicJsonDocument doc(9000);

//     deserializeJson(doc, http.getString());

//     strncpy(location.timezone, doc["timezone"], 50);
//     location.timezoneOffset = doc["timezone_offset"];

//     if (!history)
//     {
//         history = (struct HISTORICAL *)calloc(25, sizeof(struct HISTORICAL));
//     }

//     // Current in historical is the time of the request on that day
//     JsonObject current = doc["current"];
//     history[0].dayTime = current["dt"]; // 1607292481

//     if (current["dt"])
//     {
//         long tempTime = current["dt"];
//         tempTime += location.timezoneOffset;
//         dateTimeConversion(tempTime, history[0].readableDateTime, USER_PARAM.OPEN_WEATHER_DATEFORMAT);
//     }

//     history[0].sunrise = current["sunrise"]; // 1607256309

//     if (current["sunrise"])
//     {
//         long tempTime = current["sunrise"];
//         tempTime += location.timezoneOffset;
//         dateTimeConversion(tempTime, history[0].readableSunrise, USER_PARAM.OPEN_WEATHER_DATEFORMAT + 4);
//     }

//     history[0].sunset = current["sunset"]; // 1607290280

//     if (current["sunset"])
//     {
//         long tempTime = current["sunset"];
//         tempTime += location.timezoneOffset;
//         dateTimeConversion(tempTime, history[0].readableSunset, USER_PARAM.OPEN_WEATHER_DATEFORMAT + 4);
//     }

//     history[0].temperature = current["temp"];               // 35.82
//     history[0].apparentTemperature = current["feels_like"]; // 21.7
//     history[0].pressure = current["pressure"];              // 1010
//     history[0].humidity = current["humidity"];              // 51
//     history[0].dewPoint = current["dew_point"];             // 20.88
//     history[0].uvIndex = current["uvi"];                    // 1.54
//     history[0].cloudCover = current["clouds"];              // 1
//     history[0].visibility = current["visibility"];          // 16093
//     history[0].windSpeed = current["wind_speed"];           // 16.11
//     history[0].windBearing = current["wind_deg"];           // 300
//     history[0].windGust = current["wind_gust"];             // 24.16

//     // New rain and snow =======================
//     if (current["rain"])
//     {
//         if (USER_PARAM.OPEN_WEATHER_UNITS == 2)
//         {
//             float temp = current["rain"]["1h"];
//             history[0].rainVolume = (temp / 25.4); // 95
//         }
//         else
//             history[0].rainVolume = current["rain"]["1h"]; // 95
//     }
//     else
//         history[0].rainVolume = 0;

//     if (current["snow"])
//     {
//         if (USER_PARAM.OPEN_WEATHER_UNITS == 2)
//         {
//             float temp = current["snow"]["1h"];
//             history[0].snowVolume = (temp / 25.4); // 95
//         }
//         else
//             history[0].snowVolume = current["snow"]["1h"]; // 95
//     }
//     else
//         history[0].snowVolume = 0;

//     JsonObject current_weather_0 = current["weather"][0];
//     history[0].id = current_weather_0["id"]; // 800

//     history[0].main = (char *)realloc(history[0].main, sizeof(char) * strlen(current_weather_0["main"]) + 1);
//     strncpy(history[0].main, current_weather_0["main"], strlen(current_weather_0["main"]) + 1);

//     history[0].summary = (char *)realloc(history[0].summary, sizeof(char) * strlen(current_weather_0["description"]) + 1);
//     strncpy(history[0].summary, current_weather_0["description"], strlen(current_weather_0["description"]) + 1);

//     strncpy(history[0].icon, current_weather_0["icon"], strlen(current_weather_0["icon"]) + 1);

//     dateTimeConversion(history[0].dayTime, history[0].weekDayName, 9);

//     // Hourly report for the day requested begins at 00:00GMT
//     JsonArray hourly = doc["hourly"];

//     for (int x = 1; x < 24; x++)
//     {
//         JsonObject hourly_0 = hourly[x - 1];
//         history[x].dayTime = hourly_0["dt"]; // 1607212800

//         if (hourly_0["dt"])
//         {
//             long tempTime = hourly_0["dt"];
//             tempTime += location.timezoneOffset;
//             dateTimeConversion(tempTime, history[x].readableDateTime, USER_PARAM.OPEN_WEATHER_DATEFORMAT);
//         }

//         history[x].sunrise = current["sunrise"] | 1607256309; // 1607256309

//         if (current["sunrise"])
//         {
//             long tempTime = current["sunrise"];
//             tempTime += location.timezoneOffset;
//             dateTimeConversion(tempTime, history[x].readableSunrise, USER_PARAM.OPEN_WEATHER_DATEFORMAT + 4);
//         }

//         history[x].sunset = current["sunset"] | 1607290280; // 1607290280

//         if (current["sunset"])
//         {
//             long tempTime = current["sunset"];
//             tempTime += location.timezoneOffset;
//             dateTimeConversion(tempTime, history[x].readableSunset, USER_PARAM.OPEN_WEATHER_DATEFORMAT + 4);
//         }

//         history[x].temperature = hourly_0["temp"];               // 40.21
//         history[x].apparentTemperature = hourly_0["feels_like"]; // 29.39
//         history[x].pressure = hourly_0["pressure"];              // 1007
//         history[x].humidity = hourly_0["humidity"];              // 56
//         history[x].dewPoint = hourly_0["dew_point"];             // 26.51
//         history[x].cloudCover = hourly_0["clouds"];              // 1
//         history[x].visibility = hourly_0["visibility"];          // 16093
//         history[x].windSpeed = hourly_0["wind_speed"];           // 11.41
//         history[x].windBearing = hourly_0["wind_deg"];           // 290

//         // New rain and snow =======================
//         if (hourly_0["rain"])
//         {
//             if (USER_PARAM.OPEN_WEATHER_UNITS == 2)
//             {
//                 float temp = hourly_0["rain"]["1h"];
//                 history[x].rainVolume = (temp / 25.4); // 95
//             }
//             else
//                 history[x].rainVolume = hourly_0["rain"]["1h"]; // 95
//         }
//         else
//             history[x].rainVolume = 0;

//         if (hourly_0["snow"])
//         {
//             if (USER_PARAM.OPEN_WEATHER_UNITS == 2)
//             {
//                 float temp = hourly_0["snow"]["1h"];
//                 history[x].snowVolume = (temp / 25.4); // 95
//             }
//             else
//                 history[x].snowVolume = hourly_0["snow"]["1h"]; // 95
//         }
//         else
//             history[x].snowVolume = 0;

//         JsonObject hourly_0_weather_0 = hourly_0["weather"][0];
//         history[x].id = hourly_0_weather_0["id"]; // 800

//         history[x].main = (char *)realloc(history[x].main, sizeof(char) * strlen(hourly_0_weather_0["main"]) + 1);
//         strncpy(history[x].main, hourly_0_weather_0["main"], strlen(hourly_0_weather_0["main"]) + 1);

//         history[x].summary = (char *)realloc(history[x].summary, sizeof(char) * strlen(hourly_0_weather_0["description"]) + 1);
//         strncpy(history[x].summary, hourly_0_weather_0["description"], strlen(hourly_0_weather_0["description"]) + 1);

//         strncpy(history[x].icon, hourly_0_weather_0["icon"], strlen(hourly_0_weather_0["icon"]) + 1);

//         dateTimeConversion(history[0].dayTime, history[x].weekDayName, 9);
//     }

//     http.end();
//     return 0;
// }

int OpenWeatherOneCall::createAQ()
{
    char getURL[200] = {0};

    sprintf(getURL, "%s%.6f%s%.6f%s%s", AQ_URL1, USER_PARAM.OPEN_WEATHER_LATITUDE, AQ_URL2, USER_PARAM.OPEN_WEATHER_LONGITUDE, AQ_URL3, USER_PARAM.OPEN_WEATHER_DKEY);

    HTTPClient http;
    http.begin(getURL);
    int httpCode = http.GET();

    if (httpCode > 399)
    {
        if (httpCode == 401)
        {
            http.end();
            return 22;
        }

        http.end();
        return 21;
    }

    String resp = http.getString(); 
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, resp);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return -1;
    }
    doc.shrinkToFit();

    if (!quality)
        quality = (struct airQuality *)calloc(1, sizeof(struct airQuality));

    float coord_lon = doc["coord"]["lon"]; // -74.1975
    float coord_lat = doc["coord"]["lat"]; // 39.9533

    printf("%f %f\n", coord_lon, coord_lat);

    JsonObject list_0 = doc["list"][0];

    quality->aqi = list_0["main"]["aqi"]; // 2

    JsonObject list_0_components = list_0["components"];
    quality->co = list_0_components["co"];       // 230.31
    quality->no = list_0_components["no"];       // 0.43
    quality->no2 = list_0_components["no2"];     // 1.69
    quality->o3 = list_0_components["o3"];       // 100.14
    quality->so2 = list_0_components["so2"];     // 0.88
    quality->pm2_5 = list_0_components["pm2_5"]; // 0.76
    quality->pm10 = list_0_components["pm10"];   // 1.04
    quality->nh3 = list_0_components["nh3"];     // 0.43

    quality->dayTime = list_0["dt"]; // 1615838400

    http.end();
    doc.clear();
    return 0;
}

int OpenWeatherOneCall::createCurrent()
{

    char getURL[200] = {0};
    int alertz = 0;

    sprintf(getURL, "%s%d&lang=zh&units=METRIC%s%s", CI_URL1,  USER_PARAM.CITY_ID, CI_URL2, USER_PARAM.OPEN_WEATHER_DKEY);

    HTTPClient http;
    http.begin(getURL);
    int httpCode = http.GET();

    if (httpCode > 399)
    {
        if (httpCode == 401)
        {
            http.end();
            return 22;
        }

        http.end();
        return 21;
    }

    String resp = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, resp);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return -1;
    }
    doc.shrinkToFit();

    if (doc["coord"]["lon"])
    {
        USER_PARAM.OPEN_WEATHER_LONGITUDE = doc["coord"]["lon"]; // -74.2
    }

    if (doc["coord"]["lat"])
    {
        USER_PARAM.OPEN_WEATHER_LATITUDE = doc["coord"]["lat"]; // 39.95
    }
    if (!current)
    {
        current = (struct nowData *)calloc(1, sizeof(struct nowData));
        if (current == NULL)
        {
            return 23;
        }
    }

    // JsonObject currently = doc["current"];
    current->dayTime = doc["dt"]; // 1586781931

    if (doc["dt"])
    {
        long tempTime = doc["dt"];
        tempTime += location.timezoneOffset;
        dateTimeConversion(tempTime, current->readableDateTime, USER_PARAM.OPEN_WEATHER_DATEFORMAT);
        dateTimeConversion(tempTime, current->readableWeekdayName, 9);
    }

    current->sunriseTime = doc["sys"]["sunrise"]; // 1612267442

    if (doc["sys"]["sunrise"])
    {
        long tempTime = doc["sys"]["sunrise"];
        tempTime += location.timezoneOffset;
        dateTimeConversion(tempTime, current->readableSunrise, USER_PARAM.OPEN_WEATHER_DATEFORMAT + 4);
    }

    current->sunsetTime = doc["sys"]["sunset"]; // 1612304218

    if (doc["sys"]["sunset"])
    {
        long tempTime = doc["sys"]["sunset"];
        tempTime += location.timezoneOffset;
        dateTimeConversion(tempTime, current->readableSunset, USER_PARAM.OPEN_WEATHER_DATEFORMAT + 4);
    }

    current->temperature = doc["main"]["temp"];               // 287.59
    current->apparentTemperature = doc["main"]["feels_like"]; // 281.42
    current->pressure = doc["main"]["pressure"];              // 1011
    current->humidity = doc["main"]["humidity"];              // 93
    current->cloudCover = doc["clouds"]["all"];               // 90
    current->visibility = doc["visibility"];                  // 8047
    current->windSpeed = doc["wind"]["speed"];                // 10.3
    current->windBearing = doc["wind"]["deg"];                // 170

    if (doc["wind"]["gust"])
    {
        current->windGust = doc["wind"]["wind_gust"];
    }

    current->id = doc["weather"][0]["id"];

    current->main = (char *)realloc(current->main, sizeof(char) * strlen(doc["weather"][0]["main"]) + 1);
    if (current->main == NULL)
    {
        return 23;
    }
    strncpy(current->main, doc["weather"][0]["main"], strlen(doc["weather"][0]["main"]) + 1);

    current->summary = (char *)realloc(current->summary, sizeof(char) * strlen(doc["weather"][0]["description"]) + 1);
    if (current->summary == NULL)
    {
        return 23;
    }
    strncpy(current->summary, doc["weather"][0]["description"], strlen(doc["weather"][0]["description"]) + 1);

    strncpy(current->icon, doc["weather"][0]["icon"], strlen(doc["weather"][0]["icon"]) + 1);


    current->cityName = (char *)realloc(current->cityName, sizeof(char) * strlen(doc["name"]) + 1);
    if (current->cityName == NULL)
    {
        return 23;
    }
    strncpy(current->cityName, doc["name"], strlen(doc["name"]) + 1);

    http.end();
    doc.clear();
    return 0;
}

// int OpenWeatherOneCall::setExcludes(int EXCL)
// {
//     unsigned int excludeMemSize = 32768;
//     // Allocate MEM requirements for parser ****************************
//     unsigned int EXCL_SIZES[] = {1010, 3084, 15934, 4351, 607}; // Alerts, Minutely, Hourly, Daily, Currently
//     // Name the exclude options *********************
//     char *EXCL_NAMES[] = {"current", "daily", "hourly", "minutely", "alerts"};

//     exclude.all_excludes = EXCL; //<- Sets the individual bits to 1 for excludes
//     strcpy(DS_URL2, "&exclude=");

//     int SIZE_X = (sizeof(EXCL_NAMES) / sizeof(EXCL_NAMES[0]));

//     // Required to insert comma properly *****************************
//     int comma_pos = exclude.all_excludes;
//     int comma_sub = 16;

//     // Add exclude names to URL and insert a comma *******************
//     for (int x = SIZE_X - 1; x >= 0; x--)
//     {
//         if (exclude.all_excludes >> x & 1)
//         {
//             strcat(DS_URL2, EXCL_NAMES[x]);

//             if ((x == 4) && (comma_pos > 16))
//             {
//                 strcat(DS_URL2, ",");
//                 comma_pos -= comma_sub;
//             }
//             if ((x == 3) && ((comma_pos > 8) && (comma_pos < 16)))
//             {
//                 strcat(DS_URL2, ",");
//                 comma_pos -= comma_sub;
//             }

//             if ((x == 2) && ((comma_pos > 4) && (comma_pos < 8)))
//             {
//                 strcat(DS_URL2, ",");
//                 comma_pos -= comma_sub;
//             }

//             if ((x == 1) && ((comma_pos > 2) && (comma_pos < 4)))
//             {
//                 strcat(DS_URL2, ",");
//                 comma_pos -= comma_sub;
//             }

//             if (x < (sizeof(EXCL_NAMES) / sizeof(EXCL_NAMES[0])) - 1)
//             {
//                 excludeMemSize -= EXCL_SIZES[x];
//             }
//         }
//         comma_sub = comma_sub / 2;
//     }

//     return excludeMemSize;
// }

int OpenWeatherOneCall::setOpenWeatherKey(const char *owKey)
{
    int error_code = 0;
    if ((strlen(owKey) < 25) || (strlen(owKey) > 64))
    {
        return 12;
    };

    strncpy(USER_PARAM.OPEN_WEATHER_DKEY, owKey, 100);

    return 0;
}

// int OpenWeatherOneCall::setExcl(int _EXCL)
// {
//     if ((_EXCL > 31) || (_EXCL < 0))
//     {
//         USER_PARAM.OPEN_WEATHER_EXCLUDES = NULL;
//         return 14;
//     }
//     else
//         USER_PARAM.OPEN_WEATHER_EXCLUDES = _EXCL;

//     return 0;
// }

// int OpenWeatherOneCall::setUnits(int _UNIT)
// {
//     if (_UNIT > 4)
//     {
//         return 15;
//     }
//     USER_PARAM.OPEN_WEATHER_UNITS = _UNIT;

//     switch (_UNIT)
//     {
//     case 3:
//         strcpy(units, "STANDARD");
//         break;
//     case 2:
//         strcpy(units, "IMPERIAL");
//         break;
//     case 1:
//         strcpy(units, "METRIC");
//         break;
//     default:
//         strcpy(units, "IMPERIAL");
//     }

//     return 0;
// }

// int OpenWeatherOneCall::setHistory(int _HIS)
// {
//     if (_HIS > 5)
//     {
//         return 16;
//     }
//     USER_PARAM.OPEN_WEATHER_HISTORY = _HIS;

//     return 0;
// }

// int OpenWeatherOneCall::setDateTimeFormat(int _DTF)
// {
//     if (_DTF > 4)
//         return 16;
//     USER_PARAM.OPEN_WEATHER_DATEFORMAT = _DTF;
//     return 0;
// }

// free routines

void OpenWeatherOneCall::freeCurrentMem(void)
{
    if (current)
    {
        free(current->summary);
        current->summary = NULL;
        free(current->main);
        current->main = NULL;
        free(current->cityName);
        current->cityName = NULL;
        free(current);
        current = NULL;
    }
}

void OpenWeatherOneCall::freeForecastMem(void)
{
    if (forecast)
    {
        for (int x = 8; x > 0; x--)
        {
            free(forecast[x - 1].summary);
            free(forecast[x - 1].main);
        }
        free(forecast);
        forecast[0].summary = NULL;
        forecast[0].main = NULL;
        forecast = NULL;
    }
}

void OpenWeatherOneCall::freeHistoryMem(void)
{
    if (history)
    {
        for (int x = 24; x > 0; x--)
        {
            free(history[x - 1].summary);
            free(history[x - 1].main);
        }
        free(history);
        history[0].summary = NULL;
        history[0].main = NULL;
        history = NULL;
    }
}

void OpenWeatherOneCall::freeAlertMem(void)
{
    if (alert)
    {
        for (int x = MAX_NUM_ALERTS; x > 0; x--)
        {
            free(alert[x].senderName);
            free(alert[x].event);
            free(alert[x].summary);
        }
        free(alert);
        alert[0].senderName = NULL;
        alert[0].event = NULL;
        alert[0].summary = NULL;
        alert = NULL;
    }
}

void OpenWeatherOneCall::freeHourMem(void)
{
    if (hour)
    {
        for (int x = 48; x > 0; x--)
        {
            free(hour[x - 1].summary);
            free(hour[x - 1].main);
        }
        free(hour);
        hour[0].summary = NULL;
        hour[0].main = NULL;
        hour = NULL;
    }
}

void OpenWeatherOneCall::freeMinuteMem(void)
{
    if (minute)
    {
        free(minute);
        minute = NULL;
    }
}

void OpenWeatherOneCall::freeQualityMem(void)
{
    if (quality)
    {
        free(quality);
        quality = NULL;
    }
}

// char *OpenWeatherOneCall::getErrorMsgs(int _errMsg)
// {
//     if ((_errMsg > SIZEOF(errorMsgs)) || (_errMsg < 1))
//         return "Error Number Out of RANGE";
//     strcpy_P(buffer, (char *)pgm_read_dword(&(errorMsgs[_errMsg - 1])));
//     return buffer;
// }

OpenWeatherOneCall::~OpenWeatherOneCall()
{

    OpenWeatherOneCall::freeCurrentMem();
    OpenWeatherOneCall::freeForecastMem();
    OpenWeatherOneCall::freeAlertMem();
    OpenWeatherOneCall::freeHourMem();
    OpenWeatherOneCall::freeMinuteMem();
    OpenWeatherOneCall::freeHistoryMem();
    OpenWeatherOneCall::freeQualityMem();
}

// Looks like this is the end
OpenWeatherOneCall OWOC;