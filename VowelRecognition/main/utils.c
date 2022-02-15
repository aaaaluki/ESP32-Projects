#include "utils.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>


void find_tone(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *tone)
{
    tone->m_index = 0;
    tone->m_magnitude = 0;
    
    // Skip DC
    for (int i = 1; i < size; i++)
    {
        if (Pxx[i] > tone->m_magnitude)
        {
            tone->m_magnitude = Pxx[i];
            tone->m_index = i;
        }
    }

    // The periodogram length is size, which is half of NFFT
    tone->m_frequency = tone->m_index*fs/(2*size);
}


void find_formants(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *t1, Tone_t *t2)
{
    //find_formants_ratio(Pxx, size, fs, t1, t2);
    find_formants_local_max(Pxx, size, fs, t1, t2);
}


uint32_t vowel_estimation(Tone_t *t1, Tone_t *t2, Vowel_t *vowelEst)
{
    vowelEst->m_f1 = 0;
    vowelEst->m_f2 = 0;
    vowelEst->m_vowel = 'x';

    // Use Manhattan distance to find the closest vowel
    uint32_t minDist = UINT32_MAX;
    uint32_t manhDist;
    for (int i = 0; i < 5; i++)
    {
        manhDist = abs(t1->m_frequency - vowels[i].m_f1) + abs(t2->m_frequency - vowels[i].m_f2);

        if (manhDist < minDist) {
            minDist = manhDist;

            vowelEst->m_f1 = vowels[i].m_f1;
            vowelEst->m_f2 = vowels[i].m_f2;
            vowelEst->m_vowel = vowels[i].m_vowel;
        }
    }

    return minDist;
}


static void find_formants_ratio(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *t1, Tone_t *t2)
{
    static const uint32_t formantPeakThreshold = 50;
    static const uint32_t minHeight = 1000;

    t1->m_magnitude = 1;
    t1->m_index = 0;
    t2->m_magnitude = 1;
    t2->m_index = 0;

    // Skip DC lobe
    uint32_t idxRightDC;
    for (idxRightDC = 0; idxRightDC < size - 1; idxRightDC++)
    {
        if (Pxx[idxRightDC + 1] > Pxx[idxRightDC])
        {
            break;
        }
    }

    // Find first tone
    for (uint32_t i = idxRightDC + 1; i < size - 1; i++)
    {
        if (Pxx[i-1] < Pxx[i] && Pxx[i] > Pxx[i+1] && Pxx[i] > minHeight)
        {
            // Peak found now lets check if it's the first maximum
            if (t1->m_magnitude > formantPeakThreshold*Pxx[i])
            {
                break;
            } else {
                t1->m_magnitude = Pxx[i];
                t1->m_index = i;
            }
        }
    }

    // Find second tone
    for (uint32_t i = t1->m_index + 1; i < size - 1; i++)
    {
        if (Pxx[i-1] < Pxx[i] && Pxx[i] > Pxx[i+1] && Pxx[i] > minHeight)
        {
            // Peak found now lets check if it's the first maximum
            if (t2->m_magnitude > formantPeakThreshold*Pxx[i])
            {
                break;
            } else {
                t2->m_magnitude = Pxx[i];
                t2->m_index = i;
            }
        }
    }

    t1->m_frequency = t1->m_index*fs/(2*size);
    t2->m_frequency = t2->m_index*fs/(2*size);
}


static void find_formants_local_max(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *t1, Tone_t *t2)
{
    t1->m_magnitude = 1;
    t1->m_index = 0;
    t2->m_magnitude = 1;
    t2->m_index = 0;

    // Skip DC lobe
    uint32_t idxRightDC;
    for (idxRightDC = 0; idxRightDC < size - 1; idxRightDC++)
    {
        if (Pxx[idxRightDC + 1] > Pxx[idxRightDC])
        {
            break;
        }
    }

    // Find local maximum and first tone
    // The first tone is the maximum in the periodogram
    uint32_t localMaxIdx[size/4];
    uint32_t counter = 0;
    uint32_t maxValCounterIdx = 0;
    for (uint32_t i = idxRightDC + 1; i < size - 1; i++)
    {
        if (Pxx[i-1] < Pxx[i] && Pxx[i] > Pxx[i+1])
        {
            localMaxIdx[counter] = i;

            if (Pxx[i] > t1->m_magnitude)
            {
                t1->m_magnitude = Pxx[i];
                t1->m_index = i;
                maxValCounterIdx = counter;
            }

            counter++;
        }
    }

    // Find second tone
    // If we skip the DC component and the first tone the remaining local
    // maximum should be the second tone
    // Skip first tone lobe
    uint32_t idx = 0;
    for (uint32_t i = maxValCounterIdx; i < counter - 1; i++)
    {
        if (Pxx[localMaxIdx[i]] < Pxx[localMaxIdx[i + 1]])
        {
            idx = i;
            break;
        }
    }

    for (uint32_t i = idx; i < counter - 1; i++)
    {
            if (Pxx[localMaxIdx[i]] > t2->m_magnitude)
            {
                t2->m_magnitude = Pxx[localMaxIdx[i]];
                t2->m_index = localMaxIdx[i];
            }
    }

    // Roll down to the right to 

    t1->m_frequency = t1->m_index*fs / (2*size);
    t2->m_frequency = t2->m_index*fs / (2*size);

}
