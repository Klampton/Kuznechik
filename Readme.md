# Kuznechik (GOST 34.12-2015) Block Cipher Implementation

![C++](https://img.shields.io/badge/C++-17-blue.svg) ![Build](https://img.shields.io/badge/build-CMake-green.svg) ![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)

A high-performance C++ implementation of the **GOST 34.12-2015** symmetric block cipher (widely known as **Kuznechik**). This repository provides a professional CLI utility for file encryption with support for advanced cipher modes and secure key derivation.

---

### 🛡️ Technical Specifications

| Feature | Specification |
| :--- | :--- |
| **Block Size** | 128 bits (16 bytes) |
| **Key Length** | 256 bits (32 bytes) |
| **Structure** | Substitution-Permutation Network (SPN) |
| **Rounds** | 9 full rounds + 1 final key XOR |
| **Field** | GF(2^8) with polynomial x^8 + x^7 + x^6 + x + 1 (0xC3) |
| **Modes** | CBC (Cipher Block Chaining), ECB (Electronic Codebook) |

---

### ⚙️ Core Architecture

The project is designed with modularity and separation of concerns in mind:

*   **`Kuznechik` Engine**: Implements the mathematical primitives (X, S, L transformations), the non-linear S-box, and the Key Schedule using a Feistel network.
*   **`FileHandler`**: Manages binary streams, implements **PKCS#7 Padding**, and handles block chaining for CBC mode.
*   **Key Derivation**: Integrates **SHA-256** to map arbitrary user passwords to a fixed 256-bit entropy space required by the cipher.

> [!NOTE]
> The implementation has been verified against the official test vectors provided in **Annex A** of the GOST 34.12-2015 standard.

---

### 🚀 Installation & Building

The project requires a compiler with **C++17** support and **CMake** (v3.10+).

```bash
# Clone the repository
git clone https://github.com/Klampton/Kuznechik.git
cd Kuznechik

# Build using CMake
mkdir Build && cd Build
cmake ..
cmake --build . --config Release

💻 Command Line Interface

The utility supports action flags, password input, and mode selection.
code Bash

./kuznechik [e/d] [input_path] -p [password] [options]

Arguments:

    e | d : Encrypt or Decrypt the target file.

    -p <str> : Define the secret password for SHA-256 key derivation.

    -m <mode> : Cipher mode: cbc (default) or ecb.

    [!WARNING]
    ECB mode does not provide serious security for data with patterns (like images). It is included primarily for standard compliance and verification. Use CBC for production data.

Example Usage:
code Bash

# Encrypt a file using CBC mode
./kuznechik e test.txt -p "MySecurePass123" -m cbc

# Decrypt the resulting file
./kuznechik d test_e_CBC.txt -p "MySecurePass123" -m cbc

🧪 Automated Verification

The repository includes a Python-based integration test suite to verify data integrity across different file types (binary, text, media).
code Bash

python tests/run_tests.py

    [!IMPORTANT]
    The test script verifies the result by calculating SHA-256 checksums of the original and the decrypted data. A PASS status ensures bit-perfect restoration.

📂 Directory Structure
code Text

.
├── include/           # Header files (.h)
├── src/               # Implementation files (.cpp, .c)
├── tests/             # Python testing framework
├── CMakeLists.txt     # Build configuration
└── README.md          # Technical documentation

⚖️ License

This project is licensed under the MIT License. See the LICENSE file for more information.