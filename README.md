# shared_ptr、weak_ptr

shared_ptr、weak_ptr C 实现

添加 shared_ptr、weak_ptr，可以使代码更加安全、可靠、易于维护。


虽然初步完成了shared_ptr、weak_ptr，但仍有以下不足：

1. shared_ptr、weak_ptr功能还未彻底实现

在本次实验中，我尝试使用C完成了C++标准库中的shared_ptr和weak_ptr，以实现更加高效和灵活的内存管理。

然而，由于时间和技术限制，我并未完全实现这些拓展功能。例如，我尚未实现众多的运算符重载。未来的改进方向可以是实现运算符重载与加入模板。

2.语言转换程序过于简单

convert.py使用正则表达式进行替换，可能会出现误替换。如果正则表达式的模式不够准确，可能会替换掉不应该替换的字符串，导致程序出错。

可能的改进方向：
用Rust实现一个Lua解释器：https://wubingzheng.github.io/build-lua-in-rust/zh/
