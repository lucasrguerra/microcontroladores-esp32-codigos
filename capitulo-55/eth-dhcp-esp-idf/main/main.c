#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_eth.h"
#if CONFIG_ETH_USE_OPENETH
#include "esp_eth_mac_openeth.h"
#endif

static const char *TAG = "eth";

/* PHY das placas oficiais: ESP32-Ethernet-Kit e ESP32-P4-Function-EV-Board. */
#define PHY_ENDERECO  1
#if CONFIG_IDF_TARGET_ESP32P4
#define PHY_RESET     51
#else
#define PHY_RESET     5
#endif

static void ao_evento_eth(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    esp_eth_handle_t eth = *(esp_eth_handle_t *)dados;
    uint8_t mac[6];
    eth_speed_t vel;
    eth_duplex_t duplex;

    switch (id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth, ETH_CMD_G_MAC_ADDR, mac);
        esp_eth_ioctl(eth, ETH_CMD_G_SPEED, &vel);
        esp_eth_ioctl(eth, ETH_CMD_G_DUPLEX_MODE, &duplex);
        ESP_LOGI(TAG, "link ativo, MAC %02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI(TAG, "%s Mbit/s, %s", vel == ETH_SPEED_100M ? "100" : "10",
                 duplex == ETH_DUPLEX_FULL ? "full duplex" : "half duplex");
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "cabo desconectado");
        break;
    default:
        break;
    }
}

static void ao_obter_ip(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
    ip_event_got_ip_t *ev = dados;
    const esp_netif_ip_info_t *ip = &ev->ip_info;
    ESP_LOGI(TAG, "IP " IPSTR ", máscara " IPSTR ", gateway " IPSTR,
             IP2STR(&ip->ip), IP2STR(&ip->netmask), IP2STR(&ip->gw));
}

static esp_eth_handle_t criar_driver(void)
{
    eth_mac_config_t mac_cfg = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_cfg = ETH_PHY_DEFAULT_CONFIG();
    phy_cfg.phy_addr = PHY_ENDERECO;
    phy_cfg.reset_gpio_num = PHY_RESET;
#if CONFIG_ETH_USE_OPENETH
    esp_eth_mac_t *mac = esp_eth_mac_new_openeth(&mac_cfg);   /* só no QEMU */
#else
    eth_esp32_emac_config_t emac_cfg = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&emac_cfg, &mac_cfg);
#endif
    esp_eth_phy_t *phy = esp_eth_phy_new_generic(&phy_cfg);

    esp_eth_config_t cfg = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&cfg, &eth));
    return eth;
}

void app_main(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *netif = esp_netif_new(&netif_cfg);
    esp_eth_handle_t eth = criar_driver();
    ESP_ERROR_CHECK(esp_netif_attach(netif, esp_eth_new_netif_glue(eth)));

    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                               ao_evento_eth, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP,
                                               ao_obter_ip, NULL));
    ESP_ERROR_CHECK(esp_eth_start(eth));
}
