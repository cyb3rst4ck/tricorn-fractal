#include "algorithms.h"

void tricornSeq_V0(float complex start, size_t width, size_t height, float res, unsigned n, unsigned char *img)
{
    size_t arrayRows = height;
    size_t arrayCols = width;
    float complex pixelPositionC;
    float complex z0;
    float complex z1;
    size_t pixel = 0;
    double normalizedIteration = 0.0;
    int counterIteration;
    float divergenceBorder = 4.0; // Divergence border for each iteration

    int addval; // Pixel offset for each row for bmp creation
    int ctr = 0; // Sum up pixel offsets for bitmap creation
    

    /* Calculate offset for bitmap creation
     * Number of pixels per row need to be multiple of 4 in bmp file
     */
    if ((width % 4) == 0)
    {
        addval = 0;
    }
    else
    {
        addval = (4 - ((width * 3) % 4));
    }

    for (int i = 0; i < arrayRows; i++)
    {
        for (int j = 0; j < arrayCols; j++)
        {
            // Calculate c -> Pixel position in complex plane with regards to
            pixelPositionC = CMPLX(res * (j - ((arrayCols - 1) / 2.0)), res * ((arrayRows - i - 1) - ((arrayRows - 1) / 2.0)));

            // Set z0 and z1 to given or default start value
            z0 = CMPLX(crealf(start), cimagf(start));
            z1 = CMPLX(crealf(start), cimagf(start));

            // Iterate n-times for actual pixel and check for divergence
            counterIteration = 0;
            for (int k = 0; k < n; k++)
            {
                counterIteration++;
                // Set z0 (zi) to last z1 (zi-1)
                z0 = z1;

                // Calculate new zi with given m = 2
                z1 = cpow(conj(z0), 2);
                z1 += pixelPositionC;

                // Divergence criteria
                if (sqrtf((powf(crealf(z1), 2) + powf(cimagf(z1), 2))) > sqrtf(divergenceBorder))
                {   
                    counterIteration--;
                    break;
                }
            }
            // Calculate pixel position in bitmap array
            pixel = (arrayCols * i + j) * 3 + ctr;

            // Calculate normalized iteration factor to set colors based on
            normalizedIteration = (double)counterIteration / (double)n;

            // Set BGR color values -> no divergence: black, divergence: colors
            if (counterIteration >= n)
            {
                img[pixel + 0] = 0; // blue
                img[pixel + 1] = 0; // green
                img[pixel + 2] = 0; // red
            }
            else
            {
                img[pixel + 0] = 255;                       // blue
                img[pixel + 1] = 255 * normalizedIteration; // green
                img[pixel + 2] = 255 * normalizedIteration; // red
            }
        }
        // Add pixel offset for bmp creation
        ctr += addval;
    }
}

void tricornPar_V1(float complex start, size_t width, size_t height, float res, unsigned n, unsigned char *img)
{   
    // seq Params
    float complex pixelPositionC;
    float complex z0;
    float complex z1;
    int counterIteration;
    float divergenceBorder = 4.0; 
    // Berechne Anzahl der Zeilen und Spalten
    size_t arrayRows = height;
    size_t arrayCols = width;

    size_t total = arrayRows * arrayCols;
    float resAligned __attribute__((aligned(16))) = res;

    float interReal __attribute__((aligned(16))) = (arrayCols - 1) / 2.0;

    float interImag = (arrayRows - 1) - ((arrayRows - 1) / 2.0);
    //  Alle Konstant -> deshalb hier einmal berechnen
    __m128 c_simd_interReal = _mm_load_ps1(&interReal);
    __m128 c_simd_interImag = _mm_load_ps1(&interImag);

    __m128 c_simd_Res = _mm_load_ps1(&resAligned);

    __m128 c_simd_idx_col;
    __m128 c_simd_idx_row;

    // Hier wird das Ergebnis reingeschrieben
    __m128 c_simd_Real;
    __m128 c_simd_Imag;

    __m128 iter_simd_Real;
    __m128 iter_simd_Imag;
    __m128 iter_simd_Result;

    __m128 iter_simd_check_Real;
    __m128 iter_simd_check_Imag;
    __m128 finalCheck;

    float start_const_real __attribute__((aligned(16))) = crealf(start);
    float start_const_imag __attribute__((aligned(16))) = cimagf(start);


    __m128 bigDivergenceBorder = _mm_load_ps1(&divergenceBorder);

    float const_mul = 2.0;
    __m128 bigConstMul = _mm_load_ps1(&const_mul);

    int maskCheck;

    bool flag1 = false;
    bool flag2 = false;
    bool flag3 = false;
    bool flag4 = false;

    size_t pixel = 0;
    double normalizedIteration = 0.0;

    // new
    int addval = 0;

    if ((width % 4) == 0)
    {
        addval = 0;
    }
    else
    {
        addval = (4 - ((width * 3) % 4));
    }

    // Schritt 1: per SIMD für alle Arrayeinträge das C berechnen
    for (int i = 0; i < (total - (total % 4)); i += 4)
    {
        // Berechnung von C
        c_simd_idx_col = _mm_set_ps((i + 3) % arrayCols, (i + 2) % arrayCols, (i + 1) % arrayCols, (i + 0) % arrayCols); // j
        c_simd_Real = _mm_sub_ps(c_simd_idx_col, c_simd_interReal); //(a, b) -> a-b|| j - inter

        c_simd_Real = _mm_mul_ps(c_simd_Res, c_simd_Real); // res * idxtotal
    
        c_simd_idx_row = _mm_set_ps((i + 3) / arrayCols, (i + 2) / arrayCols, (i + 1) / arrayCols, (i + 0) / arrayCols); // j
   
        c_simd_Imag = _mm_sub_ps(c_simd_interImag, c_simd_idx_row); //(a, b) -> a-b|| j - inter
    
        c_simd_Imag = _mm_mul_ps(c_simd_Res, c_simd_Imag); // res * idxtotal

        // Berechnung Iterationen: z1 = z0conj ^2 + c;
        iter_simd_Real = _mm_load_ps1(&start_const_real);
        iter_simd_Imag = _mm_load_ps1(&start_const_imag);

        iter_simd_Imag = _mm_sub_ps(_mm_set1_ps(0.0), iter_simd_Imag);

        flag1 = false;
        flag2 = false;
        flag3 = false;
        flag4 = false;

        for (int j = 0; j < n; j++)
        {
            // conj
            iter_simd_Imag = _mm_sub_ps(_mm_set1_ps(0.0), iter_simd_Imag); // Conjugate last z
            // Pow2
            iter_simd_Result = _mm_mul_ps(iter_simd_Real, iter_simd_Imag); // real*imag
            iter_simd_Result = _mm_mul_ps(iter_simd_Result, bigConstMul);  // real*imag*2

            iter_simd_Real = _mm_mul_ps(iter_simd_Real, iter_simd_Real);   // real²
            iter_simd_Imag = _mm_mul_ps(iter_simd_Imag, iter_simd_Imag);   // imag²
            iter_simd_Imag = _mm_sub_ps(_mm_set1_ps(0.0), iter_simd_Imag); // bei Quadrierung von Imaginärteil geht das Vorzeichen verloren

            iter_simd_Real = _mm_add_ps(iter_simd_Imag, iter_simd_Real); // neuer REalteil
            iter_simd_Imag = iter_simd_Result;
            // + c
            iter_simd_Real = _mm_add_ps(iter_simd_Real, c_simd_Real);
            iter_simd_Imag = _mm_add_ps(iter_simd_Imag, c_simd_Imag);
            // Abbruchbedingung
            iter_simd_check_Real = _mm_mul_ps(iter_simd_Real, iter_simd_Real);
            iter_simd_check_Imag = _mm_mul_ps(iter_simd_Imag, iter_simd_Imag);

            iter_simd_check_Imag = _mm_add_ps(iter_simd_check_Real, iter_simd_check_Imag);

            finalCheck = _mm_cmpgt_ps(iter_simd_check_Imag, bigDivergenceBorder);

            maskCheck = _mm_movemask_ps(finalCheck);

            // Farbberechnung

            if (!flag1 && maskCheck & 0x1)
            {
                pixel = (arrayCols * ((i + 0) / arrayCols) + ((i + 0) % arrayCols)) * 3 + ((i + 0) / arrayCols)*addval; // (40 * 10 + 26) * 3 = 1278
                normalizedIteration = (double)(j) / (double)n;
                if ((j) < n)
                {
                    img[pixel + 0] = 255;                       // blue
                    img[pixel + 1] = 255 * normalizedIteration; // green
                    img[pixel + 2] = 255 * normalizedIteration; // red
                }
                flag1 = true;
            }

            if (!flag2 && maskCheck & 0x2)
            {
                pixel = (arrayCols * ((i + 1) / arrayCols) + ((i + 1) % arrayCols)) * 3 + ((i + 1) / arrayCols)*addval; // (40 * 10 + 26) * 3 = 1278
                normalizedIteration = (double)(j) / (double)n;
                if ((j) < n)
                {
                    img[pixel + 0] = 255;                       // blue
                    img[pixel + 1] = 255 * normalizedIteration; // green
                    img[pixel + 2] = 255 * normalizedIteration; // red
                }
                flag2 = true;
            }

            if (!flag3 && maskCheck & 0x4)
            {
                pixel = (arrayCols * ((i + 2) / arrayCols) + ((i + 2) % arrayCols)) * 3 + ((i + 2) / arrayCols)*addval; // (40 * 10 + 26) * 3 = 1278
                normalizedIteration = (double)(j) / (double)n;
                if ((j) < n)
                {
                    img[pixel + 0] = 255;                       // blue
                    img[pixel + 1] = 255 * normalizedIteration; // green
                    img[pixel + 2] = 255 * normalizedIteration; // red
                }
                flag3 = true;
            }

            if (!flag4 && maskCheck & 0x8)
            {
                pixel = (arrayCols * ((i + 3) / arrayCols) + ((i + 3) % arrayCols)) * 3 + ((i + 3) / arrayCols)*addval; // (40 * 10 + 26) * 3 = 1278
                normalizedIteration = (double)(j) / (double)n;
                if ((j) < n)
                {
                    img[pixel + 0] = 255;                       // blue
                    img[pixel + 1] = 255 * normalizedIteration; // green
                    img[pixel + 2] = 255 * normalizedIteration; // red
                }
                flag4 = true;
            }

            if (maskCheck == 15)
            {
                break;
            }
        }

        // Calculating the non multiples of 4
        for (int r = (total - (total % 4)); r < total; r ++)
        {
            // Calculate c -> Pixel position in complex plane with regards to
            pixelPositionC = CMPLX(res * ((r % arrayCols) - ((arrayCols - 1) / 2.0)), res * ((arrayRows - (r / arrayCols) - 1) - ((arrayRows - 1) / 2.0)));

            // Set z0 and z1 to given or default start value
            z0 = CMPLX(crealf(start), cimagf(start));
            z1 = CMPLX(crealf(start), cimagf(start));

            // Iterate n-times for actual pixel and check for divergence
            counterIteration = 0;
            for (int k = 0; k < n; k++)
            {
                counterIteration++;
                // Set z0 (zi) to last z1 (zi-1)
                z0 = z1;

                // Calculate new zi with given m = 2
                z1 = cpow(conj(z0), 2);
                z1 += pixelPositionC;

                // Divergence criteria
                if (sqrtf((powf(crealf(z1), 2) + powf(cimagf(z1), 2))) > sqrtf(divergenceBorder))
                {   
                    counterIteration--;
                    break;
                }
            }
            // Calculate pixel position in bitmap array
            pixel = (arrayCols * ((r) / arrayCols) + (r % arrayCols)) * 3 + ((r) / arrayCols)*addval;

            // Calculate normalized iteration factor to set colors based on
            normalizedIteration = (double)counterIteration / (double)n;

            // Set BGR color values -> no divergence: black, divergence: colors
            if (counterIteration >= n)
            {
                img[pixel + 0] = 0; // blue
                img[pixel + 1] = 0; // green
                img[pixel + 2] = 0; // red
            }
            else
            {
                img[pixel + 0] = 255;                       // blue
                img[pixel + 1] = 255 * normalizedIteration; // green
                img[pixel + 2] = 255 * normalizedIteration; // red
            }
        }
        // Add pixel offset for bmp creation        
    }
    // Schritt 1.1: Realteil: res * (j - ((arrayCols - 1) / 2.0))
    // Schritt 1.2: Imaginärteil: res * ((arrayRows - i-1) - ((arrayRows - 1) / 2.0))

    // Schritt 2: per SIMD für alle Arrayeinträge die Iterationsschritte berechnen
}

void print128_num(__m128 var)
{
    float val[4];
    memcpy(val, &var, sizeof(val));
    printf("Float Val 128bit: %f %f %f %f \n",
           val[0], val[1], val[2], val[3]);
}
