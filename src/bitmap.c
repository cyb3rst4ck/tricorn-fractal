#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include <stdint.h>


void generateBmp(unsigned char* img, char* filename, size_t width, size_t height, size_t size) {
    filename = filename[0] == 0? "tricorn.bmp": filename;

    char head[54] = {0};
    // Formal requirement for bitmap fileformat -> header
    head[0] = 'B';                                  // Typ Deklaration als Bitmap Datei in den ersten zwei Speicherzellen
    head[1] = 'M';

    *(uint32_t*)&head[2] = 54+size;                 // Size of BMP file        
    *(uint32_t*)&head[6] = 0; 
    *(uint32_t*)&head[10]  = 54;                    // Offset wo das Pixel(Picture) Array im BMP File beginnt
    *(uint32_t*)&head[14] = 40;                     // Anzahl von Bytes des DIP Header        
    *(int32_t*)&head[18] = width;                   // Width
    *(int32_t*)&head[22] = -height;                  // Height          
    *(uint16_t*)&head[26] = 1;                      // Anzahl der verwendeten Farbebenen         
    *(uint16_t*)&head[28] = 24;                     // Bumber of bits per pixel (bpp)
    *(uint32_t*)&head[30] = 0;  
    *(uint32_t*)&head[34] = size;                   // Size of raw bitmap data          
    *(int32_t*)&head[38] = 0; 
    *(int32_t*)&head[42] = 0; 
    *(uint32_t*)&head[46] = 0; 
    *(uint32_t*)&head[50] = 0; 

    FILE *bmpFile = fopen(filename,"w");       // create bmp file with w parameter (create new file and overwrite if it exists)
    fwrite(head, 1, 54, bmpFile);              // write bmp header to file
    fwrite(img, 1, size, bmpFile);             // write pixel data to file
    fclose(bmpFile);                           // close filestream
    fprintf(stdout, "> Creating output file: Bitmap was created (%s)\n", filename);
}
