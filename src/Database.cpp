#include "kv/Database.h"
#include "kv/Storage.h"
#include <stdexcept>
#include <utility>

namespace kv {


    class Database::Impl {
    public:
        explicit Impl(const std::string& path) : storage(path) {}
        Storage storage;
    };


    Database::Database(const std::string& path)
        : path_(path), impl_(new Impl(path)) {
        RecoverFromLog();
    }

    Database::~Database() {
        if (impl_) {
            try { impl_->storage.Sync(); }
            catch (...) { }
            delete impl_;
        }
    }

    Database::Database(Database&& other) noexcept
        : path_(std::move(other.path_)),
        data_(std::move(other.data_)),
        impl_(other.impl_) {
        other.impl_ = nullptr;
    }

    Database& Database::operator=(Database&& other) noexcept {
        if (this != &other) {
            delete impl_;
            path_ = std::move(other.path_);
            data_ = std::move(other.data_);
            impl_ = other.impl_;
            other.impl_ = nullptr;
        }
        return *this;
    }


    void Database::RecoverFromLog() {
        data_.clear();
        impl_->storage.Replay([this](OpType op, const std::string& key, const std::string& value) {
            if (op == OpType::Put) {
                data_[key] = value;
            }
            else if (op == OpType::Delete) {
                data_.erase(key);
            }
            });
    }


    void Database::Set(const std::string& key, const std::string& value) {
        if (key.empty()) throw std::invalid_argument("Key cannot be empty");

        impl_->storage.Append(OpType::Put, key, value);
        data_[key] = value;
    }

    std::optional<std::string> Database::Get(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) return std::nullopt;
        return it->second;
    }

    bool Database::Delete(const std::string& key) {
        auto it = data_.find(key);
        if (it == data_.end()) return false;

        impl_->storage.Append(OpType::Delete, key);
        data_.erase(it);
        return true;
    }

    bool Database::Contains(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    std::size_t Database::size() const noexcept {
        return data_.size();
    }

    void Database::Sync() {
        impl_->storage.Sync();
    }


    std::size_t Database::Compact() {
        std::size_t oldCount = impl_->storage.Replay(
            [](OpType, const std::string&, const std::string&) {});

        impl_->storage.Reset();

        for (const auto& [key, value] : data_) {
            impl_->storage.Append(OpType::Put, key, value);
        }
        impl_->storage.Sync();

        std::size_t newCount = data_.size();
        return oldCount > newCount ? oldCount - newCount : 0;
    }

} // namespace kv