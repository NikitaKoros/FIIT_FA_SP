// b_tree_disk_tests.cpp

#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <optional>
#include "b_tree_disk.hpp"
#include <client_logger_builder.h>


// Простая обёртка для int
struct SerializableInt {
    int value;
    SerializableInt() = default;
    SerializableInt(int v): value(v) {}

    void serialize(std::fstream& s) const {
        s.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
    static SerializableInt deserialize(std::fstream& s) {
        SerializableInt x;
        s.read(reinterpret_cast<char*>(&x.value), sizeof(x.value));
        return x;
    }
    size_t serialize_size() const {
        return sizeof(value);
    }
    bool operator==(SerializableInt const& o) const { return value == o.value; }
};

// Простая обёртка для std::string
struct SerializableString {
    std::string value;
    SerializableString() = default;
    SerializableString(std::string v): value(std::move(v)) {}

    void serialize(std::fstream& s) const {
        size_t len = value.size();
        s.write(reinterpret_cast<const char*>(&len), sizeof(len));
        s.write(value.data(), len);
    }
    static SerializableString deserialize(std::fstream& s) {
        SerializableString x;
        size_t len;
        s.read(reinterpret_cast<char*>(&len), sizeof(len));
        x.value.resize(len);
        s.read(x.value.data(), len);
        return x;
    }
    size_t serialize_size() const {
        return sizeof(size_t) + value.size();
    }
    bool operator==(SerializableString const& o) const { return value == o.value; }
};

struct IntCmp {
    bool operator()(const SerializableInt& a, const SerializableInt& b) const {
        return a.value < b.value;
    }
};

// Вспомогательная функция для сравнения результатов обхода
template<typename Tree>
std::vector<std::pair<int, std::string>> collect_all(Tree &tree) {
    std::vector<std::pair<int, std::string>> res;
    auto end_it = tree.end();
    for (auto it = tree.begin(); it != end_it; ++it) {
        auto [k, v] = *it;
        res.emplace_back(k.value, v.value);
    }
    return res;
}

// Считываем все элементы дерева в вектор
bool compare_results(
    const std::vector<std::pair<int, std::string>>& expected,
    const std::vector<std::pair<int, std::string>>& actual)
{
    if (expected.size() != actual.size()) return false;
    for (size_t i = 0; i < expected.size(); ++i) {
        if (expected[i] != actual[i]) return false;
    }
    return true;
}

// Удаляем файлы-основания, чтобы каждый тест начинался «с чистого slate»
void cleanup_files(std::string base) {
    std::filesystem::remove(base + ".tree");
    std::filesystem::remove(base + ".data");
}

logger *create_logger(
    std::vector<std::pair<std::string, logger::severity>> const &output_file_streams_setup,
    bool use_console_stream = true,
    logger::severity console_stream_severity = logger::severity::debug)
{
    std::unique_ptr<logger_builder> builder(new client_logger_builder());

    if (use_console_stream)
    {
        builder->add_console_stream(console_stream_severity);
    }

    for (auto &output_file_stream_setup: output_file_streams_setup)
    {
        builder->add_file_stream(output_file_stream_setup.first, output_file_stream_setup.second);
    }

    logger *built_logger = builder->build();

    return built_logger;
}

template <typename tkey, typename tvalue>
struct test_data
{
    tkey key;
    tvalue value;
    size_t depth, index;

    test_data(int depth_, int index_, tkey key_, tvalue value_)
        : depth(depth_), index(index_), key(std::move(key_)), value(std::move(value_)) {}

    test_data(size_t d, size_t i, tkey k, tvalue v) : depth(d), index(i), key(k), value(v) {}
};

template<typename Tree>
bool infix_const_iterator_test(
    Tree &tree,
    const std::vector<test_data<typename Tree::key_type, typename Tree::value_type>> &expected_result)
{
    auto it = tree.begin();
    auto end = tree.end();

    for (const auto &item : expected_result)
    {
        if (it == end)
            return false;

        const auto &[k, v] = *it;

        if (k.value != item.key || v.value != item.value || it.depth() != item.depth || it.index() != item.index)
            return false;

        ++it;
    }

    return it == end;
}


TEST(bTreeDiskPositiveTests, EmptyTree) {
    const std::string path = "btree_disk_empty";
    cleanup_files(path);

    B_tree_disk<SerializableInt, SerializableString, IntCmp, 3> tree(path);

    auto elements = collect_all(tree);

    EXPECT_TRUE(elements.empty());
}

TEST(bTreeDiskPositiveTests, InfixIterationAndAt) {
    const std::string path = "btree_disk_iter";
    cleanup_files(path);

    B_tree_disk<SerializableInt, SerializableString, IntCmp, 3> tree(path);

    std::vector<std::pair<int, std::string>> to_insert = {
        {1, "a"}, 
        {2, "b"}, 
        {15, "c"}, 
        {3, "d"}, 
        {4, "e"}, 
        {27, "f"}
    };
    int i = 0;
    for (auto &p : to_insert) {
        printf(">>>%d<<<\n", i);
        i++;
        tree.insert({ SerializableInt(p.first), SerializableString(p.second) });
    }
    
    std::vector<std::pair<int, std::string>> expected = {
        {1, "a"}, 
        {2, "b"}, 
        {3, "d"}, 
        {4, "e"}, 
        {15, "c"}, 
        {27, "f"}
    };

    auto actual = collect_all(tree);

    EXPECT_TRUE(compare_results(expected, actual));

    auto found = tree.at(SerializableInt(15));

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->value, "c");

    auto notfound = tree.at(SerializableInt(100));
    EXPECT_FALSE(notfound.has_value());
}

TEST(bTreeDiskNegativeTests, EraseNonexistent) {
    const std::string path = "btree_disk_erase";
    cleanup_files(path);

    B_tree_disk<SerializableInt, SerializableString, IntCmp, 3> tree(path);
    tree.insert({SerializableInt(1), SerializableString("a")});
    tree.insert({SerializableInt(2), SerializableString("b")});

    bool erased = tree.erase(SerializableInt(45));
    EXPECT_FALSE(erased);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
