#include "tests.h"
#include "result/result.h"
#include <include/format.hpp>
#include <iostream>

void test_result_handling() {
    // 测试Ok状态
    result::Result<int,std::string> ok_result = result::Ok(42);
    if (ok_result.is_ok() && ok_result.unwrap() == 42) {
        StringFlow::println("✅ Ok test passed").unwrap();
    }

    // 测试Err状态
    result::Result<int,std::string> err_result =result::Err(std::string("error message"));
    if (err_result.is_err() && err_result.unwrap_err() == "error message") {
        StringFlow::println("✅ Err test passed").unwrap();
    }
}

void test_string_formatting() {
    // 测试字符串格式化
    auto fmt_test = StringFlow::println("{1} {0:.2} {2:x}", "world", 3.1415, 255);
    if (fmt_test.is_ok() && fmt_test.unwrap() == 3) {
        StringFlow::println("✅ Format test passed").unwrap();
    }

    // 测试错误格式
    auto err_fmt = StringFlow::println("{} {}", 123.456, "invalid");
    if (err_fmt.is_err()) {
        StringFlow::println("✅ Format error handling test passed").unwrap();
    }
}