#ifndef KUZNECH_H
#define KUZNECH_H

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>

// Добавляем пропущенный тип режима
enum class RegimeCrypt { ECB, CBC };

class Kuznechik {
private:
    uint8_t iter_keys[10][16];
    uint8_t InvSbox[256];
    
    uint8_t gf_mul(uint8_t a, uint8_t b);
    void R(uint8_t* block);
    void L(uint8_t* block);
    void InvR(uint8_t* block);
    void InvL(uint8_t* block);
    void S(uint8_t* block);
    void InvS(uint8_t* block);
    void X(uint8_t* block, const uint8_t* key);
    void F(uint8_t* A, uint8_t* B, const uint8_t* C);

public:
    Kuznechik(const uint8_t* master_key);
    void EncryptBlock(uint8_t* block);
    void DecryptBlock(uint8_t* block);
};

// Переименовал в FileHandler (как в твоем main)
class FileHandler {
private:
    Kuznechik& engine;
public:
    FileHandler(Kuznechik& cipher) : engine(cipher) {}
    void EncryptFile(const std::string& inpath, const std::string& outpath, RegimeCrypt regime);
    void DecryptFile(const std::string& inpath, const std::string& outpath, RegimeCrypt regime);
};

#endif
