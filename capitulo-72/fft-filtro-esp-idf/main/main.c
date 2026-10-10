#include <math.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_dsp.h"

#define N      1024                       /* pontos da FFT */
#define TAXA   16000.0f                   /* Hz */
#define TAPS   63                         /* coeficientes do filtro FIR */
#define CORTE  2000.0f                    /* Hz */
static const char *TAG = "dsp";

/* O sinal tem TAPS - 1 amostras a mais: é o tempo que o filtro leva para encher. */
static float sinal[N + TAPS - 1], filtrado[N + TAPS - 1];
static float janela[N], h[TAPS], atraso[TAPS];
static __attribute__((aligned(16))) float fft[N * 2];   /* re, im intercalados */
static float db[N / 2];

/* Janela de Hann, FFT complexa e módulo em dB (0 dB = seno de amplitude 1). */
static void espectro(const float *x)
{
    for (int i = 0; i < N; i++) {
        fft[2 * i] = x[i] * janela[i];
        fft[2 * i + 1] = 0;
    }
    dsps_fft2r_fc32(fft, N);
    dsps_bit_rev_fc32(fft, N);
    for (int k = 0; k < N / 2; k++) {
        float re = fft[2 * k], im = fft[2 * k + 1];
        db[k] = 10 * log10f((re * re + im * im) / (N * N / 16.0f) + 1e-12f);
    }
}

static float nivel(float freq)
{
    return db[(int)lroundf(freq * N / TAXA)];
}

static void imprime(const char *rotulo)
{
    for (int k = 0; k < N / 2; k++) {     /* para desenhar o gráfico no PC */
        if (k % 32 == 0) {
            printf("\n%s:", rotulo);
        }
        printf(" %d", (int)db[k]);
    }
    printf("\n");
}

void app_main(void)
{
    for (int i = 0; i < N + TAPS - 1; i++) {   /* 1 kHz forte + 3,5 kHz mais fraco */
        float t = i / TAXA;
        sinal[i] = sinf(2 * M_PI * 1000 * t) + 0.5f * sinf(2 * M_PI * 3500 * t);
    }
    dsps_wind_hann_f32(janela, N);
    dsps_fft2r_init_fc32(NULL, N);        /* tabela de senos interna */

    espectro(sinal);
    ESP_LOGI(TAG, "antes:  1 kHz %6.1f dB, 3,5 kHz %6.1f dB",
             nivel(1000), nivel(3500));
    imprime("ANTES");

    /* Passa-baixas de 2 kHz: sinc ideal truncado e suavizado pela janela de Hann. */
    float fc = CORTE / TAXA, m = (TAPS - 1) / 2.0f, soma = 0;
    for (int k = 0; k < TAPS; k++) {
        float x = k - m;
        h[k] = (x == 0 ? 2 * fc : sinf(2 * M_PI * fc * x) / (M_PI * x)) *
               (0.5f - 0.5f * cosf(2 * M_PI * k / (TAPS - 1)));
        soma += h[k];
    }
    for (int k = 0; k < TAPS; k++) {
        h[k] /= soma;                     /* ganho 1 em 0 Hz */
    }
    fir_f32_t fir;
    dsps_fir_init_f32(&fir, h, atraso, TAPS);
    dsps_fir_f32(&fir, sinal, filtrado, N + TAPS - 1);

    espectro(filtrado + TAPS - 1);        /* descarta o transitório */
    ESP_LOGI(TAG, "depois: 1 kHz %6.1f dB, 3,5 kHz %6.1f dB",
             nivel(1000), nivel(3500));
    imprime("DEPOIS");
}
