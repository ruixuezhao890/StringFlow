# StringFlow

C++17实现的现代化错误处理与字符串格式化工具库，提供类似Rust的Result类型和高效格式化工具。

## 特性

✅ 强类型错误处理：提供`Result<T, E>`类型实现Rust风格的错误处理  
✨ 链式调用支持：`and_then`/`or_else`方法链式处理结果  
📦 零依赖：仅需C++标准库支持  
🚀 高性能格式化：基于constexpr编译期字符串处理  

## 安装

```bash
git clone https://github.com/yourname/stringflow.git
cd stringflow
mkdir build && cd build
cmake ..
make install
```

## 使用示例

```cpp
#include <stringflow/result.h>

Result<int, std::string> divide(int a, int b) {
    if (b == 0) return Err("Division by zero");
    return Ok(a / b);
}

void example() {
    auto result = divide(10, 2)
        .and_then([](int v) { return Ok(v * 2); })
        .map_error([](auto err) { return "CALC ERROR: " + err; });

    if (result.is_ok()) {
        std::cout << "Result: " << result.unwrap();
    }
}
```

## StringFlow格式化示例

```cpp
#include <stringflow/format.h>

void format_examples() {
    // 基本格式化
    StringFlow::println("Hello {}! PI≈{:.2}", "World", 3.14159).unwrap();
    
    // 类型转换与错误处理
    auto hex = StringFlow::println("{:#x}", 255)
        .map_error([](auto err) { 
            return "HEX ERROR: " + StringFlow::format_error_to_string(err);
        });
    
    // 格式化选项配置
    StringFlow::println("{:*>10}{:+}\n", 42, -3.14)
        .and_then([](size_t len) {
            return StringFlow::println("Formatted {} chars", len);
        });
}
```

## 贡献

欢迎通过Issue提交问题或PR参与开发，请遵循：
1. 保持C++17兼容性
2. 添加完整的单元测试
3. 使用Clang-format格式化代码

## 许可证

MIT License © 2024 YourName