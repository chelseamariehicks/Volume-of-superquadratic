/**********************************************************************************
 * Name: Chelsea Marie Hicks
 * 
 * Description: use parallel reduction to compute the volume of a superquadric
 *
 * Compiled on macOS using g++-10 -o superquad superquad.cpp -O3 -lm -fopenmp
 * Compiled on flip using g++ -o superquad superquad.cpp -O3 -lm -fopenmp
***********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>

#define XMIN        -1
#define XMAX         1
#define YMIN        -1
#define YMAX         1

#define N           0.70

/*
//Set number of threads
#ifndef NUMT
#define NUMT        1
#endif

//Set number of nodes
#ifndef NUMNODES
#define NUMNODES       10000
#endif
*/

//Set number of tries to find maximum performance
#ifndef NUMTRIES
#define NUMTRIES        100
#endif

int NUMT = 1;
int NUMNODES = 10000;

//iu, iv = 0 ..NUMNODES - 1
float Height(int iu, int iv) {
    float x = -1. + 2. * (float)iu /(float) (NUMNODES-1); //-1. to +1.
    float y = -1. + 2. * (float)iv /(float) (NUMNODES-1); //-1. to +1.

    float xn = pow( fabs(x), (double)N );
    float yn = pow( fabs(y), (double)N );

    float r = 1. - xn - yn;

    if(r <= 0.) {
        return 0.;
    }
    float height = pow( r, 1./(float)N );
    return height;
}

int main(int argc, char *argv[]) {

#ifndef _OPENMP
    fprintf(stderr, "No OpenMP support!\n");
    return 1;
#endif

    if(argc >= 2) {
        NUMT = atoi(argv[1]);
    }

    if(argc >= 3) {
        NUMNODES = atoi(argv[2]);
    }

    //Area of a single full-sized tile
    float fullTileArea = (((XMAX-XMIN)/(float)(NUMNODES-1)) * ((YMAX-YMIN)/(float)(NUMNODES-1)));

    float maxPerformance = 0.;
    float totalVolume = 0.;

    //Set the number of threads
    omp_set_num_threads(NUMT);

    //Looking for max performance
    for(int tries = 0; tries < NUMTRIES; tries++) {
        double time0 = omp_get_wtime();
        float volume = 0.;

        //default(none) shared(fullTileArea) 
        #pragma omp parallel for reduction(+:volume)
        for(int i = 0; i < NUMNODES * NUMNODES; i ++) {
            int iu = i % NUMNODES;
            int iv = i / NUMNODES;
            float z = Height(iu, iv);

            float tileArea = fullTileArea;

            //corner tiles
            if((iu == NUMNODES-1 || iu == 0) && (iv ==NUMNODES-1 || iv == 0)) {
                tileArea *= 0.25;
            }
            //edge tiles
            else if((iu == NUMNODES-1 || iu == 0) || (iv == NUMNODES-1 || iv == 0)) {
                tileArea *= 0.50;
            }
            volume += 2 * z * tileArea;
        }
        double time1 = omp_get_wtime();
        float megaHeightsPerSecond = (float) NUMNODES * NUMNODES / (time1 - time0) / 1000000.;
        if(megaHeightsPerSecond > maxPerformance) {
            maxPerformance = megaHeightsPerSecond;
        }
        totalVolume += volume;
    }
    float avgVolume = totalVolume/NUMTRIES;
    fprintf(stderr, "%2d threads: %8d NUMNODES; volume = %6.2f; megaheights/sec = %6.21f\n", 
        NUMT, NUMNODES, avgVolume, maxPerformance);
    return 0;
}