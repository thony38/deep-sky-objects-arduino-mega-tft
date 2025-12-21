// astro_display.cpp
// Contient l'implémentation de toutes les fonctions utilitaires et d'affichage

#include "astro_display.h"


// --- DÉFINITIONS DES CONSTELLATIONS (Coordonnées relatives 0-100) ---

// Les coordonnées sont (X_début, Y_début, X_fin, Y_fin)
// Zone de dessin virtuelle : 100x100

// 1. Andromède (And) - Représentation simplifiée de la chaîne d'étoiles
const int AND_LINES[] = {
    // --- 1. Chaîne principale (Beaucoup plus plate, de Y=70 à Y=55) ---
    
    // Segment 1: Almach (X=10, Y=70) à Mirach (X=35, Y=65)
    // Almach est la première étoile (la plus à gauche/bas)
    10, 70,  35, 65,  
    
    // Segment 2: Mirach (X=35, Y=65) à l'étoile suivante (X=65, Y=60)
    35, 65,  65, 60,  
    
    // Segment 3: Étoile suivante à Alpheratz (X=90, Y=55)
    65, 60,  90, 55,
    
    // --- 2. Bifurcation vers le haut (partant de Mirach - Plus vertical et net) ---
    // Segment 4: Part de Mirach (X=35, Y=65) vers l'étoile 1 de la bifurcation (X=40, Y=40)
    35, 65,  40, 40, 
    
    // Segment 5: Étoile 1 de la bifurcation (X=40, Y=40) à l'étoile 2 (X=45, Y=10)
    40, 40,  45, 10,
    
    // Terminateur
    -1, -1, -1, -1 
};

// 2. Orion (Ori) - Représentation du sablier avec la ceinture et l'épée
const int ORI_LINES[] = {
    // Épaule supérieure gauche à épaule supérieure droite
    10, 10,  90, 10, 
    10, 10,  20, 80,
    90, 10,  80, 80,
    20, 80,  80, 80, 
    
    // Ceinture d'Orion (centrée horizontalement)
    25, 45,  50, 45,
    50, 45,  75, 45,
    
    // Épée d'Orion (vers la Nébuleuse M42)
    50, 45,  50, 65,
    // Terminateur
    -1, -1, -1, -1 
};

// 3. Taureau (Tau) - Le "V" de la tête (Hyades) et les Pléiades (M45)
const int TAU_LINES[] = {
    // Tête du Taureau (Hyades en forme de V)
    10, 80,  50, 30, 
    90, 80,  50, 30, 
    
    // Corne 1 (Gauche)
    50, 30,  20, 5,  
    // Corne 2 (Droite)
    50, 30,  80, 5,  
    // Terminateur
    -1, -1, -1, -1 
};


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
        *name = "LMC";
        *constellation = "DORADE/MENSA";
        *ra = LMC_RA_HOURS;
        *dec_coord = LMC_DEC_DEGREES;
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
    my_lcd.Set_Text_Size(2);
    my_lcd.Fill_Rect(270, 10, 200, 40, BLACK);

    my_lcd.Print_String("Date: ", 270, 10);
    my_lcd.Print_Number_Int(now.day(), 335, 10, 2, '0', 10);
    my_lcd.Print_String("/", 365, 10);
    my_lcd.Print_Number_Int(now.month(), 380, 10, 2, '0', 10);
    my_lcd.Print_String("/", 400, 10);
    my_lcd.Print_Number_Int(now.year(), 420, 10, 4, '0', 10);
    my_lcd.Print_String("Heure: ", 290, 30);
    my_lcd.Print_Number_Int(now.hour(), 370, 30, 2, '0', 10);
    my_lcd.Print_String(":", 400, 30);
    my_lcd.Print_Number_Int(now.minute(), 420, 30, 2, '0', 10);
}

// Implémentation de displayObjectInfo (Affichage universel pour M31, M42, M45)
void displayObjectInfo(int objectId, double altitude, double azimuth, FLOAT rise, FLOAT set, RiseAndSetState riseAndSetState, int offsetHeure) {
    my_lcd.Fill_Rect(250, 80, 230, 240, BLACK); 

    const char *objectName, *constellationName;
    double ra_dummy, dec_dummy;
    getObjectMetadata(objectId, &objectName, &constellationName, &ra_dummy, &dec_dummy);

    my_lcd.Set_Text_Size(2);
    
    // Titre de l'objet (CYAN)
    my_lcd.Set_Text_colour(CYAN);
    my_lcd.Print_String(objectName, 250, 80); 

    
    // Définir la zone d'affichage (Exemple : 260, 240, 100x100 pixels pour le dessin)
 // 2. Définition des dimensions (avec une marge de 10px de chaque côté)
    int constel_w = 220; // 240 (largeur dispo) - 20 (marges) = 220
    int constel_h = 110; // 130 (hauteur dispo) - 20 (marges) = 110
    
    // 3. Définition de la position de départ (coin haut-gauche du dessin)
    int constel_x = 250; // 240 (X de départ dispo) + 10 (marge) = 250
    int constel_y = 205; // 195 (Y de départ dispo) + 10 (marge) = 205

    my_lcd.Fill_Rect(250, 200, 230, 130, BLACK); // Nettoie le bas de la zone info

    my_lcd.Set_Text_colour(GREEN);
    my_lcd.Set_Text_Size(1);
    my_lcd.Print_String("Constellation:", 250, 185); 
    my_lcd.Set_Text_colour(WHITE);
    my_lcd.Print_String(constellationName, 370, 185);
    // Définition de la couleur de dessin pour les formes (Lignes et cercles)
    my_lcd.Set_Draw_color(WHITE); 
    
    // Dessiner la constellation (Dessin 100x100 relatif est maintenant projeté sur 100x75)
    drawConstellation(objectId, constel_x, constel_y, constel_w, constel_h);

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

// Implémentation de drawConstellation
void drawConstellation(int objectId, int x_start, int y_start, int width, int height) {
    const int *lines = nullptr;

    if (objectId == 0) {
        lines = AND_LINES;
    } else if (objectId == 1) {
        lines = ORI_LINES;
    } else if (objectId == 2) {
        lines = TAU_LINES;
    } else {
        return;
    }

    // Réglage de la couleur du texte (pour être sûr)
    my_lcd.Set_Text_colour(WHITE); 

    // Le tableau 'lines' contient des blocs de 4 entiers : x1, y1, x2, y2
    for (int i = 0; lines[i] != -1; i += 4) {
        int x1_rel = lines[i];
        int y1_rel = lines[i+1];
        int x2_rel = lines[i+2];
        int y2_rel = lines[i+3];

        // Transformation des coordonnées relatives (0-100) en coordonnées d'écran absolues
        int x1 = x_start + (x1_rel * width) / 100;
        int y1 = y_start + (y1_rel * height) / 100;
        int x2 = x_start + (x2_rel * width) / 100;
        int y2 = y_start + (y2_rel * height) / 100;

        // Dessiner l'étoile (point) au début de chaque segment
        my_lcd.Fill_Circle(x1, y1, 3); // Rayon 3 pour une meilleure visibilité
        
        // Dessiner la ligne
        my_lcd.Draw_Line(x1, y1, x2, y2);
        
        // Dessiner l'étoile (point) à la fin du dernier segment 
        if (lines[i+4] == -1) {
            my_lcd.Fill_Circle(x2, y2, 3); // Rayon 3 pour la dernière étoile
        }
    }
}