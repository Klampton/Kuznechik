import subprocess
import hashlib
import os
import sys

# Находим путь к папке, где лежит сам скрипт (Kuznechik/tests)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
# Определяем корень проекта (Kuznechik)
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

EXE_NAME = "kuznechik"
PASSWORD = "Aboba228"

# Список файлов для теста: Имя файла -> Содержимое (текст или байты)
TEST_FILES = {
    "test_doc.txt": "Simple text for GOST testing.",
    "test_img.png": b"\x89PNG\r\n\x1a\n" + os.urandom(512),
    "test_snd.mp3": b"\xff\xfb\x90\x44" + os.urandom(512),
}

def get_hash(file_path):
    hasher = hashlib.sha256()
    with open(file_path, "rb") as f:
        while chunk := f.read(8192):
            hasher.update(chunk)
    return hasher.hexdigest()

def find_exe():
    ext = ".exe" if os.name == "nt" else ""
    # Проверяем все возможные места, где может лежать EXE после сборки
    search_paths = [
        os.path.join(PROJECT_ROOT, "Build", EXE_NAME + ext),
        os.path.join(PROJECT_ROOT, "Build", "Debug", EXE_NAME + ext),
        os.path.join(PROJECT_ROOT, "Build", "Release", EXE_NAME + ext),
        os.path.join(PROJECT_ROOT, EXE_NAME + ext)
    ]
    for p in search_paths:
        if os.path.exists(p):
            return p
    return None

def run_test():
    exe_path = find_exe()
    if not exe_path:
        print(f"Error: {EXE_NAME} not found!")
        print("Expected paths checked:")
        print(f" - {os.path.join(PROJECT_ROOT, 'Build')}")
        return

    print(f"--- Found executable: {exe_path} ---")
    
    # Чтобы не засорять корень, будем проводить тесты в папке tests
    os.chdir(SCRIPT_DIR)

    for filename, content in TEST_FILES.items():
        print(f"\n[Testing: {filename}]")
        
        # 1. Создаем файл
        write_mode = "wb" if isinstance(content, bytes) else "w"
        with open(filename, write_mode) as f:
            f.write(content)
        
        orig_hash = get_hash(filename)
        
        for m in ["cbc", "ecb"]:
            # Имена по твоей логике: Имя_Действие_Режим.расширение
            ext = os.path.splitext(filename)[1]
            name = os.path.splitext(filename)[0]
            enc_file = f"{name}_e_{m.upper()}{ext}"
            dec_file = f"{name}_e_{m.upper()}_d_{m.upper()}{ext}"

            try:
                # 2. Шифрование
                subprocess.run([exe_path, "e", filename, "-p", PASSWORD, "-m", m], check=True)
                # 3. Дешифрование
                subprocess.run([exe_path, "d", enc_file, "-p", PASSWORD, "-m", m], check=True)

                # 4. Проверка
                if not os.path.exists(dec_file):
                    print(f"  > Mode {m.upper()}: FAIL (Output file not created)")
                    continue

                res_hash = get_hash(dec_file)
                if orig_hash == res_hash:
                    print(f"  > Mode {m.upper()}: PASS")
                else:
                    print(f"  > Mode {m.upper()}: FAIL (Hash mismatch!)")

                # Очистка
                if os.path.exists(enc_file): os.remove(enc_file)
                if os.path.exists(dec_file): os.remove(dec_file)
            except Exception as e:
                print(f"  > Mode {m.upper()}: ERROR ({e})")
        
        if os.path.exists(filename): os.remove(filename)

    print("\n" + "="*30)
    print("ALL TESTS COMPLETED")
    print("="*30)

if __name__ == "__main__":
    run_test()