# KV Store

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg)](https://cmake.org/)
[![Tests](https://img.shields.io/badge/tests-17%20passed-brightgreen.svg)](#tests)
[![Recovery](https://img.shields.io/badge/recovery-4.4M%20records%2Fs-blue.svg)](#benchmarks)

A persistent key-value store written from scratch in C++17, using an append-only log for crash-safe storage.

Persistent key-value хранилище на C++17, реализованное с нуля через append-only лог с поддержкой восстановления после сбоев.

---

## 🇬🇧 English

### About
This project implements a **persistent key-value store** using an **append-only log** — the same fundamental idea behind LevelDB, RocksDB, and PostgreSQL's WAL. All writes go to the end of a single file; reads are served from an in-memory index.

Key ideas demonstrated:
* **Append-only log** — no in-place updates, no random writes.
* **Crash recovery** — if the process dies mid-write, the log is replayed from the start and the partial trailing record is silently discarded.
* **In-memory index** — `std::unordered_map` for O(1) `Get`.
* **Compaction** — periodically rewrite the log with only live data.
* **Binary serialization** — fixed-size headers (`op(1) + key_size(4) + value_size(4)`) and raw payload bytes.
* **Pimpl idiom** — hides implementation details from the public header.

### Features
* `Set(key, value)`, `Get(key)` → `std::optional<string>`, `Delete(key)`, `Contains(key)`
* Data survives process restarts
* Crash recovery on partially written logs
* Log compaction via `Compact()`
* 17 unit tests (Google Test) + 5 benchmarks (Google Benchmark)
* Header-based public API

### Project Structure
```
kv-store/
├── include/kv/Database.h    # Public API
├── include/kv/Storage.h     # Low-level file format
├── src/Storage.cpp
├── src/Database.cpp
├── tests/                   # Google Test
├── benchmarks/              # Google Benchmark
└── apps/main.cpp            # Demo CLI
```

### Benchmark Results

Intel i5-12450H (12 cores @ 2.6 GHz), Windows 11, MSVC 2022, `x64-Release`:

| Operation | Time | Throughput |
|---|---:|---:|
| `Get` (in-memory) | **41 ns** | **24 M ops/s** |
| `Set` (memory + disk append) | 599 ns | 1.7 M ops/s |
| `Delete` | 1.3 µs | 0.8 M ops/s |
| Recovery of 10 000 records | 2.3 ms | **4.4 M records/s** |

**Get is 14× faster than Set** — thanks to the in-memory index.

### Getting Started

#### Prerequisites
* Visual Studio 2022 (Desktop development with C++)
* CMake 3.20+
* vcpkg

#### Build
1. Install dependencies:
   ```bash
   vcpkg install gtest:x64-windows
   vcpkg install benchmark:x64-windows
   ```
2. Clone:
   ```bash
   git clone https://github.com/Bloodmoon001/kv-store.git
   cd kv-store
   ```
3. Open the folder in Visual Studio (`File` → `Open` → `Folder`).
4. Select `x64-Release`, press `F5` to run the demo.

### Usage Example
```cpp
#include "kv/Database.h"
#include <iostream>

int main() {
    kv::Database db("mydb.kv");

    db.Set("name", "Alice");
    db.Set("city", "Moscow");
    db.Delete("city");

    if (auto v = db.Get("name")) {
        std::cout << *v << "\n";   // Alice
    }

    db.Sync();   // flush to disk
    return 0;
}
```

### Tests
```bash
./bin/kv_tests.exe
```
Expected: `[  PASSED  ] 17 tests.`

### License
MIT License. See `LICENSE`.

---

## 🇷🇺 Русский

### О проекте
Persistent key-value хранилище, использующее **append-only лог** — ту же фундаментальную идею, что лежит в основе LevelDB, RocksDB и PostgreSQL WAL. Все записи добавляются в конец одного файла; чтение обслуживается из индекса в памяти.

Реализованные концепции:
* **Append-only лог** — без перезаписи на месте, без случайных записей.
* **Crash recovery** — если процесс упал во время записи, при старте лог проигрывается с начала, а оборванная последняя запись молча отбрасывается.
* **In-memory индекс** — `std::unordered_map` для `Get` за O(1).
* **Compaction** — периодическая перезапись лога только с актуальными данными.
* **Бинарная сериализация** — заголовки фиксированного размера (`op(1) + key_size(4) + value_size(4)`) и сырые байты данных.
* **Pimpl idiom** — прячет детали реализации от публичного заголовка.

### Возможности
* `Set(key, value)`, `Get(key)` → `std::optional<string>`, `Delete(key)`, `Contains(key)`
* Данные переживают перезапуск процесса
* Crash recovery на частично записанных логах
* Компактизация лога через `Compact()`
* 17 unit-тестов (Google Test) + 5 бенчмарков (Google Benchmark)
* Публичный API на основе заголовков

### Структура проекта
```
kv-store/
├── include/kv/Database.h    # Публичный API
├── include/kv/Storage.h     # Низкоуровневый формат файла
├── src/Storage.cpp
├── src/Database.cpp
├── tests/                   # Google Test
├── benchmarks/              # Google Benchmark
└── apps/main.cpp            # Демо CLI
```

### Результаты бенчмарков

Intel i5-12450H (12 ядер @ 2.6 ГГц), Windows 11, MSVC 2022, `x64-Release`:

| Операция | Время | Пропускная способность |
|---|---:|---:|
| `Get` (в памяти) | **41 нс** | **24 млн оп/с** |
| `Set` (память + запись на диск) | 599 нс | 1.7 млн оп/с |
| `Delete` | 1.3 мкс | 0.8 млн оп/с |
| Восстановление 10 000 записей | 2.3 мс | **4.4 млн записей/с** |

**`Get` в 14 раз быстрее `Set`** — благодаря индексу в памяти.

### Сборка и запуск

#### Требования
* Visual Studio 2022 (с рабочей нагрузкой «Разработка классических приложений на C++»)
* CMake 3.20+
* vcpkg

#### Сборка
1. Установите зависимости:
   ```bash
   vcpkg install gtest:x64-windows
   vcpkg install benchmark:x64-windows
   ```
2. Склонируйте репозиторий:
   ```bash
   git clone https://github.com/Bloodmoon001/kv-store.git
   cd kv-store
   ```
3. Откройте папку в Visual Studio (`Файл` → `Открыть` → `Папка`).
4. Выберите `x64-Release` и нажмите `F5`.

### Пример использования
```cpp
#include "kv/Database.h"
#include <iostream>

int main() {
    kv::Database db("mydb.kv");

    db.Set("name", "Alice");
    db.Set("city", "Moscow");
    db.Delete("city");

    if (auto v = db.Get("name")) {
        std::cout << *v << "\n";   // Alice
    }

    db.Sync();
    return 0;
}
```

### Тесты
```bash
./bin/kv_tests.exe
```
Ожидаемый вывод: `[  PASSED  ] 17 tests.`

### Лицензия
MIT License. См. файл `LICENSE`.

---