#include <stdint.h>
#include "ulp_lp_core.h"
#include "ulp_lp_core_utils.h"
#include "ulp_lp_core_gpio.h"

#define PINO   LP_IO_NUM_0                  /* GPIO0: um botão para o GND */
#define LIMITE 30                           /* acorda a cada 30 rodadas */

/* Variáveis globais viram símbolos ulp_* visíveis para o núcleo principal. */
uint32_t rodadas;
uint32_t motivo;                            /* 1 = pino, 2 = limite */
uint32_t nivel_anterior = 1;

int main(void)
{
    rodadas++;
    uint32_t nivel = ulp_lp_core_gpio_get_level(PINO);
    if (nivel != nivel_anterior) {
        nivel_anterior = nivel;
        if (nivel == 0) {                   /* apertou */
            motivo = 1;
            ulp_lp_core_wakeup_main_processor();
        }
    } else if (rodadas % LIMITE == 0) {
        motivo = 2;
        ulp_lp_core_wakeup_main_processor();
    }
    return 0;                               /* dorme até o próximo período */
}
