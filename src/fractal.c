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


const char *usage_msg =
    "Usage: %s [options]                             Calculate tricorn (default: -V 0 -n 100 -r 1.0 -d 10,10 -s 0.0,0.0 -o tricorn.bmp)\n"
    "   or: %s -t testcase [-V algorithm]            Run tests with specified or default algorithm and exit (default: algorithm = 0)\n"
    "   or: %s -B [repetitions] [-V algorithm]       Run performance analysis and exit (default: algorithm = 0, repetitions = 3)\n"
    "   or: %s -h                                    Show help message and exit\n"
    "   or: %s --help                                Show help message and exit\n";

const char *help_msg =
    "\nOptional arguments:\n"
    "  -V algorithm             Set tricorn implementation (options: algorithm = {0, 1} | default: algorithm = 0)\n"
    "                               -> 0 : Non-optimized sequential C-algorithm\n"
    "                               -> 1 : Optimized SIMD C-algorithm\n"
    "  -B [repetitions]         Execute performance measurement (options: repetitions >= 1 | default: repetitions = 3)\n"
    "  -s real,imag             Set complex starting value z0 of calculation (default: real = 0.0, imag = 0.0})\n"
    "  -d width,height          Set width & height of picture (options: width >= 1, height >= 1 | default: width = 10, height = 10)\n"
    "  -n iterations            Run tests and exit (options: iterations >= 1 | default: iterations = 100)\n"
    "  -r step                  Set step size per pixel (default: step = 1.0)\n"
    "  -o file                  Set output file for bitmap (default: file = \"tricorn.bmp\")\n"
    "  -t testnumber            Run tests and exit (testnumber = {0, 1, 2, 3})\n"
    "                               -> 0 : Testcase with n=2, r=1.0, d=5,5, s=0,0\n"
    "                               -> 1 : Testcase with n=100, r=0.1, d=4,4, s=0,0\n"
    "                               -> 2 : Testcase with n=4, r=0.5, d=4,4, s=0,0\n"
    "                               -> 3 : Testcase with n=10, r=0.1, d=4,4, s=0,0\n"
    "                               -> 4 : Testcase with n=20, r=0.02, d=200,200, s=0,0\n"
    "                               -> 5 : Testcase with n=1, r=0.2, d=20,20, s=0,0\n"
    "                               -> 6 : Testcase with n=1, r=0.2, d=40,40, s=1,1\n"
    "                               -> 7 : Testcase with n=1, r=0.2, d=40,40, s=-2,0\n"                             
    "  -h                       Show help message (this text), usage and exit\n"
    "  --help                   Show help message (this text), usage and exit\n";


void print_usage(const char *progname)
{
    fprintf(stderr, usage_msg, progname, progname, progname, progname, progname);
}


void print_help(const char *progname)
{
    print_usage(progname);
    fprintf(stderr, "\n%s", help_msg);
}


int main(int argc, char **argv)
{
    // Necessary for argument parsing -> helper
    const char *progname = argv[0];
    int opt;
    char *ptr;
    int ctr = 0;
    int help_flag = 0;
    int perf_flag = 0;

    // Necessary for argument parsing -> These control the program behaviour
    char filename[256] = {0};
    float resolution = 1.0;
    int iterations = 100;
    int reruns = 3;
    int implementation = 0;
    int testcase = -1;
    size_t screenWidth = 10;
    size_t screenHeight = 10;
    float realPart = 0.0;
    float imagPart = 0.0;
    float complex cStart = CMPLX(realPart, imagPart);

    /* optarg is a global variable in getopt.h. it contains the argument
     * for this option. it is null if there was no argument.
     */

    /* table for getopt_long
     * fields: name, has_arg, flag, val
     * has_arg: no_argument, required_argument, optional_argument
     */
    struct option longopts[] = {
        {"help", no_argument, &help_flag, 1},
        {0}};


    while (1)
    {
        opt = getopt_long(argc, argv, "hV:B::s:d:n:r:o:t:", longopts, 0);

        if (opt == -1)
        {
            break;
        }

        switch (opt)
        {
        case 'h':
            print_help(progname);
            return 1;
        case 'V':
            /*
             * Usage: -V<int>
             */
            implementation = optarg ? atoi(optarg) : 0;
            if (implementation < 0)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid implementation number. The implementation number needs to be 0 or above\n");
                return 1;
            }
            break;
        case 'B':
            /*
             * Usage: -B<int>
             */
            perf_flag = 1;
            reruns = optarg ? atoi(optarg) : 3;
            if (reruns < 1)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid number of reruns! Number of reruns need to be 1 or above");
                return 1;
            }
            break;
        case 's':
            /*
             * Usage: -s<real>,<imag>
             */
            ctr = 0;
            ptr = strtok(optarg, ",");
            while (optarg != NULL && ptr != NULL)
            {
                if (ctr == 0)
                { 
                    realPart = atof(ptr);
                }
                else if (ctr == 1)
                {
                    imagPart = atof(ptr);
                }
                else
                {
                    break;
                }
                ptr = strtok(NULL, ",");
                ctr++;
            }
            if (ctr != 2)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid number of arguments for -s! Please pass width and height accordingly"); // Kleinster Wert 0 -> z0
                return 1;
            }
            cStart = CMPLX(realPart, imagPart);
            break;
        case 'd':
            /*
             * Usage: -d<int>,<int>
             */
            ctr = 0;
            ptr = strtok(optarg, ",");
            while (optarg != NULL && ptr != NULL)
            {   
                if (strstr(ptr, "-") != NULL)
                {
                    fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid values for width or height! Please pass width and height that is not negative"); // Kleinster Wert 0 -> z0
                    return 1;
                }
                if (ctr == 0)
                {   
                    screenWidth = atoi(ptr);
                }
                else if (ctr == 1)
                {
                    screenHeight = atoi(ptr);
                }
                else
                {
                    break;
                }
                ptr = strtok(NULL, ",");
                ctr++;
            }

            if (ctr != 2)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid number of arguments for -d! Please pass width and height accordingly"); // Kleinster Wert 0 -> z0
                return 1;
            }
            if (screenHeight == 0 || screenWidth == 0)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid values for width or height! Please pass width and height above 0"); // Kleinster Wert 0 -> z0
                return 1;
            }
            break;
        case 'n':
            /*
             * Usage: -n<int>
             */
            iterations = atoi(optarg);
            if (iterations < 1)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid iterations per pixel! Iterations need to be 1 or above");
                return 1;
            }
            break;
        case 'r':
            /*
             * Usage: -r<float>
             */

            resolution = atof(optarg);
            if (resolution <= 0)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid resolution! Resolution needs to be above 0");
                return 1;
            }
            break;
        case 'o':
            /*
             * Usage: -o<file>
             */

            // Copy given filname from commandline into filename variable
            strncpy(filename, optarg ? optarg : "", sizeof(filename));

            // In case the filetype is not included -> file type appended to given filename
            if (strstr(filename, ".bmp") == NULL)
            {
                strcat(filename, ".bmp");
                fprintf(stdout, "> Parsing program arguments: Filetype .bmp is missing\n");
                fprintf(stdout, "> Parsing program arguments: Filetype automatically added to given filename: %s\n", filename);
            }
            break;
        case 't':
            /*
             * Usage: -T<int>
             */
            // Parse testcase number based on commandline input
            testcase = optarg ? atoi(optarg) : -1;
            if (testcase < 0)
            {
                fprintf(stderr, "%s\n", "> Parsing program arguments: Invalid testcase number! The testcase number needs to be 0 or above");
                return 1;
            }
            break;
        case ':':
            fprintf(stderr, "%s\n", "> Parsing program arguments: Missing argument!");
            print_usage(progname);
            return 1;
        case '?':
            /* a return value of '?' indicates that an option was malformed.
             * this could mean that an unrecognized option was given, or that an
             * option which requires an argument did not include an argument.
             */

            fprintf(stderr, "%s\n", "> Parsing program arguments: Unknown parameter!\n\n");
            print_usage(progname);
            return 1;
        default:
            fprintf(stdout, "%s\n", "> Parsing program arguments: No parameter given, program will execute with default values!");
            print_usage(progname);
            return 1;
        }
    }

    // Show help&usage message
    if (help_flag)
    {
        print_help(progname);
        return 1;
    }

    // In case no arguments were passed, show respective message and show default parameters
    if (argc < 2)
    {
        fprintf(stdout, "%s\n", "> Parsing program arguments: No arguments given, program will run with default values");
        fprintf(stdout, "%s%s%s\n", "> Parsing program arguments: ", progname, " -V 0 -n 100 -r 1.0 -d 10,10 -s 0.0,0.0 -o tricorn.bmp");
    }

    // Choose Algorithm via function pointer with passed/default implementation parameter
    func_ptr algo = selectAlgorithm(implementation);
    // In case an invalid number was passed and no algorithm found -> exit
    if (algo == NULL)
    {
        return 1;
    }

    size_t arrayRows = screenHeight;
    size_t arrayCols = screenWidth;
    size_t size;
    if ((arrayCols%4) == 0){
        size = arrayRows * arrayCols * 3;
    }else{
        size = arrayRows * arrayCols * 3 + (4 - ((arrayCols*3) % 4)) * arrayRows * 1;
    }

    // Initialize array for bitmap with 0s (color = black) // TODO: evtl. kann man sogar noch optimieren indem man nur Farbwerte schreibt
    unsigned char *img = calloc(size, sizeof(unsigned char)); // allocate array for bitmap and initialize with 0's -> 0 = black

    // Check if performance analysis desired
    if (perf_flag)
    {   
        // Run performance analysis
        performanceFractal(algo, cStart, screenWidth, screenHeight, resolution, iterations, img, reruns);
    }
    else
    {   
        // Run desired tricorn algorithm which was selected by function selectAlgorithm
        algo(cStart, screenWidth, screenHeight, resolution, iterations, img);
    }

    // Generate Bitmap
    generateBmp(img, filename, arrayCols, arrayRows, size);

    if (testcase >= 0)
    {   
        // In case the -t flag was passed, also run desired testcase
        callDesiredTestcase(img, size, testcase);
    }

    // Free dynamically
    free(img);

    return 0;
}

func_ptr selectAlgorithm(int implementation)
{
    func_ptr algo;
    switch (implementation)
    {
    case 0:
        printf("> Selecting Algorithm: tricornSeq_V0\n");
        algo = &tricornSeq_V0;
        return algo;
    case 1:
        printf("> Selecting Algorithm: tricornPar_V1\n");
        algo = &tricornPar_V1;
        return algo;
    default:
        fprintf(stderr, "%s\n", "> Selecting Algorithm: Algorithm unknown. Type -h to see an overview over possible algorithms");
        return NULL;
    }
}
