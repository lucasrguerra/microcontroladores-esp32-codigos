#include <string.h>
#include "sdkconfig.h"
#include "filtro.h"

void filtro_iniciar(filtro_t *f)
{
    memset(f, 0, sizeof(*f));
}

int32_t filtro_aplicar(filtro_t *f, int32_t amostra)
{
    if (f->n == CONFIG_FILTRO_JANELA) {
        f->soma -= f->amostras[f->pos];         /* sai a mais antiga */
    } else {
        f->n++;
    }
    f->amostras[f->pos] = amostra;
    f->soma += amostra;
    f->pos = (f->pos + 1) % CONFIG_FILTRO_JANELA;
    return f->soma / f->n;
}
