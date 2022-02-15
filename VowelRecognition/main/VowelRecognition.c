#include "driver/gpio.h"
#include "esp_log.h"
#include "fft.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils.h"
#include "window.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#define FS          8000
#define NFFT        (1<<10) // 1024
#define BIT_SCALE   5       // Pxx is divided by 2^5
#define STACK_SIZE  4*NFFT

#define LCD_RS  13
#define LCD_EN  12
#define LCD_BL  14
#define LCD_D4  27
#define LCD_D5  26
#define LCD_D6  25
#define LCD_D7  33


static const char *TAG = "VowelRecognition";
static const float kaiserBeta = 10;
static float window[NFFT];

static const int32_t data[NFFT] = {-267, -251, -205, -267, -230, -161, -115, -14, 64, 126, 145, 168, 223, 223, 214, 216, 214, 260, 232, 218, 294, 310, 361, 487, 618, 871, 768, 170, 71, 7, -582, -552, -338, -547, -609, -685, -805, -885, -887, -471, -313, -257, 106, 120, 30, 136, 228, 294, 315, 354, 283, -18, -131, -200, -372, -338, -244, -251, -274, -255, -241, -221, -129, 57, 136, 191, 274, 232, 214, 223, 228, 269, 301, 317, 329, 363, 407, 453, 552, 791, 915, 510, 99, 108, -354, -724, -451, -478, -595, -579, -646, -763, -931, -703, -377, -359, -11, 251, 154, 149, 202, 234, 200, 274, 409, 225, 21, -62, -271, -441, -414, -338, -333, -297, -209, -228, -274, -149, -41, 14, 177, 264, 255, 264, 248, 253, 257, 299, 347, 331, 423, 522, 554, 676, 851, 844, 434, 106, 48, -451, -699, -460, -570, -646, -598, -662, -793, -883, -646, -412, -338, 39, 248, 163, 225, 274, 253, 237, 315, 386, 218, 39, -55, -299, -478, -467, -418, -407, -343, -244, -285, -306, -198, -120, -21, 168, 271, 294, 292, 269, 237, 234, 294, 354, 368, 444, 536, 584, 724, 887, 825, 405, 108, -5, -520, -667, -453, -577, -600, -568, -641, -784, -844, -609, -398, -297, 80, 276, 193, 269, 285, 251, 246, 322, 439, 306, 191, 99, -209, -439, -485, -510, -492, -386, -280, -294, -320, -257, -244, -184, 32, 182, 278, 354, 324, 276, 223, 237, 303, 338, 437, 533, 554, 644, 782, 883, 816, 368, 103, -28, -600, -609, -441, -605, -549, -563, -639, -747, -800, -556, -377, -267, 154, 274, 212, 329, 271, 257, 264, 320, 451, 297, 230, 145, -202, -402, -476, -549, -529, -416, -315, -290, -297, -260, -264, -212, -2, 124, 248, 349, 308, 280, 221, 186, 237, 267, 352, 455, 522, 623, 697, 802, 908, 591, 152, 103, -299, -683, -478, -540, -614, -570, -625, -685, -821, -715, -437, -349, -87, 264, 225, 255, 324, 269, 271, 287, 421, 409, 264, 260, 85, -182, -290, -395, -510, -524, -483, -457, -451, -395, -326, -303, -202, -83, -5, 126, 200, 214, 253, 225, 221, 225, 212, 287, 313, 375, 524, 609, 747, 906, 949, 763, 234, 44, -147, -667, -515, -386, -586, -559, -607, -745, -828, -814, -494, -267, -147, 262, 308, 177, 297, 269, 276, 356, 430, 501, 326, 177, 97, -166, -310, -271, -379, -462, -439, -483, -529, -499, -382, -278, -195, -34, 34, 32, 120, 147, 163, 218, 228, 228, 198, 195, 241, 264, 393, 568, 710, 892, 979, 1023, 834, 193, 16, -69, -635, -485, -299, -616, -651, -717, -894, -890, -837, -478, -168, -126, 186, 285, 90, 214, 352, 345, 455, 515, 462, 303, 62, -7, -113, -264, -163, -161, -379, -460, -531, -653, -600, -444, -276, -143, -85, -46, -80, -129, -39, 87, 145, 225, 237, 129, 67, 76, 145, 303, 448, 628, 805, 837, 903, 998, 936, 538, 69, 46, -234, -683, -448, -434, -821, -786, -793, -947, -828, -630, -354, -124, -78, 103, 230, 113, 241, 487, 418, 425, 494, 308, 145, 46, -53, -80, -152, -215, -267, -251, -205, -267, -230, -161, -115, -14, 64, 126, 145, 168, 223, 223, 214, 216, 214, 260, 232, 218, 294, 310, 361, 487, 618, 871, 768, 170, 71, 7, -582, -552, -338, -547, -609, -685, -805, -885, -887, -471, -313, -257, 106, 120, 30, 136, 228, 294, 315, 354, 283, -18, -131, -200, -372, -338, -244, -251, -274, -255, -241, -221, -129, 57, 136, 191, 274, 232, 214, 223, 228, 269, 301, 317, 329, 363, 407, 453, 552, 791, 915, 510, 99, 108, -354, -724, -451, -478, -595, -579, -646, -763, -931, -703, -377, -359, -11, 251, 154, 149, 202, 234, 200, 274, 409, 225, 21, -62, -271, -441, -414, -338, -333, -297, -209, -228, -274, -149, -41, 14, 177, 264, 255, 264, 248, 253, 257, 299, 347, 331, 423, 522, 554, 676, 851, 844, 434, 106, 48, -451, -699, -460, -570, -646, -598, -662, -793, -883, -646, -412, -338, 39, 248, 163, 225, 274, 253, 237, 315, 386, 218, 39, -55, -299, -478, -467, -418, -407, -343, -244, -285, -306, -198, -120, -21, 168, 271, 294, 292, 269, 237, 234, 294, 354, 368, 444, 536, 584, 724, 887, 825, 405, 108, -5, -520, -667, -453, -577, -600, -568, -641, -784, -844, -609, -398, -297, 80, 276, 193, 269, 285, 251, 246, 322, 439, 306, 191, 99, -209, -439, -485, -510, -492, -386, -280, -294, -320, -257, -244, -184, 32, 182, 278, 354, 324, 276, 223, 237, 303, 338, 437, 533, 554, 644, 782, 883, 816, 368, 103, -28, -600, -609, -441, -605, -549, -563, -639, -747, -800, -556, -377, -267, 154, 274, 212, 329, 271, 257, 264, 320, 451, 297, 230, 145, -202, -402, -476, -549, -529, -416, -315, -290, -297, -260, -264, -212, -2, 124, 248, 349, 308, 280, 221, 186, 237, 267, 352, 455, 522, 623, 697, 802, 908, 591, 152, 103, -299, -683, -478, -540, -614, -570, -625, -685, -821, -715, -437, -349, -87, 264, 225, 255, 324, 269, 271, 287, 421, 409, 264, 260, 85, -182, -290, -395, -510, -524, -483, -457, -451, -395, -326, -303, -202, -83, -5, 126, 200, 214, 253, 225, 221, 225, 212, 287, 313, 375, 524, 609, 747, 906, 949, 763, 234, 44, -147, -667, -515, -386, -586, -559, -607, -745, -828, -814, -494, -267, -147, 262, 308, 177, 297, 269, 276, 356, 430, 501, 326, 177, 97, -166, -310, -271, -379, -462, -439, -483, -529, -499, -382, -278, -195, -34, 34, 32, 120, 147, 163, 218, 228, 228, 198, 195, 241, 264, 393, 568, 710, 892, 979, 1023, 834, 193, 16, -69, -635, -485, -299, -616, -651, -717, -894, -890, -837, -478, -168, -126, 186, 285, 90, 214, 352, 345, 455, 515, 462, 303, 62, -7, -113, -264, -163, -161, -379, -460, -531, -653, -600, -444, -276, -143, -85, -46, -80, -129, -39, 87, 145, 225, 237, 129, 67, 76, 145, 303, 448, 628, 805, 837, 903, 998, 936, 538, 69, 46, -234, -683, -448, -434, -821, -786, -793, -947, -828, -630, -354, -124, -78, 103, 230, 113, 241, 487, 418, 425, 494, 308, 145, 46, -53, -80, -152, -215};

static void test_bessel(void *param)
{
    ESP_LOGI(TAG, "Bessel Test ###############################################");

    static const double eps = 0.0000001f;
    double val;
    double prev = 0;
    float z;

    for (int j = 0; j < 50*10 + 1; j++)
    {
        z = j / 10.0f;
        for (int i = 2;; i++)
        {
            val = I0(z, i);
            if (fabs(val - prev) < eps)
            {
                printf("iter: %3d -> I0(%.2f) = %.15lf\n", i, z, val);
                break;
            }

            prev = val;
        }
    }

    vTaskDelete(NULL);
}

static void test_window(void *param)
{
    ESP_LOGI(TAG, "Window Test ###############################################");
    // Rectangular
    printf("wRectangular = [");
    window_rectangular(window, NFFT);
    for (int i = 0; i < NFFT - 1; i++)
    {
        printf("%.4f,", window[i]);
    }
    printf("%.4f]';\n", window[NFFT-1]);

    // Hamming
    printf("wHamming = [");
    window_hamming(window, NFFT);
    for (int i = 0; i < NFFT - 1; i++)
    {
        printf("%.4f,", window[i]);
    }
    printf("%.4f]';\n", window[NFFT-1]);

    // Kaiser
    printf("wKaiser (%.2f) = [", kaiserBeta);
    window_kaiser(window, NFFT, kaiserBeta);
    for (int i = 0; i < NFFT - 1; i++)
    {
        printf("%.4f,", window[i]);
    }
    printf("%.4f]';\n", window[NFFT-1]);
    
    vTaskDelete(NULL);
}

static void test_frequency_estimation(void *param)
{
    // ESP_LOGI(TAG, "Free bytes %u from %u", uxTaskGetStackHighWaterMark(NULL), STACK_SIZE);
    ESP_LOGI(TAG, "Frequency estimation Test #################################");
    static const int f = 3072;
    static fft_config_t *fft_plan;

    // Create the FFT config structure
    fft_plan = fft_init(NFFT, FFT_REAL, FFT_FORWARD, NULL, NULL);

    // Generate window
    window_kaiser(window, NFFT, kaiserBeta);

    // Fill array with some data
    for (int k = 0 ; k < fft_plan->size ; k++)
    {
        // Maybe ADC has 10 bits res like Arduino? (scaling)
        // Generate sine wave from 0 to 1023 in y-axis
        fft_plan->input[k] = (float) 1023*(1 + sin(2*M_PI*k*f/FS))/2;
    }
    apply_window(fft_plan->input, window, fft_plan->size);

    // Execute transformation
    fft_execute(fft_plan);

    // Calculate the periodogram
    uint32_t Pxx[NFFT/2];
    uint32_t resq, imsq;
    for (int k = 0 ; k < fft_plan->size / 2 ; k++)
    {
        resq = fft_plan->output[2*k]*fft_plan->output[2*k];
        imsq = fft_plan->output[2*k+1]*fft_plan->output[2*k+1];
        Pxx[k] = (resq >> BIT_SCALE) + (imsq >> BIT_SCALE);
    }

    // Frequency estimation
    Tone_t *tone = (Tone_t *)malloc(sizeof(Tone_t));
    find_tone(Pxx, sizeof(Pxx)/sizeof(Pxx[0]), FS, tone);
    ESP_LOGI(TAG, "f_est = %d Hz", tone->m_frequency);
    ESP_LOGI(TAG, "f_act = %d Hz", f);

    // Don't forget to clean up at the end to free all the memory that was allocated
    fft_destroy(fft_plan);
    free(tone);

    vTaskDelete(NULL);
}

static void test_formant_vowel_estimation(void *param)
{
    ESP_LOGI(TAG, "Formant estimation Test ###################################");
    static fft_config_t *fft_plan;

    // Create the FFT config structure
    fft_plan = fft_init(NFFT, FFT_REAL, FFT_FORWARD, NULL, NULL);

    // Generate window
    window_kaiser(window, NFFT, kaiserBeta);

    // Fill array with some data
    srand(time(NULL));
    for (int k = 0 ; k < fft_plan->size ; k++)
    {
        fft_plan->input[k] = (float) data[k];
    }
    apply_window(fft_plan->input, window, fft_plan->size);

    // Execute transformation
    fft_execute(fft_plan);

    // Calculate the periodogram
    uint32_t Pxx[NFFT/2];
    uint32_t resq, imsq;
    for (int k = 0 ; k < fft_plan->size / 2 ; k++)
    {
        resq = fft_plan->output[2*k]*fft_plan->output[2*k];
        imsq = fft_plan->output[2*k+1]*fft_plan->output[2*k+1];

        Pxx[k] = (resq >> BIT_SCALE) + (imsq >> BIT_SCALE);
    }

    // Frequency estimation
    Tone_t *tone1 = (Tone_t *) malloc(sizeof(Tone_t));
    Tone_t *tone2 = (Tone_t *) malloc(sizeof(Tone_t));
    
    find_formants(Pxx, sizeof(Pxx)/sizeof(Pxx[0]), FS, tone1, tone2);
    ESP_LOGI(TAG, "f1 = %zu Hz", tone1->m_frequency);
    ESP_LOGI(TAG, "f2 = %zu Hz", tone2->m_frequency);
    
    // Vowel estimation
    Vowel_t *vowelEst = (Vowel_t *) malloc(sizeof(Vowel_t));
    uint32_t err = vowel_estimation(tone1, tone2, vowelEst);
    ESP_LOGI(TAG, "Vowel: %c; error = %zu", vowelEst->m_vowel, err);

    // Don't forget to clean up at the end to free all the memory that was allocated
    fft_destroy(fft_plan);
    free(tone1);
    free(tone2);

    vTaskDelete(NULL);
}

void app_main(void)
{
    //xTaskCreate(test_bessel, "bessel_task", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    //xTaskCreate(test_window, "window_task", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    //xTaskCreate(test_frequency_estimation, "freq_est_task", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(test_formant_vowel_estimation, "form_est_task", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
}
