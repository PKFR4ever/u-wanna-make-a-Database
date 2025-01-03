#include <gtest/gtest.h>
#include <mutex>
#include <condition_variable>


// 测试加法函数
int add(int a, int b) {
    return a + b;
}

// 测试用例
TEST(AddTest, PositiveNumbers) {
    EXPECT_EQ(add(1, 1), 2);
    EXPECT_EQ(add(2, 3), 5);
}

TEST(AddTest, NegativeNumbers) {
    EXPECT_EQ(add(-1, -1), -2);
    EXPECT_EQ(add(-2, 3), 1);
}

// main 函数，启动 Google Test 运行框架
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
