cmake_minimum_required(VERSION 3.15) # 指定最低 CMake 版本
set(CMAKE_CXX_STANDARD 14)  # 或者 C++14、C++17 等
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_options(-std=c++14)
add_definitions(-DGTEST_IS_THREADSAFE=1)

project(myDatabase)                   # 设置项目名称

# 下载 Google Test
option(USE_LOCAL_GTEST "Use Local Google Test" OFF)

if(USE_LOCAL_GTEST)
  message("Use Local Google Test")
  # # 指定 Google Test 的头文件和库文件的路径
  include_directories(../googletest/googletest/include)
  link_directories(../googletest/build/lib/Debug)
else()
  message("Fetch Online Google Test")
  include(FetchContent)
  FetchContent_Declare(
      gtest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG release-1.11.0
  )

  # 使 FetchContent 工作
  FetchContent_MakeAvailable(gtest)
endif()

# 创建测试可执行文件
add_executable(tests test.cpp)

# 链接 Google Test 库
target_link_libraries(tests gtest gtest_main)

# 启用测试
enable_testing()
add_test(NAME tests COMMAND tests --gtest_color=auto)

add_executable(rundb main.cpp) # 添加可执行文件
