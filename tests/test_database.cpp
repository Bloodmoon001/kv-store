#include <gtest/gtest.h>
#include "kv/Database.h"
#include <cstdio>
#include <string>

using kv::Database;

namespace {
    std::string tmpPath(const std::string& name) {
        return "test_db_" + name + ".kv";
    }
    void removeFile(const std::string& path) {
        std::remove(path.c_str());
    }
}

TEST(Database, StartsEmpty) {
    const std::string path = tmpPath("start");
    removeFile(path);
    {
        Database db(path);
        EXPECT_EQ(db.size(), 0u);
        EXPECT_FALSE(db.Get("missing").has_value());
    }
    removeFile(path);
}

TEST(Database, SetAndGet) {
    const std::string path = tmpPath("setget");
    removeFile(path);
    {
        Database db(path);
        db.Set("name", "Alice");
        db.Set("age", "30");

        ASSERT_TRUE(db.Get("name").has_value());
        EXPECT_EQ(*db.Get("name"), "Alice");
        EXPECT_EQ(*db.Get("age"), "30");
        EXPECT_EQ(db.size(), 2u);
    }
    removeFile(path);
}

TEST(Database, Overwrite) {
    const std::string path = tmpPath("overwrite");
    removeFile(path);
    {
        Database db(path);
        db.Set("key", "old");
        db.Set("key", "new");
        EXPECT_EQ(*db.Get("key"), "new");
        EXPECT_EQ(db.size(), 1u);
    }
    removeFile(path);
}

TEST(Database, Delete) {
    const std::string path = tmpPath("delete");
    removeFile(path);
    {
        Database db(path);
        db.Set("k", "v");
        EXPECT_TRUE(db.Delete("k"));
        EXPECT_FALSE(db.Contains("k"));
        EXPECT_EQ(db.size(), 0u);
        EXPECT_FALSE(db.Delete("k"));  
    }
    removeFile(path);
}

TEST(Database, PersistsAcrossReopen) {
    const std::string path = tmpPath("persist");
    removeFile(path);
    {
        Database db(path);
        db.Set("name", "Alice");
        db.Set("city", "Moscow");
        db.Delete("name");
        db.Set("name", "Bob");
        db.Sync();
    }
    {
        Database db(path);
        EXPECT_EQ(db.size(), 2u);
        EXPECT_EQ(*db.Get("name"), "Bob");
        EXPECT_EQ(*db.Get("city"), "Moscow");
    }
    removeFile(path);
}

TEST(Database, EmptyKeyThrows) {
    const std::string path = tmpPath("emptykey");
    removeFile(path);
    {
        Database db(path);
        EXPECT_THROW(db.Set("", "value"), std::invalid_argument);
    }
    removeFile(path);
}

TEST(Database, EmptyValueAllowed) {
    const std::string path = tmpPath("emptyval");
    removeFile(path);
    {
        Database db(path);
        db.Set("key", "");
        ASSERT_TRUE(db.Get("key").has_value());
        EXPECT_EQ(*db.Get("key"), "");
    }
    removeFile(path);
}

TEST(Database, CompactReducesLogSize) {
    const std::string path = tmpPath("compact");
    removeFile(path);
    {
        Database db(path);
        for (int i = 0; i < 50; ++i) {
            db.Set("key", std::to_string(i));
        }
        db.Sync();

        std::size_t removed = db.Compact();
        EXPECT_GT(removed, 0u);
        EXPECT_EQ(db.size(), 1u);
        EXPECT_EQ(*db.Get("key"), "49");
    }
    {
        Database db(path);
        EXPECT_EQ(db.size(), 1u);
        EXPECT_EQ(*db.Get("key"), "49");
    }
    removeFile(path);
}

TEST(Database, ManyKeysPersist) {
    const std::string path = tmpPath("manykeys");
    removeFile(path);
    const int N = 1000;
    {
        Database db(path);
        for (int i = 0; i < N; ++i) {
            db.Set("key" + std::to_string(i), "value" + std::to_string(i));
        }
        db.Sync();
        EXPECT_EQ(db.size(), static_cast<std::size_t>(N));
    }
    {
        Database db(path);
        EXPECT_EQ(db.size(), static_cast<std::size_t>(N));
        for (int i = 0; i < N; ++i) {
            auto v = db.Get("key" + std::to_string(i));
            ASSERT_TRUE(v.has_value());
            EXPECT_EQ(*v, "value" + std::to_string(i));
        }
    }
    removeFile(path);
}

TEST(Database, DeleteAfterReopen) {
    const std::string path = tmpPath("delreopen");
    removeFile(path);
    {
        Database db(path);
        db.Set("a", "1");
        db.Set("b", "2");
        db.Sync();
    }
    {
        Database db(path);
        EXPECT_TRUE(db.Delete("a"));
        db.Sync();
    }
    {
        Database db(path);
        EXPECT_EQ(db.size(), 1u);
        EXPECT_FALSE(db.Contains("a"));
        EXPECT_TRUE(db.Contains("b"));
    }
    removeFile(path);
}