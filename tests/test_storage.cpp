#include <gtest/gtest.h>
#include "kv/Storage.h"
#include <cstdio>
#include <string>

using kv::Storage;
using kv::OpType;

namespace {
    std::string tmpPath(const std::string& name) {
        return "test_" + name + ".kv";
    }
    void removeFile(const std::string& path) {
        std::remove(path.c_str());
    }
}

TEST(Storage, CreatesEmptyFile) {
    const std::string path = tmpPath("create");
    removeFile(path);
    {
        Storage s(path);
        EXPECT_EQ(s.fileSize(), 0u);
    }
    removeFile(path);
}

TEST(Storage, AppendIncreasesFileSize) {
    const std::string path = tmpPath("append");
    removeFile(path);
    {
        Storage s(path);
        EXPECT_EQ(s.fileSize(), 0u);

        s.Append(OpType::Put, "a", "1");
        EXPECT_EQ(s.fileSize(), 11u);

        s.Append(OpType::Put, "b", "22");
        EXPECT_EQ(s.fileSize(), 23u);
    }
    removeFile(path);
}

TEST(Storage, ReplayReadsAllRecords) {
    const std::string path = tmpPath("replay");
    removeFile(path);
    {
        Storage s(path);
        s.Append(OpType::Put, "name", "Alice");
        s.Append(OpType::Put, "age", "30");
        s.Append(OpType::Delete, "age");
    }
    {
        Storage s(path);
        int puts = 0, dels = 0;
        std::size_t count = s.Replay([&](OpType op, const std::string& k, const std::string& v) {
            if (op == OpType::Put) { ++puts; EXPECT_EQ(v, k == "name" ? "Alice" : "30"); }
            else                       ++dels;
            });
        EXPECT_EQ(count, 3u);
        EXPECT_EQ(puts, 2);
        EXPECT_EQ(dels, 1);
    }
    removeFile(path);
}

TEST(Storage, ReplayEmptyFile) {
    const std::string path = tmpPath("empty");
    removeFile(path);
    {
        Storage s(path);
        std::size_t count = s.Replay([](OpType, const std::string&, const std::string&) {});
        EXPECT_EQ(count, 0u);
    }
    removeFile(path);
}

TEST(Storage, EmptyKeyAndValue) {
    const std::string path = tmpPath("empty_kv");
    removeFile(path);
    {
        Storage s(path);
        s.Append(OpType::Put, "", "");
        s.Append(OpType::Put, "k", "");
    }
    {
        Storage s(path);
        std::size_t count = s.Replay([](OpType, const std::string&, const std::string&) {});
        EXPECT_EQ(count, 2u);
    }
    removeFile(path);
}

TEST(Storage, ResetClearsFile) {
    const std::string path = tmpPath("reset");
    removeFile(path);
    {
        Storage s(path);
        s.Append(OpType::Put, "k", "v");
        EXPECT_GT(s.fileSize(), 0u);

        s.Reset();
        EXPECT_EQ(s.fileSize(), 0u);
    }
    removeFile(path);
}

TEST(Storage, PersistsAcrossReopen) {
    const std::string path = tmpPath("persist");
    removeFile(path);
    {
        Storage s(path);
        s.Append(OpType::Put, "persistent", "value");
        s.Sync();
    }
    {
        Storage s(path);
        std::size_t count = s.Replay([](OpType, const std::string&, const std::string&) {});
        EXPECT_EQ(count, 1u);
    }
    removeFile(path);
}