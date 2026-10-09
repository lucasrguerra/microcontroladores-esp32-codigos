#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_vfs_fat.h"

static const char *BASE = "/dados";
static wl_handle_t s_wl = WL_INVALID_HANDLE;

/* Acrescenta uma linha e só retorna quando ela está gravada na flash. */
static void registrar(const char *linha)
{
    FILE *f = fopen("/dados/registro.txt", "a");
    if (f == NULL) {
        perror("fopen");
        return;
    }
    fprintf(f, "%s\n", linha);
    fflush(f);                          /* da biblioteca C para o FatFs */
    fsync(fileno(f));                   /* do FatFs para a flash */
    fclose(f);
}

void app_main(void)
{
    esp_vfs_fat_mount_config_t cfg = {
        .max_files = 4,
        .format_if_mount_failed = true,         /* primeira vez: formata */
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE,
    };
    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_mount_rw_wl(BASE, "storage", &cfg, &s_wl));

    char linha[48];
    for (int i = 1; i <= 3; i++) {
        snprintf(linha, sizeof linha, "medida %d: %d", i, 20 + i);
        registrar(linha);
    }

    struct stat st;
    if (stat("/dados/registro.txt", &st) == 0) {
        printf("registro.txt tem %ld bytes\n", (long)st.st_size);
    }
    FILE *f = fopen("/dados/registro.txt", "r");
    while (f && fgets(linha, sizeof linha, f)) {
        printf("  %s", linha);
    }
    if (f) {
        fclose(f);
    }

    uint64_t total = 0, livre = 0;
    ESP_ERROR_CHECK(esp_vfs_fat_info(BASE, &total, &livre));
    printf("FAT: %llu KB no total, %llu KB livres\n", total / 1024, livre / 1024);
    ESP_ERROR_CHECK(esp_vfs_fat_spiflash_unmount_rw_wl(BASE, s_wl));
}
