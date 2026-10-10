#include <math.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_audio_enc_default.h"

#define TAXA     16000                    /* amostras por segundo */
#define AMOSTRAS (TAXA * 2)              /* 2 s de áudio mono de 16 bits */
static const char *TAG = "audio";

/* Sinal parecido com voz: fundamental de 140 Hz com harmônicos e vibrato,
   sílabas de 250 ms separadas por silêncio. */
static void sinal_de_teste(int16_t *pcm)
{
    float fase = 0;
    for (int i = 0; i < AMOSTRAS; i++) {
        float t = (float)i / TAXA;
        fase += 2 * M_PI * (140 + 10 * sinf(2 * M_PI * 5 * t)) / TAXA;
        float s = 0;
        for (int h = 1; h <= 8; h++) {
            s += sinf(h * fase) / h;
        }
        float u = fmodf(t, 0.4f);                       /* posição na sílaba */
        float silaba = u < 0.25f ? sinf(M_PI * u / 0.25f) : 0;
        pcm[i] = (int16_t)(6000 * s * silaba);
    }
}

static void comprime(const char *nome, esp_audio_type_t tipo, void *cfg, int tam_cfg,
                     int16_t *pcm)
{
    esp_audio_enc_config_t enc_cfg = {.type = tipo, .cfg = cfg, .cfg_sz = tam_cfg};
    esp_audio_enc_handle_t enc;
    if (esp_audio_enc_open(&enc_cfg, &enc) != ESP_AUDIO_ERR_OK) {
        ESP_LOGE(TAG, "%s: falha ao abrir o codificador", nome);
        return;
    }
    int quadro_in, quadro_out;
    esp_audio_enc_get_frame_size(enc, &quadro_in, &quadro_out);
    uint8_t *saida = malloc(quadro_out);
    int total = 0, bytes_pcm = AMOSTRAS * 2;
    int64_t t0 = esp_timer_get_time();
    for (int pos = 0; pos + quadro_in <= bytes_pcm; pos += quadro_in) {
        esp_audio_enc_in_frame_t in = {.buffer = (uint8_t *)pcm + pos,
                                       .len = quadro_in};
        esp_audio_enc_out_frame_t out = {.buffer = saida, .len = quadro_out};
        if (esp_audio_enc_process(enc, &in, &out) == ESP_AUDIO_ERR_OK) {
            total += out.encoded_bytes;
        }
    }
    int64_t ms = (esp_timer_get_time() - t0) / 1000;
    ESP_LOGI(TAG, "%-5s quadro %4d B: %6d bytes, %5.1f kbit/s, %4.1f:1, %lld ms",
             nome, quadro_in, total, total * 8 / 2000.0,
             (float)bytes_pcm / total, ms);
    free(saida);
    esp_audio_enc_close(enc);
}

void app_main(void)
{
    int16_t *pcm = malloc(AMOSTRAS * 2);
    sinal_de_teste(pcm);
    esp_audio_enc_register_default();    /* registra todos os codificadores */
    ESP_LOGI(TAG, "PCM: %d bytes, 256.0 kbit/s", AMOSTRAS * 2);

    esp_g711_enc_config_t g711 = ESP_G711_ENC_CONFIG_DEFAULT();
    g711.sample_rate = TAXA;
    comprime("G.711", ESP_AUDIO_TYPE_G711A, &g711, sizeof(g711), pcm);

    esp_adpcm_enc_config_t adpcm = ESP_ADPCM_ENC_CONFIG_DEFAULT();
    adpcm.sample_rate = TAXA;
    comprime("ADPCM", ESP_AUDIO_TYPE_ADPCM, &adpcm, sizeof(adpcm), pcm);

    esp_aac_enc_config_t aac = ESP_AAC_ENC_CONFIG_DEFAULT();
    aac.sample_rate = TAXA;
    aac.channel = ESP_AUDIO_MONO;
    aac.bitrate = 32000;
    comprime("AAC", ESP_AUDIO_TYPE_AAC, &aac, sizeof(aac), pcm);

    esp_opus_enc_config_t opus = ESP_OPUS_ENC_CONFIG_DEFAULT();
    opus.sample_rate = TAXA;
    opus.channel = ESP_AUDIO_MONO;
    opus.bitrate = 16000;                 /* voz inteligível com 16 kbit/s */
    comprime("Opus", ESP_AUDIO_TYPE_OPUS, &opus, sizeof(opus), pcm);
    free(pcm);
}
