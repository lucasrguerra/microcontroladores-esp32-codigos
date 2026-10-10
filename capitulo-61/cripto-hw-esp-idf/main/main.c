#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "bootloader_random.h"
#include "psa/crypto.h"
#include "soc/soc_caps.h"

static const char *TAG = "cripto";
static uint8_t bloco[32 * 1024];               /* 32 KB para medir o SHA-256 */

static void mostra(const char *nome, const uint8_t *b, size_t n)
{
    char txt[2 * 32 + 1] = "";
    for (size_t i = 0; i < n && i < 32; i++) {
        sprintf(txt + 2 * i, "%02x", b[i]);
    }
    ESP_LOGI(TAG, "%s %s", nome, txt);
}

static void aleatorios(void)
{
    uint8_t chave[16];
    bootloader_random_enable();                /* ruído do ADC alimenta o TRNG */
    esp_fill_random(chave, sizeof(chave));
    bootloader_random_disable();
    mostra("sorteio:", chave, sizeof(chave));
}

static void resumo_sha256(void)
{
    uint8_t hash[32];
    size_t n;
    memset(bloco, 0xA5, sizeof(bloco));
    int64_t t0 = esp_timer_get_time();
    psa_hash_compute(PSA_ALG_SHA_256, bloco, sizeof(bloco), hash, sizeof(hash), &n);
    int64_t us = esp_timer_get_time() - t0;
    mostra("sha256:", hash, 8);
    ESP_LOGI(TAG, "32 KB em %lld us (%lld KB/s)", us, 32LL * 1000000 / us);
}

static void cifra_gcm(void)
{
    psa_key_attributes_t at = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_type(&at, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&at, 256);
    psa_set_key_algorithm(&at, PSA_ALG_GCM);
    psa_set_key_usage_flags(&at, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_key_id_t id;
    if (psa_generate_key(&at, &id) != PSA_SUCCESS) {   /* chave sorteada na RAM */
        ESP_LOGE(TAG, "falha ao gerar a chave");
        return;
    }

    const char *msg = "abrir portao 3";
    uint8_t nonce[12], cifrado[64], volta[64];
    size_t nc, nv;
    esp_fill_random(nonce, sizeof(nonce));     /* nunca repetir com a mesma chave */
    psa_aead_encrypt(id, PSA_ALG_GCM, nonce, sizeof(nonce), NULL, 0,
                     (const uint8_t *)msg, strlen(msg),
                     cifrado, sizeof(cifrado), &nc);
    mostra("cifrado:", cifrado, nc);

    psa_status_t st = psa_aead_decrypt(id, PSA_ALG_GCM, nonce, sizeof(nonce), NULL, 0,
                                       cifrado, nc, volta, sizeof(volta), &nv);
    ESP_LOGI(TAG, "decifrado (%d): %.*s", (int)st, (int)nv, (char *)volta);

    cifrado[0] ^= 0x01;                        /* um bit trocado no caminho */
    st = psa_aead_decrypt(id, PSA_ALG_GCM, nonce, sizeof(nonce), NULL, 0,
                          cifrado, nc, volta, sizeof(volta), &nv);
    ESP_LOGW(TAG, "adulterado: status %d (%s)", (int)st,
             st == PSA_ERROR_INVALID_SIGNATURE ? "rejeitado" : "aceito?!");
    psa_destroy_key(id);
}

void app_main(void)
{
    ESP_ERROR_CHECK(psa_crypto_init() == PSA_SUCCESS ? ESP_OK : ESP_FAIL);
#if SOC_AES_SUPPORTED
    ESP_LOGI(TAG, "acelerador AES: sim");
#else
    ESP_LOGI(TAG, "acelerador AES: não (AES em software)");
#endif
#if SOC_SHA_SUPPORTED
    ESP_LOGI(TAG, "acelerador SHA: sim");
#endif
    aleatorios();
    resumo_sha256();
    cifra_gcm();
}
