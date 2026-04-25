// astro_display.cpp
// Contient l'implémentation de toutes les fonctions utilitaires et d'affichage

#include "astro_display.h"

// Implémentation de isDST (Détection Heure d'Été/Hiver)
bool isDST(int year, int month, int day, int hour) {
    // Règle : Du dernier dimanche de mars (à 2h00) au dernier dimanche d'octobre (à 3h00)
    int lastSundayMarch = 31 - (5 * year / 4 + 4) % 7;
    int lastSundayOct = 31 - (5 * year / 4 + 1) % 7;

    if (month < 3 || month > 10) return false;
    if (month > 3 && month < 10) return true;

    if (month == 3) return (day > lastSundayMarch) ||
    (day == lastSundayMarch && hour >= 2);
    if (month == 10) return (day < lastSundayOct) ||
    (day == lastSundayOct && hour < 3);
    return false;
}

// Implémentation de azimutToCardinal
const char* azimutToCardinal(double azimut) {
    if (azimut >= 337.5 || azimut < 22.5) return "NORD";
    if (azimut >= 22.5 && azimut < 67.5) return "NORD-EST";
    if (azimut >= 67.5 && azimut < 112.5) return "EST";
    if (azimut >= 112.5 && azimut < 157.5) return "SUD-EST";
    if (azimut >= 157.5 && azimut < 202.5) return "SUD";
    if (azimut >= 202.5 && azimut < 247.5) return "SUD-OUEST";
    if (azimut >= 247.5 && azimut < 292.5) return "OUEST";
    if (azimut >= 292.5 && azimut < 337.5) return "NORD-OUEST";
    return "?";
}

// Implémentation de getObjectMetadata
void getObjectMetadata(int objectId, const char** name, const char** constellation, double* ra, double* dec_coord) {

    if (objectId == 0) {
        *name = "ANDROMEDE (M31)";
        *constellation = "Andromede";
        *ra = M31_RA_HOURS;
        *dec_coord = M31_DEC_DEGREES; 
    } else if (objectId == 1) {
        *name = "ORION (M42)";
        *constellation = "Orion";
        *ra = M42_RA_HOURS;
        *dec_coord = M42_DEC_DEGREES; 
    } else if (objectId == 2) {
        *name = "PLEIADES (M45)";
        *constellation = "Taureau";
        *ra = M45_RA_HOURS;
        *dec_coord = M45_DEC_DEGREES; 
    } else if (objectId == 3) {
        *name = "Neb.Amerique du Nord (NGC7000)";
        *constellation = "Cygne";
        *ra = NGC7000_RA_HOURS;
        *dec_coord = NGC7000_DEC_DEGREES;
    }
    else {
        *name = "INCONNU";
        *constellation = "N/A";
    }
}

// Implémentation de displayRtcError (Non bloquant, affichage en bas à gauche)
void displayRtcError(int status) {
    my_lcd.Fill_Rect(50, 290, 200, 30, BLACK); 

    if (status == 1) {
        // Erreur I2C/Module
        my_lcd.Set_Text_colour(RED);
        my_lcd.Set_Text_Size(2);
        my_lcd.Print_String("ERREUR RTC", 50, 290);
        my_lcd.Set_Text_Size(1);
        my_lcd.Print_String("Verifiez I2C", 50, 310);
    } else if (status == 2) {
        // Pile faible
        my_lcd.Set_Text_colour(YELLOW);
        my_lcd.Set_Text_Size(2);
        my_lcd.Print_String("PILE RTC FAIBLE", 50, 290);
        my_lcd.Set_Text_Size(1);
        my_lcd.Print_String("Heure instable", 50, 310);
    }
}

// Implémentation de displayDateTime
void displayDateTime(DateTime now) {
    my_lcd.Set_Text_Mode(1);
    my_lcd.Set_Text_colour(GREEN);
    my_lcd.Set_Text_Back_colour(BLACK);
    my_lcd.Set_Text_Size(2); // Taille de texte
    my_lcd.Fill_Rect(0, 0, 250, 25, BLACK);

    my_lcd.Print_Number_Int(now.day(), 10, 5, 2, '0', 10);
    my_lcd.Print_String("/", 40, 5);
    my_lcd.Print_Number_Int(now.month(), 55, 5, 2, '0', 10);
    my_lcd.Print_String("/", 85, 5);
    my_lcd.Print_Number_Int(now.year(), 100, 5, 4, '0', 10);
    my_lcd.Print_String("-", 160, 5);
    my_lcd.Print_Number_Int(now.hour(), 180, 5, 2, '0', 10);
    my_lcd.Print_String(":", 205, 5);
    my_lcd.Print_Number_Int(now.minute(), 220, 5, 2, '0', 10);
}

// Implémentation de displayObjectInfo (Affichage universel pour M31, M42, M45)
void displayObjectInfo(int objectId, double altitude, double azimuth, FLOAT rise, FLOAT set, RiseAndSetState riseAndSetState, int offsetHeure) {
    my_lcd.Fill_Rect(250, 0, 229, 320, BLACK);
    const char *objectName, *constellationName;
    double ra_dummy, dec_dummy;
    getObjectMetadata(objectId, &objectName, &constellationName, &ra_dummy, &dec_dummy);

    my_lcd.Set_Text_Size(2);
    
    // Titre de l'objet (CYAN)
    my_lcd.Set_Text_colour(CYAN);
    my_lcd.Print_String(objectName, 275, 20); 

    
    my_lcd.Fill_Rect(250, 200, 230, 130, BLACK); // Nettoie le bas de la zone info

    my_lcd.Set_Text_colour(GREEN);
    my_lcd.Set_Text_Size(1);
    my_lcd.Print_String("Constellation:", 250, 185); 
    my_lcd.Set_Text_colour(WHITE);
    my_lcd.Print_String(constellationName, 370, 185);

    my_lcd.Set_Draw_color(WHITE); 

    // --- Altitude (Label VERT, Valeur JAUNE, Statut ROUGE/VERT) ---
    my_lcd.Set_Text_Size(2);
    if (altitude > 0) {
        my_lcd.Set_Text_colour(GREEN);
        my_lcd.Print_String("Altitude:", 250, 110);
        my_lcd.Set_Text_colour(YELLOW); 
        my_lcd.Print_Number_Float(altitude, 1, 300, 110, '.', 10, ' ');
        my_lcd.Set_Text_colour(GREEN);
        my_lcd.Print_String(" deg", 430, 110);
    } else {
        my_lcd.Set_Text_colour(RED);
        my_lcd.Print_String("SOUS L'HORIZON", 280, 130);
    }

    // --- Direction (Label VERT, Direction JAUNE) ---
    my_lcd.Set_Text_Size(2);
    if (altitude > 0) {
        my_lcd.Set_Text_colour(GREEN);
        my_lcd.Print_String("Direction:", 250, 140);
        my_lcd.Set_Text_colour(YELLOW); 
        my_lcd.Print_String(azimutToCardinal(azimuth), 370, 140);
    }

    // --- Lever/Coucher (Label BLANC, Heures MAGENTA) ---
    my_lcd.Set_Text_Size(1);
    my_lcd.Set_Text_colour(WHITE);
    my_lcd.Print_String("Lever/Couch (Loc):", 250, 170);

    char timeBuffer[10];
    int h, m; float s;
    
    if (riseAndSetState == RiseAndSetOk) {
        my_lcd.Set_Text_colour(MAGENTA); 
        // Lever
        Ephemeris::floatingHoursToHoursMinutesSeconds(Ephemeris::floatingHoursWithUTCOffset(rise, offsetHeure), &h, &m, &s);
        sprintf(timeBuffer, "%02d:%02d", h, m);
        my_lcd.Print_String(timeBuffer, 370, 170);
        
        my_lcd.Set_Text_colour(WHITE);
        my_lcd.Print_String(" / ", 410, 170);
        
        // Coucher
        my_lcd.Set_Text_colour(MAGENTA); 
        Ephemeris::floatingHoursToHoursMinutesSeconds(Ephemeris::floatingHoursWithUTCOffset(set, offsetHeure), &h, &m, &s);
        sprintf(timeBuffer, "%02d:%02d", h, m);
        my_lcd.Print_String(timeBuffer, 440, 170);

    } else {
        // Statut d'invisibilité (ROUGE)
        my_lcd.Set_Text_Size(1);
        my_lcd.Fill_Rect(250, 220, 230, 15, BLACK);
        my_lcd.Set_Text_colour(RED);
        if (riseAndSetState == ObjectAlwaysInSky) {
            my_lcd.Print_String("Toujours visible toute la nuit", 250, 220);
        } else if (riseAndSetState == ObjectNeverInSky) {
            my_lcd.Print_String("Jamais visible (circumpolaire)", 250, 220);
        } else {
            my_lcd.Print_String("Calcul Lever/Couch indisponible", 250, 220);
        }
    }
}

/*void displayVisibilitySlide(int currentObjId) {
    // 1. Nettoyage complet de l'écran
    my_lcd.Fill_Screen(BLACK);
    
    // 2. Configuration STRICTE du texte pour éviter le fond rouge
    my_lcd.Set_Text_Mode(0);              // Mode 0 = Opaque (écrase ce qu'il y a derrière)
    my_lcd.Set_Text_Back_colour(BLACK);   // Force le fond du texte en NOIR
    
    // 3. Titre
    my_lcd.Set_Text_Size(2);
    my_lcd.Set_Text_colour(CYAN);
    my_lcd.Print_String("CALENDRIER DE VISIBILITE", 50, 15);

    int startX = 90;   
    int startY = 80;   
    int cellW = 30;    
    int cellH = 40;    
    
    const char* mois[] = {"JAN","FEV","MAR","AVR","MAI","JUN","JUL","AOU","SEP","OCT","NOV","DEC"};
    const char* noms[] = {"M31", "M42", "M45", "NGC"};

    for (int m = 0; m < 12; m++) {
        // Mois en haut
        my_lcd.Set_Text_Size(1);
        my_lcd.Set_Text_colour(WHITE);
        my_lcd.Set_Text_Back_colour(BLACK); // Sécurité répétée
        my_lcd.Print_String(mois[m], startX + (m * cellW) + 2, startY - 20);
        
        for (int obj = 0; obj < 4; obj++) {
            if(m == 0) {
                // Noms à gauche
                my_lcd.Set_Text_Size(2);
                my_lcd.Set_Text_colour(obj == currentObjId ? CYAN : WHITE);
                my_lcd.Set_Text_Back_colour(BLACK); // Sécurité répétée
                my_lcd.Print_String(noms[obj], 15, startY + (obj * cellH) + 12);
            }

            uint16_t color = BLACK;
            int mm = m + 1; 
            
            if(obj == 0) { // M31
                color = (mm >= 8 && mm <= 11) ? GREEN : ((mm >= 12 || mm <= 2) ? YELLOW : BLACK);
            } else if(obj == 1 || obj == 2) { // M42 & M45
                color = (mm >= 11 || mm <= 1) ? GREEN : ((mm == 10 || mm == 2) ? YELLOW : BLACK);
            } else if(obj == 3) { // NGC 7000
                color = (mm >= 6 && mm <= 9) ? GREEN : ((mm == 5 || mm == 10) ? YELLOW : BLACK);
            }

            int x1 = startX + (m * cellW);
            int y1 = startY + (obj * cellH);
            int x2 = x1 + cellW - 4;
            int y2 = y1 + cellH - 4;

            if(color != BLACK) {
                my_lcd.Set_Draw_color(color);
                my_lcd.Fill_Rectangle(x1, y1, x2, y2);
            } else {
                my_lcd.Set_Draw_color(0x2104); 
                my_lcd.Draw_Rectangle(x1, y1, x2, y2);
            }
        }
    }

    // Curseur mois actuel
    extern RTC_DS3231 rtc;
    int curM = rtc.now().month() - 1;
    my_lcd.Set_Draw_color(WHITE);
    my_lcd.Draw_Rectangle(startX + (curM * cellW) - 2, startY - 5, 
                         startX + ((curM + 1) * cellW) - 2, startY + (4 * cellH) + 2);
}*/
