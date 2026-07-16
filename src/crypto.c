#include "crypto.h"
#include <stdint.h>
#include <string.h>

uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod){
    uint64_t result = 1;
    base = base%mod;
    while (exp>0){
        if (exp & 1){
            result = (result*base)%mod;
        }
        base = (base*base)% mod;
        exp >>= 1;
    }
    return result;
}

uint64_t dh_generate_shared(uint64_t pvtkey,uint64_t other_public,uint64_t p){
    return mod_pow(other_public,pvtkey,p);
}

static uint64_t lcg_state;

static uint64_t lcg_next(void){
    lcg_state = (lcg_state*1103515245ULL + 12345ULL) & 0xFFFFFFFFULL;
    return lcg_state;
}

void xor_encrypt(const unsigned char *plain,unsigned char *cipher,size_t len,uint64_t key){
    lcg_state = key;
    for (size_t i=0; i<len;i++){
        uint8_t keystream_byte = lcg_next() & 0xFF;
        cipher[i] = plain[i] ^ keystream_byte;
    }
}

void xor_decrypt(const unsigned char *cipher,unsigned char *plain,size_t len,uint64_t key){
    xor_encrypt(cipher,plain,len,key);
}
