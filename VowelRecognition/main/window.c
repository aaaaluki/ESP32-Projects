#include "window.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>


void apply_and_create_window(float *x, float *w, uint32_t N, WindowType winType)
{
    static const float kaiser_beta = 38;
    
    // Generate corresponding window
    switch (winType)
    {
    case Rectangular:
        window_rectangular(w, N);
        break;
    
    case Hamming:
        window_hamming(w, N);
        break;
    
    case Kaiser:
        window_kaiser(w, N, kaiser_beta);
        break;
    
    default:
        window_rectangular(w, N);
        break;
    }

    // Apply window
    apply_window(x, w, N);
}


void apply_window(float *x, float *w, uint32_t N)
{
    for (int i = 0; i < N; i++)
    {
        x[i] = x[i]*w[i];
    }
}


void window_rectangular(float *w, uint32_t L)
{
    for (int i = 0; i < L; i++)
    {
        w[i] = 1;
    }
}


void window_hamming(float *w, uint32_t L)
{
    // Alpha and Beta calculation
    // Sauce: https://ccrma.stanford.edu/~jos/sasp/Hamming_Window.html
    static const float alpha = 25.0f / 46.0f;
    static const float beta = 0.5f*(1 - alpha);

    // Window Generation
    // Sauce: https://es.mathworks.com/help/signal/ref/hamming.html
    for (int i = 0; i < L; i++)
    {
        w[i] = alpha - 2*beta*cos(2*M_PI*i/(L - 1));
    }
}


void window_kaiser(float *w, uint32_t L, float beta)
{
    // nIterations found by empirically
    static const int nIterations = 60;
    // Window Generation
    // Sauce: https://es.mathworks.com/help/signal/ref/kaiser.html
    double N = L - 1;
    double tmp;
    double I0_beta = I0(beta, nIterations);

    for (int n = 0; n <= N; n++)
    {
        tmp = (n - N/2)/(N/2);
        w[n] = (float) I0(beta * sqrt(1 - tmp*tmp), nIterations) / I0_beta;
    }
}


double I0(float x, int nIterations)
{
    double result = 1;
    double factorial = 1;
    double numerator;

    // Sauce: https://ccrma.stanford.edu/~jos/sasp/Kaiser_Window.html
    // We can skip the first term in the summation because it will allways be one
    for (int k = 1; k < nIterations; k++)
    {
        // Calculate numerator = (x/2)^k
        numerator = 1;
        for (int j = 0; j < k; j++)
        {
            numerator *= x / 2;
        }

        factorial = factorial*k;

        result += (numerator*numerator) / (factorial*factorial);
    }

    return result;
}

