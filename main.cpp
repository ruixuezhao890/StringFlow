#include <iostream>
#include "StringFlow/include/format.hpp"
#include "examples/tests.h"


int main() {
    // test_result_handling();
    // test_string_formatting();
    auto string_rec = StringFlow::println("hello {1} {0}","world","shangfan is a dog").unwrap();
    auto string_rec1 =StringFlow::println("{1:.2} {0} {2:x} {2:b} {2:o}","hello",242.232,42);
    auto string_rec2 =StringFlow::println(nullptr);
    if (string_rec2.is_err()) {
        StringFlow::println("{}",StringFlow::format_error_to_string(string_rec2.unwrap_err()).c_str()).unwrap();
    }
    StringFlow::println("{:^^30}","hello").unwrap();
    return 0;
}

