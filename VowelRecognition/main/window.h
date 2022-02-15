#ifndef __WINDOW_H
#define __WINDOW_H

#include <stdint.h>

typedef enum {
    Rectangular,
    Hamming,
    Kaiser
} WindowType;


void apply_and_create_window(float *x, float *w, uint32_t N, WindowType winType);
void apply_window(float *x, float *w, uint32_t N);

// Window generators
void window_rectangular(float *w, uint32_t L);
void window_hamming(float *w, uint32_t L);
void window_kaiser(float *w, uint32_t L, float beta);

// Zero-order modified Bessel function of the first kind
double I0(float x, int nIterations);

#endif
