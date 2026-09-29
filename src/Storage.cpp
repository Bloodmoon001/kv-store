#include "kv/Storage.h"
#include <stdexcept>
#include <limits>

namespace kv {

    namespace {
        // op(1) + key_size(4) + value_size(4)
        constexpr std::size_t HEADER_SIZE = 1 + 4 + 4;
        constexpr std::uint32_t MAX_FIELD_SIZE = std::numeric_limits<std::uint32_t>::max();
    }


    Storage::Storage(const std::string& path) : path_(path) {
        file_.open(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);

        if (!file_.is_open()) {
            file_.clear();
            std::ofstream create(path, std::ios::binary);
            if (!create) throw std::runtime_error("Cannot create file: " + path);
            create.close();

            file_.open(path, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
            if (!file_.is_open()) {
                throw std::runtime_error("Cannot open file: " + path);
            }
        }
    }

    Storage::~Storage() {
        if (file_.is_open()) {
            file_.flush();
            file_.close();
        }
    }

    Storage::Storage(Storage&& other) noexcept
        : path_(std::move(other.path_)), file_(std::move(other.file_)) {
    }

    Storage& Storage::operator=(Storage&& other) noexcept {
        if (this != &other) {
            if (file_.is_open()) { file_.flush(); file_.close(); }
            path_ = std::move(other.path_);
            file_ = std::move(other.file_);
        }
        return *this;
    }


    void Storage::Append(OpType op, const std::string& key, const std::string& value) {
        if (key.size() > MAX_FIELD_SIZE)
            throw std::runtime_error("Key too large: " + std::to_string(key.size()));
        if (value.size() > MAX_FIELD_SIZE)
            throw std::runtime_error("Value too large: " + std::to_string(value.size()));

        const std::uint8_t  opByte = static_cast<std::uint8_t>(op);
        const std::uint32_t keySize = static_cast<std::uint32_t>(key.size());
        const std::uint32_t valueSize = static_cast<std::uint32_t>(value.size());

        file_.write(reinterpret_cast<const char*>(&opByte), sizeof(opByte));
        file_.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
        file_.write(reinterpret_cast<const char*>(&valueSize), sizeof(valueSize));

        if (!key.empty())   file_.write(key.data(), key.size());
        if (!value.empty()) file_.write(value.data(), value.size());

        if (!file_) {
            throw std::runtime_error("Failed to write to file: " + path_);
        }
    }


    std::size_t Storage::Replay(const ReplayCallback& callback) const {
        std::ifstream in(path_, std::ios::binary);
        if (!in.is_open()) {
            throw std::runtime_error("Cannot open file for reading: " + path_);
        }

        std::size_t count = 0;

        while (true) {
            std::uint8_t  opByte{};
            std::uint32_t keySize{};
            std::uint32_t valueSize{};

            in.read(reinterpret_cast<char*>(&opByte), sizeof(opByte));
            if (in.gcount() == 0) break;   
            if (!in) break;                

            in.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
            if (!in) break;

            in.read(reinterpret_cast<char*>(&valueSize), sizeof(valueSize));
            if (!in) break;

            std::string key(keySize, '\0');
            if (keySize > 0) {
                in.read(key.data(), keySize);
                if (!in) break;
            }

            std::string value(valueSize, '\0');
            if (valueSize > 0) {
                in.read(value.data(), valueSize);
                if (!in) break;
            }

            callback(static_cast<OpType>(opByte), key, value);
            ++count;
        }

        return count;
    }


    void Storage::Sync() {
        file_.flush();
        if (!file_) {
            throw std::runtime_error("Failed to flush file: " + path_);
        }
    }

    std::uintmax_t Storage::fileSize() {
        file_.flush();

        std::ifstream in(path_, std::ios::binary | std::ios::ate);
        if (!in.is_open()) return 0;
        return static_cast<std::uintmax_t>(in.tellg());
    }

    void Storage::Reset() {
        file_.flush();
        file_.close();

        file_.open(path_, std::ios::out | std::ios::binary | std::ios::trunc);
        file_.close();

        file_.open(path_, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to reset file: " + path_);
        }
    }

} // namespace kv