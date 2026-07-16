#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stddef.h>

uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod);

uint64_t dh_generate_shared(uint64_t pvtkey, uint64_t other_public, uint64_t p);

void xor_encrypt(const unsigned char *plain, unsigned char *cipher, size_t len, uint64_t key);
void xor_decrypt(const unsigned char *cipher, unsigned char *plain, size_t len, uint64_t key);

#endif
