// astro_display.h
// Contient les prototypes des fonctions d'affichage et les déclarations externes

#ifndef ASTRO_DISPLAY_H
#define ASTRO_DISPLAY_H

#include <Arduino.h>
#include <RTClib.h>
#include <LCDWIKI_GUI.h>
#include <LCDWIKI_KBV.h>
#include <Ephemeris.h>

// Déclarations des variables globales (extern)
extern LCDWIKI_KBV my_lcd;
extern int rtcStatus;

// Déclaration des constantes astronomiques (définies dans DeepSkyObjects.h)
extern const double M31_RA_HOURS; 
extern const double M31_DEC_DEGREES; 
extern const double M42_RA_HOURS; 
extern const double M42_DEC_DEGREES; 
extern const double M45_RA_HOURS; 
extern const double M45_DEC_DEGREES; 
extern const double LMC_RA_HOURS;
extern const double LMC_DEC_DEGREES;

// Définitions de couleurs (pour que toutes les fonctions d'affichage les voient)
#define BLACK    0x0000
#define WHITE    0xFFFF
#define RED      0xF800
#define YELLOW   0xFFE0
#define GREEN    0x07E0
#define BLUE     0x001F
#define CYAN     0x07FF
#define MAGENTA  0xF81F

// Prototypes des fonctions
const char* azimutToCardinal(double azimut);
void getObjectMetadata(int objectId, const char** name, const char** constellation, double* ra, double* dec_coord);
void displayRtcError(int status);
void displayDateTime(DateTime now);
void displayObjectInfo(int objectId, double altitude, double azimuth, FLOAT rise, FLOAT set, RiseAndSetState riseAndSetState, int offsetHeure);
bool isDST(int year, int month, int day, int hour);

void drawConstellation(int objectId, int x_start, int y_start, int width, int height);

#endif