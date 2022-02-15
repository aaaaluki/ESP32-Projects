#ifndef __UTILS_H
#define __UTILS_H

#include <stdint.h>

typedef struct {
    uint32_t m_frequency;    
    uint32_t m_index;
    uint32_t m_magnitude;
} Tone_t;

typedef struct
{
    uint32_t m_f1;
    uint32_t m_f2;
    char m_vowel;
} Vowel_t;

static Vowel_t vowels[] = {{700, 1350, 'A'},
                           {600, 1900, 'E'},
                           {350, 2150, 'I'},
                           {500, 1050, 'O'},
                           {400,  900, 'U'}};

void find_tone(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *tone);
void find_formants(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *t1, Tone_t *t2);
uint32_t vowel_estimation(Tone_t *t1, Tone_t *t2, Vowel_t *vowelEst);

static void find_formants_ratio(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *t1, Tone_t *t2);
static void find_formants_local_max(uint32_t Pxx[], uint32_t size, uint32_t fs, Tone_t *t1, Tone_t *t2);

#endif