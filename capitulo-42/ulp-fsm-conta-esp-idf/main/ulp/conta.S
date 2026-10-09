#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"

    .bss
    .global contagem
contagem: .long 0                 /* total de rodadas (16 bits úteis) */
    .global passo
passo:    .long 0                 /* rodadas desde o último despertar */

    .text
    .global entry
entry:
    move r3, contagem             /* r3 = endereço de contagem */
    ld   r0, r3, 0
    add  r0, r0, 1
    st   r0, r3, 0
    move r3, passo
    ld   r0, r3, 0
    add  r0, r0, 1
    st   r0, r3, 0
    jumpr acordar, 10, ge         /* passo >= 10: hora de acordar */
    halt                          /* dorme até o próximo período */

acordar:
    move r0, 0
    st   r0, r3, 0                /* zera passo */
espera:
    READ_RTC_FIELD(RTC_CNTL_LOW_POWER_ST_REG, RTC_CNTL_RDY_FOR_WAKEUP)
    and  r0, r0, 1
    jump espera, eq               /* o chip ainda não pode acordar */
    wake
    halt
