#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <cstddef>

namespace kv {

    class Database {
    public:
        explicit Database(const std::string& path);
        ~Database();

        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
        Database(Database&&) noexcept;
        Database& operator=(Database&&) noexcept;

        void Set(const std::string& key, const std::string& value);
        std::optional<std::string> Get(const std::string& key) const;
        bool Delete(const std::string& key);
        bool Contains(const std::string& key) const;

        std::size_t size() const noexcept;

        void Sync();
        std::size_t Compact();

        const std::string& path() const noexcept { return path_; }

    private:
        void RecoverFromLog();

        std::string path_;
        std::unordered_map<std::string, std::string> data_;

        class Impl;      
        Impl* impl_ = nullptr;
    };

} // namespace kv