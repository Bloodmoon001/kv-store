#include "kv/Database.h"
#include <iostream>
#include <cstdio>
#include <string>

int main() {
    const std::string path = "demo.kv";
    std::remove(path.c_str());

    std::cout << "=== Session 1: writing data ===\n";
    {
        kv::Database db(path);
        db.Set("name", "Alice");
        db.Set("age", "30");
        db.Set("city", "Moscow");
        db.Set("name", "Bob");        
        db.Delete("age");              
        db.Sync();

        std::cout << "Size: " << db.size() << "\n";
        std::cout << "name = " << *db.Get("name") << "\n";
        std::cout << "age exists? " << (db.Contains("age") ? "yes" : "no") << "\n";
        std::cout << "city = " << *db.Get("city") << "\n";
        std::cout << "missing = "
            << (db.Get("nonexistent").has_value() ? "found" : "nullopt") << "\n";
    }

    std::cout << "\n=== Session 2: reopen, data recovered ===\n";
    {
        kv::Database db(path);
        std::cout << "Size after recovery: " << db.size() << "\n";
        std::cout << "name = " << *db.Get("name") << "\n";
        std::cout << "city = " << *db.Get("city") << "\n";
        std::cout << "age exists? " << (db.Contains("age") ? "yes" : "no") << "\n";
    }

    std::cout << "\n=== Session 3: compaction ===\n";
    {
        kv::Database db(path);
        
        for (int i = 0; i < 100; ++i) {
            db.Set("counter", std::to_string(i));
        }
        db.Sync();

        std::cout << "Size (unique keys): " << db.size() << "\n";
        std::size_t removed = db.Compact();
        std::cout << "Records removed by compaction: " << removed << "\n";
        std::cout << "counter = " << *db.Get("counter") << "\n";
    }

    std::cout << "\n=== Session 4: verify after compaction ===\n";
    {
        kv::Database db(path);
        std::cout << "Size after recovery: " << db.size() << "\n";
        std::cout << "name = " << *db.Get("name") << "\n";
        std::cout << "counter = " << *db.Get("counter") << "\n";
    }

    std::remove(path.c_str());
    return 0;
}