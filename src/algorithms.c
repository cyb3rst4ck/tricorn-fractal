#include "algorithms.h"



void tricornSeq(float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img){
    // Deklarieren des Arrays basierend auf Resolution und width/height
    size_t arrayRows = (size_t) ceil(height/res);
    size_t arrayCols = (size_t) ceil(width/res);

    float complex actualValue;
    // Setze alle Einträge in Array auf Startwert
    float complex z0;
    float complex z1;
    size_t pixel = 0;
    double visualData = 0.0;
    int counterIteration;
    float divergenceBorder = 4.0;

    for (int i=0;i<arrayRows;i++){
        for (int j=0;j<arrayCols;j++){
            // Setze alle Einträge in Array auf Startwert
            //actualValue = CMPLX(j * res - ((float) width / 2.0), i * res - ((float) height /2.0));
            actualValue = CMPLX(res * (j - ((arrayCols - 1) / 2.0)), res * ((arrayRows - i-1) - ((arrayRows - 1) / 2.0)));
            /* if (i == 0 && j == 3){
                printf("C: Zeile %d und Spalte %d\nReal: %f\nImag: %f\n", i, j, crealf(actualValue), cimagf(actualValue));
            }  */
            z0 = CMPLX(crealf(start), cimagf(start));
            z1 = CMPLX(crealf(start), cimagf(start));
            /*
                Abbruchkriterien:
                1. Fall: der Betrag des berechneten Tricorn Fractals liegt zum Quadrat genommen oberhalb von divergenceBorder
                2. Fall: die als Parameter übergebene maximale Iterationsanzahl wurde überschritten
            */
            counterIteration = 0;
            for (int k = 0;k<n;k++){
                counterIteration++;
                z0 = z1;
                z1 = cpow(conj(z0),2); // Power ist m = 2
     
                z1 += actualValue;
                if (i == 10 && j == 26){
                    printf("-------------------------------------------\n");
                    printf("Iteration: %d\n", counterIteration);
                    printf("Real: %4.8f\nImag: %4.8f\n", crealf(z1), cimagf(z1));
                    printf("-------------------------------------------\n");
                }  
                if (pow(crealf(z1),2) + pow(cimagf(z1),2) > divergenceBorder){
                    // divergentm
                    break;
                } 
            }
            actualValue = z1;
            /* if (i == 11 && j == 16){
                    printf("Abbruch Iteration: %d\n", counterIteration);
            }  */
            /* if (i == 0 && j == 0){
                printf("Zeile %d und Spalte %d\nReal: %f\nImag: %f\n", i, j, crealf(actualValue), cimagf(actualValue));
            }  */
            //printf("Zeile %d und Spalte %d\nReal: %f\nImag: %f\n", i, j, crealf(actualValue), cimagf(actualValue));
            // Berechne Visualierungsdaten um Farbwerte für jeden Pixel festzulegen
            pixel = (arrayCols * i + j) * 3; // (40 * 10 + 26) * 3 = 1278 
            // umgekehrte rgb reihenfolge da Little Endian
            visualData = (double) counterIteration / (double) n;
            if (pixel == 1278){
                printf("Zeile: %d\nSpalte: %d\n", i, j);
            } 
            if (counterIteration >= n){
                img[pixel + 0] = 0;                   // blau
                img[pixel + 1] = 0;                   // grün
                img[pixel + 2] = 0;                   // rot
            } else{
                img[pixel + 0] = 255;                 // blau
                img[pixel + 1] = 255 * visualData;    // grün
                img[pixel + 2] = 255 * visualData;    // rot
            } 
            if (pixel == 1278){
                printf("B: %d\n", img[pixel + 0]);
                printf("G: %d\n", img[pixel + 1]);
                printf("R: %d\n", img[pixel + 2]);
            } 
        } 
    }
} 

void tricornPar(float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img){
    // Berechne Anzahl der Zeilen und Spalten
    size_t arrayRows = (size_t) ceil(height/res);
    size_t arrayCols = (size_t) ceil(width/res);

    size_t total = arrayRows * arrayCols;
    float resAligned __attribute__((aligned (16))) = res;

    float interReal __attribute__((aligned (16))) = (arrayCols - 1) / 2.0;
    //printf("InterReal: %f\n", interReal);

    float interImag = (arrayRows - 1) - ((arrayRows - 1) / 2.0);
    //printf("InterImag: %f\n", interImag);
    // Alle Konstant -> deshalb hier einmal berechnen
    __m128 c_simd_interReal = _mm_load_ps1(&interReal);
    //print128_num(c_simd_interReal);
    __m128 c_simd_interImag = _mm_load_ps1(&interImag);
    //print128_num(c_simd_interImag);
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

    float start_const_real __attribute__((aligned (16))) = crealf(start);
    float start_const_imag __attribute__((aligned (16))) = cimagf(start);
    
    float divergenceBorder = 4.0;
    __m128 bigDivergenceBorder = _mm_load_ps1(&divergenceBorder);

    float const_mul = 2.0;
    __m128 bigConstMul = _mm_load_ps1(&const_mul);

    int maskCheck;

    bool flag1 = false;
    bool flag2 = false;
    bool flag3 = false;
    bool flag4 = false;

    //int counterIteration = 0;

    size_t pixel = 0;
    double visualData = 0.0;

    // Schritt 1: per SIMD für alle Arrayeinträge das C berechnen
    for (int i = 0; i < (total - (total % 4)); i += 4){
        printf("\n----------------------------------------------------------------------------\n");
        // Berechnung von C
        printf("\nReal:\n");
        c_simd_idx_col = _mm_set_ps((i+3)%arrayCols, (i+2)%arrayCols, (i+1)%arrayCols, (i+0)%arrayCols); // j
        print128_num(c_simd_idx_col);
        c_simd_Real = _mm_sub_ps(c_simd_idx_col, c_simd_interReal); //(a, b) -> a-b|| j - inter 
        //print128_num(c_simd_Real);
        c_simd_Real = _mm_mul_ps(c_simd_Res, c_simd_Real); // res * idxtotal
        print128_num(c_simd_Real);
      
        printf("\nImag:\n");
        c_simd_idx_row = _mm_set_ps((i+3)/arrayCols, (i+2)/arrayCols, (i+1)/arrayCols, (i+0)/arrayCols); // j
        print128_num(c_simd_idx_row);
        c_simd_Imag = _mm_sub_ps(c_simd_interImag, c_simd_idx_row); //(a, b) -> a-b|| j - inter 
        //print128_num(c_simd_Imag);
        c_simd_Imag = _mm_mul_ps(c_simd_Res, c_simd_Imag); // res * idxtotal
        print128_num(c_simd_Imag);
        
        // Berechnung Iterationen: z1 = z0conj ^2 + c;
        printf("\nIter:\n");
        iter_simd_Real = _mm_load_ps1(&start_const_real);
        print128_num(iter_simd_Real);
        iter_simd_Imag = _mm_load_ps1(&start_const_imag);
        print128_num(iter_simd_Imag);
        // Test
        //iter_simd_Imag = _mm_xor_ps(iter_simd_Imag, iter_xor_const);
        iter_simd_Imag = _mm_sub_ps(_mm_set1_ps(0.0),iter_simd_Imag);
        print128_num(iter_simd_Imag);
        //print128_num(iter_xor_const);
        //
        //for (int j = 0; j < (n - (n % 4)); j += 4){
        flag1 = false;
        flag2 = false;
        flag3 = false;
        flag4 = false;
        //counterIteration = 0;
        
        for (int j = 0; j < n; j ++){
         
            // conj
            iter_simd_Imag = _mm_sub_ps(_mm_set1_ps(0.0),iter_simd_Imag); // Conjugate last z
            // Pow2
            iter_simd_Result = _mm_mul_ps(iter_simd_Real, iter_simd_Imag); // real*imag
            iter_simd_Result = _mm_mul_ps(iter_simd_Result, bigConstMul); // real*imag*2

            iter_simd_Real = _mm_mul_ps(iter_simd_Real, iter_simd_Real); // real²
            iter_simd_Imag = _mm_mul_ps(iter_simd_Imag, iter_simd_Imag); // imag²
            iter_simd_Imag = _mm_sub_ps(_mm_set1_ps(0.0),iter_simd_Imag); // bei Quadrierung von Imaginärteil geht das Vorzeichen verloren

            iter_simd_Real = _mm_add_ps(iter_simd_Imag, iter_simd_Real); // neuer REalteil
            iter_simd_Imag = iter_simd_Result;
            // + c
            iter_simd_Real = _mm_add_ps(iter_simd_Real, c_simd_Real);
            iter_simd_Imag = _mm_add_ps(iter_simd_Imag, c_simd_Imag);
            // Abbruchbedingung
            printf("\nIterLoop: %d\n", j);
            print128_num(iter_simd_Real);
            print128_num(iter_simd_Imag);
            // 
            iter_simd_check_Real = _mm_mul_ps(iter_simd_Real, iter_simd_Real);
            iter_simd_check_Imag = _mm_mul_ps(iter_simd_Imag, iter_simd_Imag);

            iter_simd_check_Imag = _mm_add_ps(iter_simd_check_Real, iter_simd_check_Imag);

            finalCheck = _mm_cmpgt_ps(iter_simd_check_Imag, bigDivergenceBorder);
            print128_num(finalCheck);
            maskCheck = _mm_movemask_ps(finalCheck);
            printf("Mask: %d\n", maskCheck);

            // Farbberechnung
            

            if (!flag1 && maskCheck & 0x1){ // 0b1
                printf("Erstes Bit gesetzt\n");
                pixel = (arrayCols * ((i+0)/arrayCols) + ((i+0)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278
                visualData = (double) (j) / (double) n;
                img[pixel + 0] = 255;                 // blau
                img[pixel + 1] = 255 * visualData;    // grün
                img[pixel + 2] = 255 * visualData;    // rot
                flag1 = true;
            } 

            if (!flag2 && maskCheck & 0x2){ // 0b10
                printf("Zweites Bit gesetzt\n");
                pixel = (arrayCols * ((i+1)/arrayCols) + ((i+1)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278
                visualData = (double) (j) / (double) n;
                img[pixel + 0] = 255;                 // blau
                img[pixel + 1] = 255 * visualData;    // grün
                img[pixel + 2] = 255 * visualData;    // rot
                flag2 = true;
            } 

            if (!flag3 && maskCheck & 0x4){ // // 0b100
                printf("Drittes Bit gesetzt\n");
                pixel = (arrayCols * ((i+2)/arrayCols) + ((i+2)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278 
                visualData = (double) (j) / (double) n;
                img[pixel + 0] = 255;                 // blau
                img[pixel + 1] = 255 * visualData;    // grün
                img[pixel + 2] = 255 * visualData;    // rot
                flag3 = true;
            } 

            if (!flag4 && maskCheck & 0x8){ // 0b1000
                printf("Viertes Bit gesetzt\n");
                pixel = (arrayCols * ((i+3)/arrayCols) + ((i+3)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278 
                visualData = (double) (j) / (double) n;
                img[pixel + 0] = 255;                 // blau
                img[pixel + 1] = 255 * visualData;    // grün
                img[pixel + 2] = 255 * visualData;    // rot
                flag4= true;
            } 
    
            if (maskCheck == 15){ //0b1111 
                printf("Stop IterLoop: %d\n", j);
                break;
            } 
            


            // Speichern der ITerationen in Bitmap
        } 
        if (!flag1){ // 0b1
            printf("Erstes Bit gesetzt -> black\n");
            pixel = (arrayCols * ((i+0)/arrayCols) + ((i+0)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278 
            img[pixel + 0] = 0;                 // blau
            img[pixel + 1] = 0;                 // grün
            img[pixel + 2] = 0;                 // rot
        } 

        if (!flag2){ // 0b10
            printf("Zweites Bit gesetzt -> black\n");
            pixel = (arrayCols * ((i+1)/arrayCols) + ((i+1)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278 
            img[pixel + 0] = 0;                 // blau
            img[pixel + 1] = 0;                 // grün
            img[pixel + 2] = 0;                 // rot
        } 

        if (!flag3){ // // 0b100
            printf("Drittes Bit gesetzt -> black\n");
            pixel = (arrayCols * ((i+2)/arrayCols) + ((i+2)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278 
            img[pixel + 0] = 0;                 // blau
            img[pixel + 1] = 0;                 // grün
            img[pixel + 2] = 0;                 // rot
        } 

        if (!flag4){ // 0b1000
            printf("Viertes Bit gesetzt -> black\n");
            pixel = (arrayCols * ((i+3)/arrayCols) + ((i+3)%arrayCols)) * 3; // (40 * 10 + 26) * 3 = 1278 
            img[pixel + 0] = 0;                 // blau
            img[pixel + 1] = 0;                 // grün
            img[pixel + 2] = 0;                 // rot
        } 
        // Speichern in img Array
    } 
    // Schritt 1.1: Realteil: res * (j - ((arrayCols - 1) / 2.0))
    // Schritt 1.2: Imaginärteil: res * ((arrayRows - i-1) - ((arrayRows - 1) / 2.0))

    // Schritt 2: per SIMD für alle Arrayeinträge die Iterationsschritte berechnen
    return;
} 

void print128_num(__m128 var)
{
    float val[4];
    memcpy(val, &var, sizeof(val));
    printf("Float Val 128bit: %f %f %f %f \n", 
           val[0], val[1], val[2], val[3]);
}
