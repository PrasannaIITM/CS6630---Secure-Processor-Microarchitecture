#include <stdio.h>
#include <stdint.h>
#include <string.h>

extern void bitwise_xor(uint8_t *output, const uint8_t *input_1, const uint8_t *input_2, int8_t length);
extern void F0(uint8_t *dst, const uint8_t *src, const uint8_t *RK);
extern void F1(uint8_t *dst, const uint8_t *src, const uint8_t *RK);
extern void gfn4(uint8_t *block, const uint8_t *RK, int8_t r);
extern void gfn4_inv(uint8_t *block, const uint8_t *RK, int8_t r);
extern void double_swap(uint8_t *X);
extern void gen_con_set(uint8_t *con, const uint8_t *iv, int8_t lk);
extern void key_scheduling(uint8_t *key, uint8_t *RK);
