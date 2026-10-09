#pragma once

#include <stdint.h>
#include "esp_err.h"

/* Pisca um LED no GPIO indicado, trocando de estado a cada periodo_ms. */
esp_err_t pisca_iniciar(int gpio, uint32_t periodo_ms);
