#include <stdio.h> /* for printf */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <complex.h>
#include "fractal.h"
#include "algorithms.h"
#include "performance.h"
#include "bitmap.h"
#include "testing.h"

// TODO: ./fractal -ds1.1,2.3 -> ERROR!


int main(int argc, char** argv) {
    // für Parsen der Argumente benötigt
    int opt; 
    char* ptr;
    int ctr = 0;
    // Benötigt für Argumentoptionen
    char filename[256] = { 0 };
    int help_flag = 0;
    int perf_flag = 0;
    
    float resolution = 1.0;
    int iterations = 0;
    int reruns = 1;
    int implementation = 0; // Standardmäßig V0 ausführen wenn -V nicht angegeben
    int testcase = -1; 

    size_t screenWidth = 100;
    size_t screenHeight = 100;

    float realPart = 0.0;
    float imagPart = 0.0;
    float complex cStart = CMPLX(realPart,imagPart);



    /* optarg is a global variable in getopt.h. it contains the argument
    * for this option. it is null if there was no argument.
    */

    /* table for getopt_long
     * fields: name, has_arg, flag, val
     * has_arg: no_argument, required_argument, optional_argument
     */
    struct option longopts[] = {
        { "help", no_argument, &help_flag, 1 },
        { 0 }
    };

    while (1) {
        opt = getopt_long (argc, argv, "hV:B::s:d:n:r:o:T:", longopts, 0);

        if (opt == -1) {
            break;
        }

        switch (opt) {
        case 'h':
            printf("-h passed as argument\n");
            break;
        case 'V':
            /* 
            * Usage: -V<int>
            * Die Implementierung, die verwendet werden soll. Hierbei soll mit -V0
            * Ihre Hauptimplementierung verwendet werden. Wenn diese Option nicht gesetzt
            * wird, soll ebenfalls die Hauptimplementierung ausgeführt werden.
            */
            printf("-V passed as argument\n");
            implementation = optarg?atoi(optarg):0;
            if (implementation < 0){
                printf("Invalid implementation number\nThe implementation number needs to be 0 or above\n");
                return 1;
            } 
            printf("Implementation: %d\n", implementation);
            break;
        case 'B':
            /* 
            * Usage: -B<int>
            * Falls gesetzt, wird die Laufzeit der angegebenen Implementierung
            * gemessen und ausgegeben. Das optionale Argument dieser Option gibt die Anzahl
            * an Wiederholungen des Funktionsaufrufs an.
            */
            printf("-B passed as argument\n");
            perf_flag = 1;
            reruns = optarg?atoi(optarg):0;
            if (reruns < 0){
                printf("Invalid number of reruns!\nNumber of reruns need to be 0 or above\n");
                return 1;
            } 
            printf("Reruns: %d\n", reruns);
            break;
		case 's':
            /* 
            * Usage: -s<real>,<imag>
            * Startpunkt der Berechnung
            */
            printf("-s passed as argument\n");
            printf("%s passed as value\n",optarg);
            ctr = 0;
            ptr = strtok(optarg,","); 
            while (optarg != NULL && ptr != NULL){
                if (ctr == 0){
                    realPart = atof(ptr);
                }else if (ctr == 1){
                    imagPart = atof(ptr);
                }else{
                    break;
                } 
                ptr = strtok(NULL, ",");
                ctr++;
            } 
            printf("%4.4f passed as value\n",realPart);
            printf("%4.4f passed as value\n",imagPart);
            cStart = CMPLX(realPart,imagPart);
            printf("%4.4f+%4.4fi passed as value\n",crealf(cStart),cimagf(cStart));

            break;
		case 'd':
            /* 
            * Usage: -d<int>,<int>
            * Breite und Höhe des Bildausschnitts
            */
            printf("-d passed as argument\n");
            ctr = 0;
            ptr = strtok(optarg,","); 
            while (optarg != NULL && ptr != NULL){
                if (ctr == 0){
                    screenWidth = atoi(ptr);
                }else if (ctr == 1){
                    screenHeight = atoi(ptr);
                }else{
                    break;
                } 
                ptr = strtok(NULL, ",");
                ctr++;
            } 
            printf("Input Width: %ld\n",screenWidth);
            printf("Input Height: %ld\n",screenHeight);
            
            break;
		case 'n':
            /* 
            * Usage: -n<int>
            * Maximale Anzahl an Iterationen pro Bildpixel
            */
            printf("-n passed as argument\n");
            iterations = atoi(optarg);
            if (iterations < 0){
                printf("Invalid number of iterations!\nNumber of iterations need to be 0 or above\n"); // TODO: Muss kleinster Wert 0 oder 1 sein?
                return 1;
            } 
            printf("%d passed as value\n",iterations);
            break;
		case 'r':
            /* 
            * Usage: -r<float>
            * Schrittweite pro Bildpixel
            */
            printf("-r passed as argument\n");
            printf("%s passed as value\n",argv[optind-1]);
            printf("%s passed as value\n",optarg);
            resolution = atof(optarg);
            if (resolution <= 0){
                printf("Invalid number of iterations!\nNumber of iterations need to be above 0\n"); // TODO: kleinster Wert?
                return 1;
            } 
            printf("%1.16f passed as value\n",resolution);
            break;
		case 'o':
            /* 
            * Usage: -o<file>
            * Ausgabedatei
            */
            printf("Filename Old: %s\n ", filename);
            printf("-o passed as argument\n");
            strncpy (filename, optarg ? optarg : "error", sizeof (filename));
            printf("Filename New: %s\n ", filename);
            break;
        case 'T':
            /* 
            * Usage: -T<int>
            * Der Testfall, der getestet werden soll. Es muss eine Zahl angegeben werden.
            */
            printf("-T passed as argument\n");
            testcase = optarg?atoi(optarg):-1;
            if (testcase < 0){
                printf("Invalid Testcase number\nThe testcase number needs to be 0 or above\n");
                return 1;
            } 
            printf("Testcase: %d\n", testcase);
            break;
        case ':':
            printf("missing argument\n");
            return 1;
        case '?':
            /* a return value of '?' indicates that an option was malformed.
             * this could mean that an unrecognized option was given, or that an
             * option which requires an argument did not include an argument.
             */
            //usage(stderr, argv[0]);
            printf("unknown parameter\n");
            return 1;
        default:
            printf("0 arguments given, program will execute with default values\n");
            break;
        }
    }


    // Show help message
    if (help_flag) {
        printf("--help passed as argument\n");
        return 0;
    }

    // Choose Algorithm
    func_ptr algo = selectAlgorithm(implementation);
    if (algo == NULL){
        return 1;
    } 
    
    // Initialize array
    size_t arrayRows = (size_t) ceil(screenHeight/resolution);
    size_t arrayCols = (size_t) ceil(screenWidth/resolution);
    size_t size = arrayRows * arrayCols * 3;

    unsigned char *img = calloc(size, sizeof(unsigned char));  // allocate array for bitmap and initialize with 0's -> 0 = black
    // Execute Performance Test
    if (perf_flag){
        performanceFractal(algo, cStart, screenWidth, screenHeight, resolution, iterations, img, reruns);
    } else{
        algo(cStart, screenWidth, screenHeight, resolution, iterations, img);
        if (testcase >= 0){
            callDesiredTestcase(img, size, testcase);
        } 
    } 

    // Generate Bitmap
    generateBmp(img, filename, arrayCols, arrayRows, size);
    printf("Bitmap was created\n");
    free(img);
	
	return 0;
}


func_ptr selectAlgorithm(int implementation){
    func_ptr algo;
    switch (implementation){
        case 0:
            printf("Selected Algorithm: tricornSeq\n");
            algo = &tricornSeq;
            return algo;
        case 1:
            printf("Selected Algorithm: tricornPar\n");
            algo = &tricornPar;
            return algo;
        default:
            printf("Algorithm unknown! Please choose a value between 0 and 1.\nProgram will be closed.\n");
            return NULL;
    } 
} 
