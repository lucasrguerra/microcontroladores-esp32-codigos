#include <cmath>
#include "esp_log.h"
#include "esp_timer.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model.h"                       // g_model: rede treinada para y = sen(x)

static const char *TAG = "ia";
constexpr int TAM_ARENA = 2000;          // memória de trabalho do interpretador
static uint8_t arena[TAM_ARENA];

extern "C" void app_main(void)
{
    const tflite::Model *modelo = tflite::GetModel(g_model);
    if (modelo->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "versão do modelo %lu incompatível", modelo->version());
        return;
    }
    static tflite::MicroMutableOpResolver<1> operacoes;
    operacoes.AddFullyConnected();       // a única camada que esta rede usa
    static tflite::MicroInterpreter interp(modelo, operacoes, arena, TAM_ARENA);
    if (interp.AllocateTensors() != kTfLiteOk) {
        ESP_LOGE(TAG, "arena pequena demais");
        return;
    }
    TfLiteTensor *ent = interp.input(0);
    TfLiteTensor *sai = interp.output(0);
    ESP_LOGI(TAG, "modelo: %d bytes, arena usada: %u de %d bytes", g_model_len,
             (unsigned)interp.arena_used_bytes(), TAM_ARENA);

    float erro_max = 0;
    int64_t t0 = esp_timer_get_time();
    for (int i = 0; i <= 8; i++) {
        float x = i * 2 * M_PI / 8;
        // int8: valor real = (inteiro - zero_point) * scale
        long q = lroundf(x / ent->params.scale) + ent->params.zero_point;
        ent->data.int8[0] = (int8_t)q;
        interp.Invoke();
        float y = (sai->data.int8[0] - sai->params.zero_point) * sai->params.scale;
        erro_max = fmaxf(erro_max, fabsf(y - sinf(x)));
        ESP_LOGI(TAG, "x = %4.2f   rede: %6.3f   sinf: %6.3f", x, y, sinf(x));
    }
    int64_t us = esp_timer_get_time() - t0;
    ESP_LOGI(TAG, "erro máximo %.3f; 9 inferências em %lld us", erro_max, us);
}
