#include <iostream>
#include <string>
#include <filesystem>
#include <cstring>
#include "KuzNech.h"

// Исправленное подключение C-библиотеки SHA-256
extern "C" {
#include "sha256.h"
}

// Определение типа BYTE, если его нет в sha256.h
typedef unsigned char BYTE;

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    // Проверка минимального количества аргументов: kuznechik [e/d] [path] -p [pass]
    if (argc < 4) {
        std::cout << "=== Kuznechik GOST 34.12-2015 CLI Tool ===\n";
        std::cout << "Usage: kuznechik [action: e/d] [input_path] -p [password] [-m cbc/ecb]\n";
        return 1;
    }

    // Чтение базовых настроек
    std::string action = argv[1]; 
    fs::path inpath = argv[2];    
    std::string password = "";
    RegimeCrypt selectedRegime = RegimeCrypt::CBC; // По умолчанию безопасный CBC

    // Парсинг флагов пароля и режима
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-p" && i + 1 < argc) {
            password = argv[i + 1];
            i++;
        }
        if (arg == "-m" && i + 1 < argc) {
            if (std::string(argv[i + 1]) == "ecb") {
                selectedRegime = RegimeCrypt::ECB;
            }
            i++;
        }
    }

    // Проверки безопасности
    if (password.empty()) { 
        std::cerr << "Error: Password required! Use flag -p [password]\n"; 
        return 1; 
    }

    if (!fs::exists(inpath)) {
        std::cerr << "Error: File not found: " << inpath.string() << "\n";
        return 1;
    }

    // 1. Генерация 256-битного мастер-ключа из пароля через SHA-256
    uint8_t master_key[32];
    SHA256_CTX ctx; 
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE*)password.c_str(), password.length());
    sha256_final(&ctx, (BYTE*)master_key);

    // 2. Логика формирования outpath (Путь + Имя_Действие_Режим.расширение)
    std::string modeStr = (selectedRegime == RegimeCrypt::CBC) ? "CBC" : "ECB";
    
    // Формируем новое имя: например, Test_e_CBC.txt
    std::string newFileName = inpath.stem().string() + "_" + action + "_" + modeStr + inpath.extension().string();
    
    // Сохраняем в ту же папку, где лежит входной файл
    fs::path outpath = inpath.parent_path() / newFileName;

    // 3. Инициализация движка Кузнечик и обработчика файлов
    Kuznechik engine(master_key);
    FileHandler handler(engine);

    // 4. Запуск процесса
    try {
        if (action == "e") {
            std::cout << "Action: Encrypting [" << modeStr << "]...\n";
            handler.EncryptFile(inpath.string(), outpath.string(), selectedRegime);
            std::cout << "SUCCESS: File was encrypted.\n";
        } 
        else if (action == "d") {
            std::cout << "Action: Decrypting [" << modeStr << "]...\n";
            handler.DecryptFile(inpath.string(), outpath.string(), selectedRegime);
            std::cout << "SUCCESS: File was decrypted.\n";
        } 
        else {
            std::cerr << "Error: Unknown action '" << action << "'. Use 'e' or 'd'.\n";
            return 1;
        }
        
        std::cout << "Result saved to: " << outpath.string() << "\n";
    } 
    catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
