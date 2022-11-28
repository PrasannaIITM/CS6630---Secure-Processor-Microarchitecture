#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "t_box.h"
#include "constants.h"
#include "utils.h"

void encrypt(uint8_t *block, uint8_t *round_keys)
{
    const uint8_t *RK = round_keys;
    bitwise_xor(block + 4, block + 4, RK + 0, 4);
    bitwise_xor(block + 12, block + 12, RK + 4, 4);
    RK += 8;

    gfn4(block, RK, NUM_ROUNDS);

    bitwise_xor(block + 4, block + 4, RK + NUM_ROUNDS * 8 + 0, 4);
    bitwise_xor(block + 12, block + 12, RK + NUM_ROUNDS * 8 + 4, 4);
}

void decrypt(uint8_t *block, uint8_t *round_keys)
{
    const uint8_t *RK = round_keys;
    bitwise_xor(block + 4, block + 4, RK + NUM_ROUNDS * 8 + 8, 4);
    bitwise_xor(block + 12, block + 12, RK + NUM_ROUNDS * 8 + 12, 4);
    RK += 8;

    gfn4_inv(block, RK, NUM_ROUNDS);

    bitwise_xor(block + 4, block + 4, RK - 8, 4);
    bitwise_xor(block + 12, block + 12, RK - 4, 4);
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Enter 'e' for encryption or 'd' for decryption\n");
        return 0;
    }
    else if (strcmp(argv[1], "e") != 0 && strcmp(argv[1], "d") != 0)
    {
        printf("%s \n", argv[1]);
        printf("Enter 'e' for encryption or 'd' for decryption\n");
        return 0;
    }
    uint8_t plain_text[BLOCK_SZ] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
    uint8_t secret_key[KEY_SZ] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
    uint8_t expected_cipher_output[BLOCK_SZ] = {0xde, 0x2b, 0xf2, 0xfd, 0x9b, 0x74, 0xaa, 0xcd, 0xf1, 0x29, 0x85, 0x55, 0x45, 0x94, 0x94, 0xfd};

    uint8_t *round_keys, *key = secret_key, *block = expected_cipher_output;
    key_scheduling(key, round_keys);

    if (strcmp(argv[1], "e") == 0)
    {
        encrypt(block, round_keys);
        int encryption_success = 1;
        for (int i = 0; i < BLOCK_SZ; i++)
        {
            if (*block != expected_cipher_output[i])
            {
                printf("[FAILURE] Encrypted text does not match with expected output :( \n");
                printf("Index: %d Expected: %x Output: %x \n", i, expected_cipher_output[i], *block);
                encryption_success = 0;
            };
            block++;
        }
        if (encryption_success)
            printf("[SUCCESS] Encrypted plain text :) \n");
    }
    else if (strcmp(argv[1], "d") == 0)
    {
        decrypt(block, round_keys);
        int decryption_success = 1;
        for (int i = 0; i < BLOCK_SZ; i++)
        {
            if (*block != plain_text[i])
            {
                printf("[FAILURE] Decrypted text does not match original plain text :( \n");
                printf("Index: %d Expected: %x Output: %x \n", i, plain_text[i], *block);
                decryption_success = 0;
            };
            block++;
        }
        if (decryption_success)
            printf("[SUCCESS] Decrypted cypher output :) \n");
    }

    return 0;
}