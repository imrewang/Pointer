# shared_ptr、weak_ptr

shared_ptr、weak_ptr C 实现

添加 shared_ptr、weak_ptr，可以使代码更加安全、可靠、易于维护。

果只是使用C++完成，网上有许多教程，但是我们的最终目标是使用C，所以还是有些许差别，但只要突破几个关键点，之后的工作还是比较容易的。

关于shared_ptr、weak_ptr的实现细节代码文件里有，这里就不赘述，下面主要讲解四个学习到的点。

## （一）结构体设计

1.sp_counter_t
使用 sp_counter_t 结构体的主要作用是实现计数器的共享和管理。在使用 shared_ptr 和 weak_ptr 时，需要跟踪指向资源的引用计数，以确保资源在不再被引用时能够被正确释放。

如果不使用计数器结构体，每个 shared_ptr 和 weak_ptr 对象都需要维护自己的引用计数变量，这样会导致代码冗余，并且容易出现引用计数不一致的问题。
```
typedef struct sp_counter {
    int cnt_share;
    int cnt_weak;

    void* resourse;
    sp_deleter_fn_t deleter;
} sp_counter_t;
```

使用 sp_counter_t 结构体为我们带来了以下的方便之处：

1.简化代码：使用计数器结构体可以避免每个 shared_ptr 和 weak_ptr 对象都需要维护自己的引用计数变量，从而简化代码。

2.统一管理：使用计数器结构体可以统一管理 shared_ptr 和 weak_ptr 对象的引用计数，避免出现引用计数不一致的问题。

3.支持多线程：使用计数器结构体可以支持多线程环境下的共享资源管理，避免出现竞态条件的问题。

4.支持自定义删除器：计数器结构体中包含了一个用于删除资源的函数指针 deleter，可以支持自定义删除器的使用，增强了资源管理的灵活性。

2. sp_shared_ptr_t、 sp_weak_ptr_t

```
typedef struct sp_shared_ptr {
    struct sp_counter* counter;
} sp_shared_ptr_t;
```

sp_shared_ptr_t 结构体包含一个指向 sp_counter_t 结构体的指针 counter。

```
typedef struct sp_weak_ptr {
    struct sp_counter* counter;
} sp_weak_ptr_t;
```
sp_weak_ptr_t 结构体也包含一个指向 sp_counter_t 结构体的指针 counter。weak_ptr 对象并不会增加 cnt_share 计数器。当 cnt_share 计数器为 0 时，资源将被释放，因此 weak_ptr 对象需要检查资源是否已经被释放。

## （二）__attribute__((cleanup(XXX)))

定义宏 on_destroy，它使用了 GCC 的 attribute((cleanup)) 扩展。__attribute__((cleanup(...)))修饰了一个变量，在它的作用域结束时可以自动执行一个指定的方法。
```
#define on_destroy(T, _cleanup)  __attribute__((cleanup(_cleanup))) T
```
使用 on_destroy 宏自动释放资源，避免了手动释放资源的繁琐和容易出错的问题。同时，可以支持 RAII（资源获取即初始化）编程范式，将资源的获取和释放封装在同一个对象中，提高代码的可读性和可维护性。

## （三）#define的使用

C语言中，可以用 #define 定义一个标识符来表示一个常量。其特点是：定义的标识符不占内存，只是一个临时的符号，预编译后这个符号就不存在了。

本项目中，使用#define简化编写手动分配和释放内存的函数的问题，如：
```
#define MAKE_SHARED(var, T) on_destroy(sp_shared_ptr_t*, shared_ptr_destructor) var = shared_ptr_new(malloc(sizeof(T)))
```
同时，使用 #define可以提高代码的可维护性，避免了因为手动创建或销毁对象而导致的代码冗余和重复编写的问题。

## （四）在C中实现“重载”

__builtin_choose_expr是GCC内置宏，它可以根据条件选择两个表达式之一进行求值。
```
__builtin_choose_expr(CONDITION, EXP1, EXP2)
```
如果CONDITION为真，则求值EXP1，否则求值EXP2。它的返回值是表达式EXP1或EXP2的返回值，具体取决于CONDITION的值。

在本项目中，我们使用__builtin_choose_expr来实现类似函数重载的功能。
```
#define ptr_use_count(A)\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A),sp_shared_ptr_t*),\
              shared_ptr_use_count(*(sp_shared_ptr_t**)&A),\
    __builtin_choose_expr(__builtin_types_compatible_p(typeof(A),sp_weak_ptr_t*),\
              weak_ptr_use_count(*(sp_weak_ptr_t**)&A),\
   printf("don't support this type parameter,please check again\n")))
```  
在这个代码例子中，使用了内置函数__builtin_types_compatible_p来检查传递给函数的参数类型是否与指定类型匹配。如果匹配，就会调用相应的函数。如果不匹配，则会输出一条错误消息。这种方法可以用于支持多种类型的共享指针和弱指针，并且可以方便地添加新的类型支持。

---------------------------

虽然初步完成了shared_ptr、weak_ptr，但仍有以下不足：

1.shared_ptr、weak_ptr功能还未彻底实现

在本次实验中，我尝试使用C完成了C++标准库中的shared_ptr和weak_ptr，以实现更加高效和灵活的内存管理。

然而，由于时间和技术限制，我并未完全实现这些拓展功能。例如，我尚未实现众多的运算符重载。未来的改进方向可以是实现运算符重载与加入模板。

2.语言转换程序过于简单

convert.py使用正则表达式进行替换，可能会出现误替换。如果正则表达式的模式不够准确，可能会替换掉不应该替换的字符串，导致程序出错。

可能的改进方向：
用Rust实现一个Lua解释器：https://wubingzheng.github.io/build-lua-in-rust/zh/
