
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <curl/curl.h>
#include "get_gps_data.h"


typedef struct {
    double GPS_lat;
    double GPS_lon;
} DMS_Coordinates;

typedef struct {
    double lat;
    double lon;
} Coordinates;

typedef struct {
    Coordinates center;
    double minLat;
    double maxLat;
    double minLon;
    double maxLon;
} Zone;

typedef struct {
    char icao24[7];
    char callsign[9];
    char originCountry[20];
    int lastPositionUpdate;
    int lastContact;
    double longitude;
    double latitude;
    double baroAltitude;
    int isOnGround;
    double velocityOverGround;
    double heading;
    double verticalRate;
    int sensorSerials;
    double geoAltitude;
    char squawk[5];
    int isSpi;
    int positionSource;
} Aircraft;

Coordinates DMSToDecimal(const DMS_Coordinates* dms_coordinates) {
    double lat_degrees = floor(dms_coordinates->GPS_lat / 100);
    double lon_degrees = floor(dms_coordinates->GPS_lon / 100);
    double lat_minutes = dms_coordinates->GPS_lat - (lat_degrees * 100);
    double lon_minutes = dms_coordinates->GPS_lon - (lon_degrees * 100);
    Coordinates decimal_coords;
    decimal_coords.lat = lat_degrees + (lat_minutes / 60.0);
    decimal_coords.lon = lon_degrees + (lon_minutes / 60.0);
    return decimal_coords;
}


Coordinates NMEA_parser(const char* nmea) {
    DMS_Coordinates dms_coords;
    // $GPRMC, 184154.000, A, 4848.8444, N, 00223.7085, E, 0. 17,287.70, 310523, , , A*6C
    sscanf(nmea, "$GPRMC,%*[^,], A,%lf, N,%lf,%*[^,]", &dms_coords.GPS_lat, &dms_coords.GPS_lon);
    Coordinates coords = DMSToDecimal(&dms_coords);
    printf("Current position : \nlat: %lf, lon: %lf\n", coords.lat, coords.lon);
    return coords;
}


Zone getZone(const Coordinates* center, double distance) {
    Zone zone;
    const double EARTH_RADIUS = 6371.0;
    double distanceRad = distance / EARTH_RADIUS;
    double centerLatRad = center->lat * M_PI / 180.0;

    // Calcul des valeurs de latitude minimale et maximale
    zone.minLat = center->lat - (distanceRad * 180.0 / M_PI);
    zone.maxLat = center->lat + (distanceRad * 180.0 / M_PI);

    // Calcul des valeurs de longitude minimale et maximale
    zone.minLon = center->lon - (distanceRad * 180.0 / M_PI) / cos(centerLatRad);
    zone.maxLon = center->lon + (distanceRad * 180.0 / M_PI) / cos(centerLatRad);
    printf("minLat : %lf, maxLat : %lf, minLon : %lf, maxLon : %lf\n", zone.minLat, zone.maxLat, zone.minLon, zone.maxLon);

    return zone;
}

size_t handleResponse(char *ptr, size_t size, size_t nmemb, char *data) //récupère les données reçues, obligatoire.
{   size_t total_size = size * nmemb;
    strncat(data, ptr, total_size);
    return total_size;
}

void urlGenerator(const Zone* zone, char* url){
  sprintf(url, "https://opensky-network.org/api/states/all?lamin=%lf&lomin=%lf&lamax=%lf&lomax=%lf", zone->minLat, zone->minLon, zone->maxLat, zone->maxLon);
  printf("%s\n",url);
}

void performHttpRequest(char *response_data, const char* url){
  CURL *curl;
	CURLcode res;
  char data[10000];
	curl = curl_easy_init();
	if (curl){
  	curl_easy_setopt(curl, CURLOPT_URL, url); //Spécifie l'URL
  	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
  	res = curl_easy_perform(curl); //Effectue la requète
  	curl_easy_cleanup(curl);
	}
  strcpy(response_data, data);

  /*
  Exemple : {"time":1685608033,"states":[["405456","BAW604P ","United Kingdom",1685608033,1685608033,2.3532,48.8418,11277.6,false,222.13,137.63,0,null,11590.02,"5230",false,0]]}
  Response format :
  * Time :
  * Time since epoch
  * States:
  * [
  * 0 icao24: string,
  * 1 callsign: string,
  * 2 originCountry: string,
  * 3 lastPositionUpdate: int,
  * 4 lastContact: int,
  * 5 longitude: float #.####,
  * 6 latitude: float #.####,
  * 7 baroAltitude: float #.##
  * 8 isOnGround: bool,
  * 9 velocityOverGround: float #.##,
  * 10 heading: float #.##,
  * 11 verticalRate: float #.##,
  * 12 sensorSerials: Array[int],
  * 13 geoAltitude: float #.##,
  * 14 squawk: string,
  * 15 isSpi: bool,
  * 16 positionSource: int 0 for ADS-B, 1 for Asterix, 2 for MLAT
  * ]
    */
}

Aircraft aircraftSelector(char* response_data);


int main(void){
  const char* nmea = "$GPRMC, 184154.000, A, 4848.8444, N, 00223.7085, E, 0. 17,287.70, 310523, , , A*6C";
  GetGPSData(nmea);
  Coordinates center =  NMEA_parser(nmea);
  Zone zone = getZone(&center,20);
  char url[512];
  urlGenerator(&zone,url);
  char response_data[10000];
  performHttpRequest(response_data, url);
  printf("%s\n",response_data);
	return 0;
}
