/*
Auteur : Thony38 
Date : 21/12/2025 
Projet : Horloge Astronomique 
*/

#include <SD.h>       // Gestion de la carte SD
#include <SPI.h>      // Communication SPI pour SD et TFT
#include <Wire.h>     // Communication I2C pour le module RTC
#include <RTClib.h>   // Gestion du module RTC
//-----Bibliothèques Écran-----//
#include <LCDWIKI_GUI.h> // Fonctions graphiques de base
#include <LCDWIKI_KBV.h> // Driver spécifique pour l'écran
//-----Astronomie-----//
#include <Ephemeris.h> // Calculs astronomiques (Altitude, Azimut, Lever/Coucher)
#include <math.h>      // Fonctions mathématiques
// --- INCLUSIONS DES FICHIERS PERSONNALISÉS ---
#include "DeepSkyObjects.h" // Contient les constantes des coordonnées (M31, M42, M45, NGC7000)
#include "astro_display.h"  // Contient les prototypes des fonctions utilitaires et d'affichage

//             CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12  D13  D14  D15 
//Arduino Mega 40  38  39  /   41   37  36  35  34  33  32  31  30  22  23  24   25   26   27   28   29

// --- DÉCLARATIONS DES OBJETS ET VARIABLES GLOBALES ---

// Initialisation de l'écran TFT
LCDWIKI_KBV my_lcd(ILI9486, 40, 38, 39, -1, 41);
// Initialisation du module RTC
RTC_DS3231 rtc;

#define SD_CS 53 // Broche Chip Select pour la carte SD

// Variables d'état et de contrôle
int rtcStatus = 0; // 0: OK, 1: Erreur I2C, 2: Pile faible
int minuteCounter = 60; // Compteur pour rafraîchissement TFT (Force un affichage au démarrage)
int currentObject = 0; // 0=M31, 1=M42, 2=M45, 3=NGC7000

/*5 minutes équivalent à 5 x 60 = 300 secondes.
300 secondes équivalent à 300 x 1000 = 300,000 millisecondes.*/
unsigned long lastRotationTime = 0;
const long rotationInterval = 60000; // Rotation d'objet toutes les 10 secondes

//Constantes pour les images BMP 
const char* andromedaImage = "ANDRO.bmp";
const char* orionImage = "ORION.bmp";    
const char* pleiadesImage = "PLEIADE.bmp"; 
const char* ngc7000Image = "NGC7000.bmp"; 
const char* ngc2237Image = "NGC2237.bmp";
const char* ic1805Image = "IC1805.bmp";

const char* currentImage = andromedaImage; // Pointeur vers l'image actuellement affichée


// --- FONCTIONS SD/BMP ---

bool check_bmp_header(File &bmpFile) {
    if (bmpFile.read() != 'B' || bmpFile.read() != 'M') return false;
    bmpFile.seek(10);
    uint32_t bmp_offset = bmpFile.read() |
    (bmpFile.read() << 8) | (bmpFile.read() << 16) | (bmpFile.read() << 24);
    bmpFile.seek(bmp_offset);
    return true;
}

void draw_bmp_picture(const char *filename, int x_offset, int y_offset) {
    File bmpFile = SD.open(filename);
    if (!bmpFile || !check_bmp_header(bmpFile)) {
        Serial.print("Erreur : Fichier BMP "); Serial.println(filename);
        if (bmpFile) bmpFile.close();
        return;
    }
    Serial.print("Chargement de l'image : "); Serial.println(filename);
    uint16_t lineBuffer[240];
    for (int y = 239; y >= 0; y--) { 
        for (int x = 0; x < 240; x++) {
            uint8_t b = bmpFile.read();
            uint8_t g = bmpFile.read();
            uint8_t r = bmpFile.read();
            lineBuffer[x] = my_lcd.Color_To_565(r, g, b);
        }
        my_lcd.Set_Addr_Window(x_offset, y + y_offset, x_offset + 239, y + y_offset);
        my_lcd.Push_Any_Color(lineBuffer, 240, 1, 0); 
    }
    bmpFile.close();
    Serial.println("✅ Image affichée !");
}


// --- SETUP (Initialisation) ---
void setup() {
    Serial.begin(9600);
    my_lcd.Init_LCD();
    my_lcd.Fill_Screen(BLACK);
    my_lcd.Set_Rotation(1); // Orientation Paysage

    // Configuration de la position géographique (Paris, France)
    Ephemeris::setLocationOnEarth(48, 51, 24, 2, 21, 8);
        
    // Initialisation de la carte SD
    if (!SD.begin(SD_CS)) {
        my_lcd.Set_Text_colour(RED);
        my_lcd.Set_Text_Size(2);
        my_lcd.Print_String("Erreur: Carte SD", 50, 50);
        Serial.println("⛔ Erreur SD. Les images ne chargeront pas.");
    } else {
        my_lcd.Fill_Rect(0, 40, 240, 280, BLACK);
        // Affichage initial de l'image d'Andromède
        draw_bmp_picture(currentImage, 0, 40); 
    }
    
   // Initialisation du module RTC 
    if (!rtc.begin()) {
        rtcStatus = 1; 
        Serial.println("⛔ Erreur RTC critique (I2C/Module non detecte).");
    } else {
        // force de la mise à jour à chaque démarrage du code
        /* LIGNE À DÉCOMMENTER POUR RÉGLER L'HEURE
          ->  Téléverser le code
          ->  Commenter 
          -> Retéléverser
        */
         //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));


        if (rtc.lostPower()) {
            rtcStatus = 2; // Pile faible
            Serial.println("⚠️ Pile RTC faible ou heure perdue. Reinitialisation necessaire.");
        }
        Serial.println("✅ RTC prêt.");
    }

    displayRtcError(rtcStatus); 
}


// --- LOOP ---
void loop() {
    DateTime now = rtc.now();
    unsigned long currentTime = millis();
    
    int offsetHeure = isDST(now.year(), now.month(), now.day(), now.hour()) ? 2 : 1;
    int utcHour = now.hour() - offsetHeure;
    if (utcHour < 0) utcHour += 24; 

    int previousObject = currentObject; 
    
    // --- Gestion de la Rotation (Toutes les 10 secondes) ---
    if (currentTime - lastRotationTime >= rotationInterval) {
        currentObject++;
        if (currentObject > 3) {
            currentObject = 0; // Cycle: M31 -> M42 -> M45 -> NGC7000 -> ... -> M31...
        }
        lastRotationTime = currentTime;
        minuteCounter = 60;
    }

    if (currentObject != previousObject) {
        if (currentObject == 0) {
            currentImage = andromedaImage;
        } else if (currentObject == 1) {
            currentImage = orionImage;
        } else if (currentObject == 2) {
            currentImage = pleiadesImage;
        } else if (currentObject == 3) { 
            currentImage = ngc7000Image;
        }
        draw_bmp_picture(currentImage, 0, 40); 
    }
    

    // --- Récupération des données pour l'objet actuel ---
    double RA, dec_coord;
    const char *name_dummy, *constellation_dummy;
    
    getObjectMetadata(currentObject, &name_dummy, &constellation_dummy, &RA, &dec_coord);
    
    EquatorialCoordinates eqCoordinates;
    eqCoordinates.ra = RA; 
    eqCoordinates.dec = dec_coord;

    // Calcul de l'Altitude, Azimut, Lever/Coucher
    HorizontalCoordinates horizonData = Ephemeris::equatorialToHorizontalCoordinatesAtDateAndTime(
        eqCoordinates, now.day(), now.month(), now.year(), utcHour, now.minute(), now.second());

    double altitude = horizonData.alt;
    double azimuth = horizonData.azi;
    FLOAT rise = 0.0;
    FLOAT set = 0.0;
    
    RiseAndSetState riseAndSetState = Ephemeris::riseAndSetForEquatorialCoordinatesAtDateAndTime(
        eqCoordinates, &rise, &set, now.day(), now.month(), now.year(), utcHour, now.minute(), now.second()
    );


    Serial.println("--------------------");
    Serial.print("OBJET ACTUEL: "); Serial.println(name_dummy);
    Serial.print("🔺 Altitude : "); Serial.print(altitude, 2); Serial.println(" deg");
    Serial.print("🧭 Azimut : "); Serial.print(azimuth, 2); Serial.print(" deg ("); Serial.print(azimutToCardinal(azimuth)); Serial.println(")");
    
    if (riseAndSetState == RiseAndSetOk) {
        int h_rise, m_rise; float s_rise;
        char riseTimeBuffer[6]; 
        Ephemeris::floatingHoursToHoursMinutesSeconds(Ephemeris::floatingHoursWithUTCOffset(rise, offsetHeure), &h_rise, &m_rise, &s_rise);
        sprintf(riseTimeBuffer, "%02d:%02d", h_rise, m_rise);
        Serial.print("Lever (Loc): "); Serial.print(riseTimeBuffer);
        
        int h_set, m_set; float s_set;
        char setTimeBuffer[6]; 
        Ephemeris::floatingHoursToHoursMinutesSeconds(Ephemeris::floatingHoursWithUTCOffset(set, offsetHeure), &h_set, &m_set, &s_set);
        sprintf(setTimeBuffer, "%02d:%02d", h_set, m_set);
        Serial.print(" | Coucher (Loc): "); Serial.println(setTimeBuffer);
        
    } else {
        Serial.print("Lever/Coucher : ");
        if (riseAndSetState == ObjectAlwaysInSky) Serial.println("Toujours visible");
        else if (riseAndSetState == ObjectNeverInSky) Serial.println("Jamais visible");
        else Serial.println("Calcul indisponible");
    }
    Serial.println("--------------------");


    // --- Gestion du rafraîchissement TFT (Toutes les minutes OU changement d'objet) ---
    minuteCounter++;
    if (minuteCounter >= 60) {
        minuteCounter = 0; 

        displayDateTime(now);
        displayObjectInfo(currentObject, altitude, azimuth, rise, set, riseAndSetState, offsetHeure);
        displayRtcError(rtcStatus);
    }

    delay(1000); 
}
