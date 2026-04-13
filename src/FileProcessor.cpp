#include <cstring>
#include <ctime>
#include "KuzNech.h"

void FileHandler::EncryptFile(const std::string& inpath, const std::string& outpath, RegimeCrypt regime) {
    std::ifstream infile(inpath, std::ios::binary);
    std::ofstream outfile(outpath, std::ios::binary);
    if (!infile.is_open()) { std::cerr << "File is not found!\n"; return; }

    uint8_t block[16];
    uint8_t prev_cipher[16] = {0};

    if (regime == RegimeCrypt::CBC) {
        for(int i=0; i<16; i++) prev_cipher[i] = rand() % 256;
        outfile.write(reinterpret_cast<char*>(prev_cipher), 16);
    }

    while (infile.read(reinterpret_cast<char*>(block), 16)) {
        if (regime == RegimeCrypt::CBC) for(int i=0; i<16; i++) block[i] ^= prev_cipher[i];
        engine.EncryptBlock(block);
        if (regime == RegimeCrypt::CBC) memcpy(prev_cipher, block, 16);
        outfile.write(reinterpret_cast<char*>(block), 16);
    }

    long last_block_size = infile.gcount();
    uint8_t padding_value = 16 - (uint8_t)last_block_size;
    for (int i = last_block_size; i < 16; i++) block[i] = padding_value;

    if (regime == RegimeCrypt::CBC) for(int i=0; i<16; i++) block[i] ^= prev_cipher[i];
    engine.EncryptBlock(block);
    outfile.write(reinterpret_cast<char*>(block), 16);
}

void FileHandler::DecryptFile(const std::string& inpath, const std::string& outpath, RegimeCrypt regime) {
    std::ifstream infile(inpath, std::ios::binary);
    std::ofstream outfile(outpath, std::ios::binary);
    if (!infile.is_open()) { std::cerr << "File is not found!\n"; return; }

    uint8_t prev_cipher[16] = {0}, current_block[16], next_block[16];
    if (regime == RegimeCrypt::CBC) infile.read(reinterpret_cast<char*>(prev_cipher), 16);

    if (!infile.read(reinterpret_cast<char*>(current_block), 16)) return;

    while (infile.read(reinterpret_cast<char*>(next_block), 16)) {
        uint8_t temp_cipher[16];
        if (regime == RegimeCrypt::CBC) memcpy(temp_cipher, current_block, 16);
        
        engine.DecryptBlock(current_block);
        if (regime == RegimeCrypt::CBC) {
            for(int i=0; i<16; i++) current_block[i] ^= prev_cipher[i];
            memcpy(prev_cipher, temp_cipher, 16);
        }
        outfile.write(reinterpret_cast<char*>(current_block), 16);
        memcpy(current_block, next_block, 16);
    }

    engine.DecryptBlock(current_block);
    if (regime == RegimeCrypt::CBC) for(int i=0; i<16; i++) current_block[i] ^= prev_cipher[i];
    uint8_t padding_value = current_block[15];
    if (padding_value <= 16) outfile.write(reinterpret_cast<char*>(current_block), 16 - padding_value);
}
