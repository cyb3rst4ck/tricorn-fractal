#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include <stdint.h>


void generateBmp(unsigned char* img, char* filename, size_t width, size_t height, size_t size) {
    filename = filename[0] == 0? "tricorn.bmp": filename;

    char head[54] = {0};
    // Formale Voraussetzungen für BMP Datei
    head[0] = 'B';                             // Typ Deklaration als Bitmap Datei in den ersten zwei Speicherzellen
    head[1] = 'M';

    *(uint32_t*)&head[2] = 54+size;                 // Größe der BMP-Datei        
    *(uint32_t*)&head[6] = 0; 
    *(uint32_t*)&head[10]  = 54;                    // Offset wo das Pixel(Picture) Array im BMP File beginnt
    *(uint32_t*)&head[14] = 40;                     // Anzahl von Bytes des DIP Header        
    *(int32_t*)&head[18] = width;                  // Breite
    *(int32_t*)&head[22] = height;                 // Höhe          
    *(uint16_t*)&head[26] = 1;                    // Anzahl der verwendeten Farbebenen         
    *(uint16_t*)&head[28] = 24;                   // Anzahl der Bits pro Pixel
    *(uint32_t*)&head[30] = 0;  
    *(uint32_t*)&head[34] = size;                   // Größe der rohen Bitmap-Daten          
    *(int32_t*)&head[38] = 0; 
    *(int32_t*)&head[42] = 0; 

    FILE *bmpFile = fopen(filename,"w");       // erzeugt ein bmp file w steht für write, und das gesamte gibt ein stream zurück
    fwrite(head, 1, 54, bmpFile);              // schreibt den bmp head in die Datei
    fwrite(img, 1, size, bmpFile);             // schreibt die Bildinformation in die Datei
    fclose(bmpFile);                           // schließt den Stream BMPFile */
}
