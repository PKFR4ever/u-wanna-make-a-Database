#include <gtest/gtest.h>
// #include <bits/stdc++.h>
#include <fstream>
#include <stdio.h>
#include <cstdio> 

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#define USERNAME_SIZE 32 // username字段 4 byte
#define EMAIL_SIZE 255 // email字段 4 byte

// 依次执行 commands 中的命令 并返回所有输出
std::vector<std::string> run_script(const std::vector<std::string>& commands) {
    std::vector<std::string> output;
    std::string temp_filename = "temp_script.txt";

    // Create a temporary script file
    std::ofstream script_file(temp_filename);
    for (const auto& command : commands) {
        script_file << command << "\n";
    }
    script_file.close();

    // Execute the script and capture output
    std::string command = "rundb.exe < " + temp_filename;
    std::ostringstream raw_output;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("Failed to execute script");
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        raw_output << buffer;
    }
    pclose(pipe);

    // Remove the temporary file
    std::remove(temp_filename.c_str());

    // Split output into lines
    std::istringstream output_stream(raw_output.str());
    std::string line;
    while (std::getline(output_stream, line)) {
        output.push_back(line);
    }

    return output;
}

// 单个insert
TEST(DBtestCmd, InsertOneAndSelectAll) {
    // 模拟用户输入
    std::vector<std::string> commands = {
        "insert 1 user1 person1@example.com",
        "select",
        ".exit",
    };
    // 存放程序的真实输出
    std::vector<std::string> result = run_script(commands);
    // 存放正确的程序输出
    std::vector<std::string> expected = {
        "db > Executed",
        "db > ",
        "(1, user1, person1@example.com)",
        "Executed",
        "db > ",
    };
    // 检查输出
    EXPECT_EQ(result.size(), expected.size());
    for(int i=0;i<result.size();i++){
        EXPECT_EQ(result[i], expected[i]);
    }
}

// 单个insert 但username和email长度最大
TEST(DBtestCmd, InsertOneLongAndSelectAll) {
    // 模拟用户输入
    std::string long_username, long_email;
    for(int i=0;i<USERNAME_SIZE-1;i++) long_username += "a";
    for(int i=0;i<EMAIL_SIZE-1;i++) long_email += "b";
    std::string insert_cmd = "insert 1 " + long_username + " " + long_email;

    std::vector<std::string> commands = {
        insert_cmd,
        "select",
        ".exit",
    };

    // 存放程序的真实输出
    std::vector<std::string> result = run_script(commands);

    // 存放正确的程序输出
    std::string the_only_row = "(1, " + long_username + ", " + long_email + ")";
    std::vector<std::string> expected = {
        "db > Executed",
        "db > ",
        the_only_row,
        "Executed",
        "db > ",
    };
    // 检查输出
    EXPECT_EQ(result.size(), expected.size());
    for(int i=0;i<result.size();i++){
        EXPECT_EQ(result[i], expected[i]);
    }
}

// 单个insert 但username和email长度过大(不合法)
TEST(DBtestCmd, InsertOneTooLongAndSelectAll) {
    // 模拟用户输入
    std::string long_username, long_email;
    for(int i=0;i<USERNAME_SIZE;i++) long_username += "a";
    for(int i=0;i<EMAIL_SIZE;i++) long_email += "b";
    std::string insert_cmd = "insert 1 " + long_username + " " + long_email;

    std::vector<std::string> commands = {
        insert_cmd,
        "select",
        ".exit",
    };

    // 存放程序的真实输出
    std::vector<std::string> result = run_script(commands);

    // 存放正确的程序输出
    std::string the_only_row = "(1, " + long_username + ", " + long_email + ")";
    std::vector<std::string> expected = {
        "db > Invalid_args: too long or wrong type or negative",
        "db > ",
        "Executed",
        "db > ",
    };
    // 检查输出
    EXPECT_EQ(result.size(), expected.size());
    for(int i=0;i<result.size();i++){
        EXPECT_EQ(result[i], expected[i]);
    }
}

// 单个insert 但id为负数(不合法)
TEST(DBtestCmd, InsertOneNegativeAndSelectAll) {
    // 模拟用户输入
    std::vector<std::string> commands = {
        "insert -1 user1 person1@example.com",
        "select",
        ".exit",
    };
    // 存放程序的真实输出
    std::vector<std::string> result = run_script(commands);
    // 存放正确的程序输出
    std::vector<std::string> expected = {
        "db > Invalid_args: too long or wrong type or negative",
        "db > ",
        "Executed",
        "db > ",
    };
    // 检查输出
    EXPECT_EQ(result.size(), expected.size());
    for(int i=0;i<result.size();i++){
        EXPECT_EQ(result[i], expected[i]);
    }
}

// 多个insert 但不超过table容量
TEST(DBtestCmd, InsertManyAndSelectAll) {
    std::vector<std::string> commands;
    for(int i=1;i<=1000;i++){
      commands.push_back("insert " + std::to_string(i) + " user" + std::to_string(i) +
                         " person" + std::to_string(i) + "@example.com");
    }
    commands.push_back("select");
    commands.push_back(".exit");

    std::vector<std::string> result = run_script(commands);
    std::vector<std::string> expected;
    for(int i=1;i<=1000;i++){
        expected.push_back("db > Executed");
    }
    expected.push_back("db > ");
    for(int i=1;i<=1000;i++){
        std::string temp = "(" + std::to_string(i) + ", user" + std::to_string(i) + ", person" + std::to_string(i) + "@example.com)";
        expected.push_back(temp);
    }
    expected.push_back("Executed");
    expected.push_back("db > ");
    // 检查输出
    EXPECT_EQ(result.size(), expected.size());
    for(int i=0;i<result.size();i++){
        EXPECT_EQ(result[i], expected[i]);
    }
}

// 多个insert 超过table容量
TEST(DBtestCmd, InsertTooManyAndSelectAll) {
    std::vector<std::string> commands;
    // 其中1-1400可以正常insert 其他无法insert
    for(int i=1;i<=1410;i++){
      commands.push_back("insert " + std::to_string(i) + " user" + std::to_string(i) +
                         " person" + std::to_string(i) + "@example.com");
    }
    commands.push_back("select");
    commands.push_back(".exit");

    std::vector<std::string> result = run_script(commands);
    std::vector<std::string> expected;
    for(int i=1;i<=1400;i++){
        expected.push_back("db > Executed");
    }
    for(int i=1401;i<=1410;i++){
        expected.push_back("db > Error: Table full");
    }
    expected.push_back("db > ");
    for(int i=1;i<=1400;i++){
        std::string temp = "(" + std::to_string(i) + ", user" + std::to_string(i) + ", person" + std::to_string(i) + "@example.com)";
        expected.push_back(temp);
    }
    expected.push_back("Executed");
    expected.push_back("db > ");
    // 检查输出
    EXPECT_EQ(result.size(), expected.size());
    for(int i=0;i<result.size();i++){
        EXPECT_EQ(result[i], expected[i]);
    }
}

// main 函数，启动 Google Test 运行框架
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
