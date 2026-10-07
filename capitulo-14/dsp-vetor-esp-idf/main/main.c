#include <stdio.h>
#include <inttypes.h>
#include "esp_cpu.h"
#include "esp_dsp.h"
#include "sdkconfig.h"

#define N 1024

static float a[N] __attribute__((aligned(16)));
static float b[N] __attribute__((aligned(16)));

void app_main(void)
{
    for (int i = 0; i < N; i++) {
        a[i] = (float)i / N;
        b[i] = 1.0f - a[i];
    }
    float r_ansi = 0, r_otim = 0;

    uint32_t t0 = esp_cpu_get_cycle_count();
    dsps_dotprod_f32_ansi(a, b, &r_ansi, N);
    uint32_t c_ansi = esp_cpu_get_cycle_count() - t0;

    // dsps_dotprod_f32 aponta para a versão otimizada da série
    // (com instruções vetoriais no S3)
    t0 = esp_cpu_get_cycle_count();
    dsps_dotprod_f32(a, b, &r_otim, N);
    uint32_t c_otim = esp_cpu_get_cycle_count() - t0;

    printf("%s: C puro %" PRIu32 " ciclos, otimizada %" PRIu32 " ciclos (%.1fx)\n",
           CONFIG_IDF_TARGET, c_ansi, c_otim, (float)c_ansi / c_otim);
    printf("Resultados: %.4f e %.4f\n", r_ansi, r_otim);
}
