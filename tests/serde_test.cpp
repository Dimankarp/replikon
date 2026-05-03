#include <gtest/gtest.h>
#include "test_utils.h"
#include "serial/serde.h"
#include <tuple>
#include <variant>

struct DummyStruct {
    uint32_t a;
    std::string b;

    bool operator==(const DummyStruct& other) const {
        return a == other.a && b == other.b;
    }
};

SERIALIZABLE(DummyStruct, 2)

TEST(SerdeTest, SerializeDeserializeNumeric) {
    replikon::serde::Buffer buf;

    int32_t val1 = -12345;
    uint64_t val2 = 987654321;
    double val3 = 3.14159;
    std::byte val4{42};
    bool val5 = true;
    
    replikon::serde::serialize(buf, val1);
    replikon::serde::serialize(buf, val2);
    replikon::serde::serialize(buf, val3);
    replikon::serde::serialize(buf, val4);
    replikon::serde::serialize(buf, val5);

    auto view = replikon::serde::BufferView(buf);
    
    auto res1 = replikon::serde::deserialize<int32_t>(view);
    auto res2 = replikon::serde::deserialize<uint64_t>(view);
    auto res3 = replikon::serde::deserialize<double>(view);
    auto res4 = replikon::serde::deserialize<std::byte>(view);
    auto res5 = replikon::serde::deserialize<bool>(view);
    
    ASSERT_TRUE(res1.has_value());
    ASSERT_TRUE(res2.has_value());
    ASSERT_TRUE(res3.has_value());
    ASSERT_TRUE(res4.has_value());
    ASSERT_TRUE(res5.has_value());
    ASSERT_EQ(val1, res1.value());
    ASSERT_EQ(val2, res2.value());
    ASSERT_DOUBLE_EQ(val3, res3.value());
    ASSERT_EQ(val4, res4.value());
    ASSERT_EQ(val5, res5.value());
}

TEST(SerdeTest, SerializeDeserializeString) {
    replikon::serde::Buffer buf;

    std::string empty_str = "";
    std::string normal_str = "hello replikon";
    std::string binary_str = std::string("foo\0bar", 7);
    
    replikon::serde::serialize(buf, empty_str);
    replikon::serde::serialize(buf, normal_str);
    replikon::serde::serialize(buf, binary_str);

    auto view = replikon::serde::BufferView(buf);
    
    auto res1 = replikon::serde::deserialize<std::string>(view);
    auto res2 = replikon::serde::deserialize<std::string>(view);
    auto res3 = replikon::serde::deserialize<std::string>(view);
    
    ASSERT_EQ(empty_str, res1.value());
    ASSERT_EQ(normal_str, res2.value());
    ASSERT_EQ(binary_str, res3.value());
}

TEST(SerdeTest, SerializeDeserializeComplexTypes) {
    replikon::serde::Buffer buf;

    std::vector<int> empty_vec;
    std::vector<std::string> vec = {"a", "b", "c"};
    std::array<uint16_t, 3> arr = {1, 2, 3};
    std::optional<int> opt_null = std::nullopt;
    std::optional<int> opt_val = 123;
    std::tuple<int, std::string, double> tup = {42, "hello", 1.23};
    std::pair<std::string, int> pair = {"key", 999};
    std::monostate mono;
    DummyStruct dummy{999, "dummy"};

    replikon::serde::serialize(buf, empty_vec);
    replikon::serde::serialize(buf, vec);
    replikon::serde::serialize(buf, arr);
    replikon::serde::serialize(buf, opt_null);
    replikon::serde::serialize(buf, opt_val);
    replikon::serde::serialize(buf, tup);
    replikon::serde::serialize(buf, pair);
    replikon::serde::serialize(buf, mono);
    replikon::serde::serialize(buf, dummy);

    auto view = replikon::serde::BufferView(buf);

    ASSERT_EQ(empty_vec, replikon::serde::deserialize<std::vector<int>>(view).value());
    ASSERT_EQ(vec, replikon::serde::deserialize<std::vector<std::string>>(view).value());
    ASSERT_EQ(arr, (replikon::serde::deserialize<std::array<uint16_t, 3>>(view).value()));
    ASSERT_EQ(opt_null, replikon::serde::deserialize<std::optional<int>>(view).value());
    ASSERT_EQ(opt_val, replikon::serde::deserialize<std::optional<int>>(view).value());
    ASSERT_EQ(tup, (replikon::serde::deserialize<std::tuple<int, std::string, double>>(view).value()));
    ASSERT_EQ(pair, (replikon::serde::deserialize<std::pair<std::string, int>>(view).value()));
    ASSERT_TRUE(replikon::serde::deserialize<std::monostate>(view).has_value());
    ASSERT_EQ(dummy, replikon::serde::deserialize<DummyStruct>(view).value());
}

TEST(SerdeTest, ErrorHandling) {
    replikon::serde::Buffer empty_buf;
    auto empty_view = replikon::serde::BufferView(empty_buf);
    
    ASSERT_FALSE(replikon::serde::deserialize<int32_t>(empty_view).has_value());
    ASSERT_FALSE(replikon::serde::deserialize<std::string>(empty_view).has_value());

    replikon::serde::Buffer partial_str_buf;
    replikon::serde::serialize(partial_str_buf, static_cast<size_t>(100)); // String length 100 
    auto partial_view = replikon::serde::BufferView(partial_str_buf);
    
    ASSERT_FALSE(replikon::serde::deserialize<std::string>(partial_view).has_value());
}
