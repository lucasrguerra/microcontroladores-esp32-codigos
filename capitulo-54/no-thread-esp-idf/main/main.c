#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_vfs_eventfd.h"
#include "nvs_flash.h"
#include "esp_openthread.h"
#include "openthread/dataset.h"
#include "openthread/instance.h"
#include "openthread/ip6.h"
#include "openthread/thread.h"

static const char *TAG = "thread";

/* Rede de bancada: todos os nós precisam dos mesmos parâmetros. */
#define CANAL    15
#define PAN_ID   0x1234
static const uint8_t XPANID[8] = {0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xfe};
static const uint8_t CHAVE[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
static const uint8_t PREFIXO[8] = {0xfd, 0x00, 0xdb, 0x80, 0x00, 0x00, 0x00, 0x00};

static void ao_mudar(otChangedFlags flags, void *ctx)
{
    otInstance *ot = ctx;
    if (flags & OT_CHANGED_THREAD_ROLE) {
        otDeviceRole papel = otThreadGetDeviceRole(ot);
        ESP_LOGI(TAG, "papel: %s", otThreadDeviceRoleToString(papel));
    }
}

static void montar_rede(otInstance *ot)
{
    otOperationalDataset ds;
    memset(&ds, 0, sizeof(ds));
    ds.mActiveTimestamp.mSeconds = 1;
    ds.mComponents.mIsActiveTimestampPresent = true;
    ds.mChannel = CANAL;
    ds.mComponents.mIsChannelPresent = true;
    ds.mPanId = PAN_ID;
    ds.mComponents.mIsPanIdPresent = true;
    memcpy(ds.mExtendedPanId.m8, XPANID, sizeof(XPANID));
    ds.mComponents.mIsExtendedPanIdPresent = true;
    memcpy(ds.mNetworkKey.m8, CHAVE, sizeof(CHAVE));
    ds.mComponents.mIsNetworkKeyPresent = true;
    strcpy(ds.mNetworkName.m8, "bancada");
    ds.mComponents.mIsNetworkNamePresent = true;
    memcpy(ds.mMeshLocalPrefix.m8, PREFIXO, sizeof(PREFIXO));
    ds.mComponents.mIsMeshLocalPrefixPresent = true;

    ESP_ERROR_CHECK(otDatasetSetActive(ot, &ds) == OT_ERROR_NONE ? ESP_OK : ESP_FAIL);
    otSetStateChangedCallback(ot, ao_mudar, ot);
    otIp6SetEnabled(ot, true);             /* "ifconfig up" */
    otThreadSetEnabled(ot, true);          /* "thread start" */
}

static void tarefa_thread(void *arg)
{
    esp_openthread_platform_config_t cfg = {
        .radio_config = { .radio_mode = RADIO_MODE_NATIVE },
        .host_config = { .host_connection_mode = HOST_CONNECTION_MODE_NONE },
        .port_config = {
            .storage_partition_name = "nvs",
            .netif_queue_size = 10,
            .task_queue_size = 10,
        },
    };
    ESP_ERROR_CHECK(esp_openthread_init(&cfg));
    montar_rede(esp_openthread_get_instance());

    esp_openthread_launch_mainloop();      /* só volta se a pilha parar */
    esp_openthread_deinit();
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_vfs_eventfd_config_t efd = { .max_fds = 3 };
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&efd));
    xTaskCreate(tarefa_thread, "ot", 10240, NULL, 5, NULL);
}
