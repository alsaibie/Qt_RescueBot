#ifndef GPSCONVERSION_H
#define GPSCONVERSION_H
#include <iostream>
#include <string>
#include <math.h>
#include <QtCore>
/*
 Input:
    Lat: WGS84 Latitude dd mm.mmmm
    Lon: WGS84 Longitude ddd mm.mmm
 Output:
    x: UTM easting in meters
    y: UTM northing in meters
    utmzone: UTM longitudinal zone

   Ported from Matlab to C and modified by Ali AlSaibie
   August 2014


   Original Author:
   Alexandre Schimel
   Coastal Marine Group - University of Waikato
   Hamilton, New Zealand
   Version:
   April 2007
*/
#define PI 3.14159265358979323846

float ddmmm2radians(float coordinate){
  float min = fmod(coordinate, 100.0)/60.0;
  float deg = float(int(coordinate/100));
  return (deg+min)*PI/180.0;
}

float ddmmm2dd(float coordinate){
  float min = fmod(coordinate, 100.0)/60.0;
  float deg = float(int(coordinate/100));
  return (deg+min);
}

// Lat Lon in dd mm.mmm format
void wgs2utm(float Latitude, float Longitude, float *utm_E, float *utm_N, int *utmZone)
{

float Lat = ddmmm2dd(Latitude);
float Lon = ddmmm2dd(Longitude);

float lat =	Lat*PI/180.0;
float lon = Lon*PI/180.0;
// WGS84 parameters
float a = 6378137.0;           //semi-major axis
float b = 6356752.314245;    //semi-minor axis
// b = 6356752.314140;  //GRS80 value, originally used for WGS84 before refinements
float e = sqrt(1-powf((b/a),2.0)); // eccentricity
// UTM parameters

float Lon0 = floor(Lon/6)*6+3; // reference longitude in degrees
float lon0 = Lon0*PI/180;      // in radians
float k0 = 0.9996;               // scale on central meridian

float FE = 500000;              // false easting
float FN = (Lat < 0)*10000000; // false northing

// Equations parameters
float eps = pow(e,2.0)/(1.0-pow(e,2.0));  // e prime square

//std::cout<<e<<std::endl;
// N: radius of curvature of the earth perpendicular to meridian plane
// Also, distance from point to polar axis
float N = a/sqrt(1-powf(e,2.0)*powf(sin(lat),2.0));
float T = powf(tan(lat),2.0);
float C = ((powf(e,2.0))/(1-powf(e,2.0)))*powf(cos(lat),2.0);
float A = (lon-lon0)*cos(lat);
// M: true distance along the central meridian from the equator to lat
float M = a*(  ( 1.0 - powf(e,2.0) / 4.0 - 3.0*powf(e,4.0) / 64.0 - 5.0*powf(e,6.0)/256.0 )  * lat
         -( 3.0 * powf(e,2.0)/8.0 + 3.0 * powf(e,4.0) / 32.0 + 45.0*powf(e,6.0)/1024.0 ) * sin(2.0*lat)
         +( 15.0 * powf(e,4.0)/256.0 + 45.0*powf(e,6.0)/1024.0 )            * sin(4.0*lat)
         -( 35.0 * powf(e,6.0)/3072.0 ) * sin(6.0*lat) );
// easting
*utm_E = FE + k0*N*(A
                 + (1-T+C)                      * pow(A,3.0)/6
                 + (5-18*T+pow(T,2.0)+72*C-58*eps) * pow(A,5.0)/120 );

// northing
// M(lat0) = 0 so not used in following formula
*utm_N = FN + k0*M + k0*N*tan(lat)*(                                     pow(A,2.0)/2
                                   + (5-T+9*C+4*pow(C,2.0))              * pow(A,4.0)/24
                                   + (61-58*T+pow(T,2.0)+600*C-330*eps) * pow(A,6.0)/720 );

//UTM zone
*utmZone = floor(Lon0/6)+31;
}

void getDistance_Bearing(float *targetLat, float *targetLon, float *baseLat, float *baseLon, quint16 &distance, quint16 &bearing )
{

    // Using Haversine formula for calculating Over-the-surface distance
    // Source: http://www.movable-type.co.uk/scripts/latlong.html
    int R = 6371000;   // Mean Earth radius

    float lat = ddmmm2radians(*baseLat);
    float latTarget = ddmmm2radians(*targetLat);
    float lon = ddmmm2radians(*baseLon);
    float lonTarget = ddmmm2radians(*targetLon);

    float dlat = latTarget-lat;
    float dlong = lonTarget-lon;

    float a = sin(dlat/2)*sin(dlat/2)+sin(dlong/2)*sin(dlong/2)*cos(lat)*cos(latTarget);
    float c = 2*atan2(sqrt(a),sqrt(1-a));

    distance = R*c;

    // Using the formula for bearing to calculate the required current heading.
    float y = sin(dlong)*cos(latTarget);
    float x = cos(lat)*sin(latTarget)-sin(lat)*cos(latTarget)*cos(dlong);

    bearing = fmod((atan2(y,x)*57.295779513+360),360); // Multiplied by constant to convert to decimal degrees.

}


#endif // GPSCONVERSION_H
