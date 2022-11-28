#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "t_box.h"
#include "constants.h"

void bitwise_xor(uint8_t *output, const uint8_t *input_1, const uint8_t *input_2, int8_t length)
{
    while (length-- > 0)
    {
        *output++ = *input_1++ ^ *input_2++;
    }
}

void F0(uint8_t *dst, const uint8_t *src, const uint8_t *RK)
{
    /*
        ref: https://datatracker.ietf.org/doc/html/rfc6114#section-4.2
    */
    uint8_t x[4], y[4];
    bitwise_xor(x, src, RK, 4);

    uint32_t fout = T0_F0[x[0]] ^ T1_F0[x[1]] ^ T2_F0[x[2]] ^ T3_F0[x[3]];

    y[0] = fout & 0xFF;
    y[1] = (fout >> 8) & 0xFF;
    y[2] = (fout >> 16) & 0xFF;
    y[3] = (fout >> 24) & 0xFF;

    memcpy(dst + 0, src + 0, 4);
    bitwise_xor(dst + 4, src + 4, y, 4);
}

void F1(uint8_t *dst, const uint8_t *src, const uint8_t *RK)
{
    /*
        ref: https://datatracker.ietf.org/doc/html/rfc6114#section-4.2
    */

    uint8_t x[4], y[4];

    bitwise_xor(x, src, RK, 4);

    uint32_t fout = T0_F1[x[0]] ^ T1_F1[x[1]] ^ T2_F1[x[2]] ^ T3_F1[x[3]];

    y[0] = fout & 0xFF;
    y[1] = (fout >> 8) & 0xFF;
    y[2] = (fout >> 16) & 0xFF;
    y[3] = (fout >> 24) & 0xFF;

    memcpy(dst + 0, src + 0, 4);
    bitwise_xor(dst + 4, src + 4, y, 4);
}

void gfn4(uint8_t *block, const uint8_t *RK, int8_t r)
{
    /*
        ref: https://datatracker.ietf.org/doc/html/rfc6114#section-4.1
    */
    uint8_t fin[16];

    memcpy(fin, block, 16);
    while (r-- > 0)
    {
        F0(block + 0, fin + 0, RK + 0);
        F1(block + 8, fin + 8, RK + 4);
        RK += 8;
        if (r)
        {
            memcpy(fin + 0, block + 4, 12);
            memcpy(fin + 12, block + 0, 4);
        }
    }
}

void gfn4_inv(uint8_t *block, const uint8_t *RK, int8_t r)
{
    /*
        ref: https://datatracker.ietf.org/doc/html/rfc6114#section-4.1
    */

    uint8_t fin[16];

    RK += (r - 1) * 8;
    memcpy(fin, block, 16);
    while (r-- > 0)
    {
        F0(block + 0, fin + 0, RK + 0);
        F1(block + 8, fin + 8, RK + 4);
        RK -= 8;
        if (r)
        {
            memcpy(fin + 0, block + 12, 4);
            memcpy(fin + 4, block + 0, 12);
        }
    }
}

void double_swap(uint8_t *X)
{
    /*
        ref: https://datatracker.ietf.org/doc/html/rfc6114#section-6.1

        Y = Sigma(X)
          = X[7-63] | X[121-127] | X[0-6] | X[64-120]
    */

    uint8_t Y[16];

    Y[0] = (X[0] << 7) | (X[1] >> 1);
    Y[1] = (X[1] << 7) | (X[2] >> 1);
    Y[2] = (X[2] << 7) | (X[3] >> 1);
    Y[3] = (X[3] << 7) | (X[4] >> 1);
    Y[4] = (X[4] << 7) | (X[5] >> 1);
    Y[5] = (X[5] << 7) | (X[6] >> 1);
    Y[6] = (X[6] << 7) | (X[7] >> 1);
    Y[7] = (X[7] << 7) | (X[15] & 0x7fU);
    Y[8] = (X[8] >> 7) | (X[0] & 0xfeU);
    Y[9] = (X[9] >> 7) | (X[8] << 1);
    Y[10] = (X[10] >> 7) | (X[9] << 1);
    Y[11] = (X[11] >> 7) | (X[10] << 1);
    Y[12] = (X[12] >> 7) | (X[11] << 1);
    Y[13] = (X[13] >> 7) | (X[12] << 1);
    Y[14] = (X[14] >> 7) | (X[13] << 1);
    Y[15] = (X[15] >> 7) | (X[14] << 1);

    memcpy(X, Y, 16);
}

void gen_con_set(uint8_t *con, const uint8_t *iv, int8_t lk)
{
    /*
        ref: https://datatracker.ietf.org/doc/html/rfc6114#section-6.6
    */
    uint8_t t[2];
    uint8_t tmp;

    memcpy(t, iv, 2);

    while (lk-- > 0)
    {
        con[0] = t[0] ^ 0xb7U;
        con[1] = t[1] ^ 0xe1U;
        con[2] = ~((t[0] << 1) | (t[1] >> 7));
        con[3] = ~((t[1] << 1) | (t[0] >> 7));
        con[4] = ~t[0] ^ 0x24U;
        con[5] = ~t[1] ^ 0x3fU;
        con[6] = t[1];
        con[7] = t[0];
        con += 8;

        if (t[1] & 0x01U)
        {
            t[0] ^= 0xa8U;
            t[1] ^= 0x30U;
        }
        tmp = t[0] << 7;
        t[0] = (t[0] >> 1) | (t[1] << 7);
        t[1] = (t[1] >> 1) | tmp;
    }
}

void key_scheduling(uint8_t *key, uint8_t *RK)
{
    /*
        ref: https://datatracker.ietf.org/doc/html/rfc6114#section-6.3
    */
    const uint8_t iv[2] = {0x42U, 0x8aU};
    uint8_t L[16];
    uint8_t CON_128[4 * 60];
    uint8_t i;

    gen_con_set(CON_128, iv, 30);
    memcpy(L, key, KEY_SZ);
    gfn4(L, CON_128, 12);

    memcpy(RK, key, 8);
    RK += 8;
    for (i = 0; i < 9; i++)
    {
        bitwise_xor(RK, L, CON_128 + i * 16 + (4 * 24), 16);
        if (i % 2)
        {
            bitwise_xor(RK, RK, key, 16);
        }
        double_swap(L);
        RK += 16;
    }
    memcpy(RK, key + 8, 8);
}
