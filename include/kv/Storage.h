#pragma once

#include <string>
#include <fstream>
#include <functional>
#include <cstdint>
#include <cstddef>

namespace kv {

    enum class OpType : std::uint8_t {
        Put = 0,
        Delete = 1
    };

    class Storage {
    public:
        explicit Storage(const std::string& path);

        ~Storage();

        Storage(const Storage&) = delete;
        Storage& operator=(const Storage&) = delete;
        Storage(Storage&& other) noexcept;
        Storage& operator=(Storage&& other) noexcept;

        void Append(OpType op, const std::string& key, const std::string& value = "");

        using ReplayCallback =
            std::function<void(OpType, const std::string&, const std::string&)>;

        std::size_t Replay(const ReplayCallback& callback) const;

        void Sync();

        std::uintmax_t fileSize();

        void Reset();

        const std::string& path() const noexcept { return path_; }

    private:
        std::string path_;
        std::fstream file_;
    };

} // namespace kv