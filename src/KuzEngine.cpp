#include "KuzNech.h"
#include <cstring>

static const uint8_t Sbox[256] = {
    252, 238, 221, 17, 207, 110, 49, 22, 251, 196, 250, 218, 35, 197, 4, 77, 233,
    119, 240, 219, 147, 46, 153, 186, 23, 54, 241, 187, 20, 205, 95, 193, 249, 24, 101,
    90, 226, 92, 239, 33, 129, 28, 60, 66, 139, 1, 142, 79, 5, 132, 2, 174, 227, 106, 143,
    160, 6, 11, 237, 152, 127, 212, 211, 31, 235, 52, 44, 81, 234, 200, 72, 171, 242, 42,
    104, 162, 253, 58, 206, 204, 181, 112, 14, 86, 8, 12, 118, 18, 191, 114, 19, 71, 156,
    183, 93, 135, 21, 161, 150, 41, 16, 123, 154, 199, 243, 145, 120, 111, 157, 158, 178,
    177, 50, 117, 25, 61, 255, 53, 138, 126, 109, 84, 198, 128, 195, 189, 13, 87, 223,
    245, 36, 169, 62, 168, 67, 201, 215, 121, 214, 246, 124, 34, 185, 3, 224, 15, 236,
    222, 122, 148, 176, 188, 220, 232, 40, 80, 78, 51, 10, 74, 167, 151, 96, 115, 30, 0,
    98, 68, 26, 184, 56, 130, 100, 159, 38, 65, 173, 69, 70, 146, 39, 94, 85, 47, 140, 163,
    165, 125, 105, 213, 149, 59, 7, 88, 179, 64, 134, 172, 29, 247, 48, 55, 107, 228, 136,
    217, 231, 137, 225, 27, 131, 73, 76, 63, 248, 254, 141, 83, 170, 144, 202, 216, 133,
    97, 32, 113, 103, 164, 45, 43, 9, 91, 203, 155, 37, 208, 190, 229, 108, 82, 89, 166,
    116, 210, 230, 244, 180, 192, 209, 102, 175, 194, 57, 75, 99, 182
};

static const uint8_t coeffs[16] = {148, 32, 133, 16, 194, 192, 1, 251, 1, 192, 194, 16, 133, 32, 148, 1};

Kuznechik::Kuznechik(const uint8_t* master_key) {
    for (int i = 0; i < 256; i++) InvSbox[Sbox[i]] = i;

    uint8_t C_constants[32][16];
    for (int j = 1; j <= 32; j++) {
        uint8_t C[16] = {0};
        C[15] = j;
        L(C);
        memcpy(C_constants[j-1], C, 16);
    }

    memcpy(iter_keys[0], master_key, 16);
    memcpy(iter_keys[1], master_key + 16, 16);

    for (int i = 0; i < 4; i++) {
        uint8_t A[16], B[16];
        memcpy(A, iter_keys[2*i], 16);
        memcpy(B, iter_keys[2*i+1], 16);
        for (int j = 0; j < 8; j++) F(A, B, C_constants[8*i+j]);
        memcpy(iter_keys[2*i+2], A, 16);
        memcpy(iter_keys[2*i+3], B, 16);
    }
}

uint8_t Kuznechik::gf_mul(uint8_t a, uint8_t b) {
    uint8_t res = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) res ^= a;
        bool hi_bit = (a & 0x80);
        a <<= 1;
        if (hi_bit) a ^= 0xC3;
        b >>= 1;
    }
    return res;
}

void Kuznechik::X(uint8_t* block, const uint8_t* key) {
    for (int i = 0; i < 16; i++) block[i] ^= key[i];
}

void Kuznechik::S(uint8_t* block) {
    for (int i = 0; i < 16; i++) block[i] = Sbox[block[i]];
}

void Kuznechik::R(uint8_t* block) {
    uint8_t res = 0;
    for (int i = 0; i < 16; i++) res ^= gf_mul(block[i], coeffs[i]);
    for (int i = 15; i > 0; i--) block[i] = block[i-1];
    block[0] = res;
}

void Kuznechik::L(uint8_t* block) {
    for (int i = 0; i < 16; i++) R(block);
}

void Kuznechik::F(uint8_t* A, uint8_t* B, const uint8_t* C) {
    uint8_t temp[16];
    memcpy(temp, A, 16);
    X(temp, C); S(temp); L(temp);
    for (int i = 0; i < 16; i++) temp[i] ^= B[i];
    memcpy(B, A, 16); memcpy(A, temp, 16);
}

void Kuznechik::InvR(uint8_t* block) {
    uint8_t result = block[0];
    for (int i = 0; i < 15; i++) block[i] = block[i+1];
    uint8_t sum = 0;
    for (int i = 0; i < 15; i++) sum ^= gf_mul(block[i], coeffs[i]);
    block[15] = result ^ sum;
}

void Kuznechik::InvL(uint8_t* block) {
    for (int i = 0; i < 16; i++) InvR(block);
}

void Kuznechik::InvS(uint8_t* block) {
    for (int i = 0; i < 16; i++) block[i] = InvSbox[block[i]];
}

void Kuznechik::EncryptBlock(uint8_t* block) {
    X(block, iter_keys[0]);
    for (int i = 1; i <= 9; i++) {
        S(block); L(block); X(block, iter_keys[i]);
    }
}

void Kuznechik::DecryptBlock(uint8_t* block) {
    X(block, iter_keys[9]);
    for (int i = 8; i >= 0; i--) {
        InvL(block); InvS(block); X(block, iter_keys[i]);
    }
}