关于函数指针

```c
((void (*)(void)) (ELFHDR->e_entry))();
```

在C语言中，函数名代表了函数在内存中的地址。因此，可以使用指向函数的指针来调用函数。下面是函数指针调用的基本语法：

```c
return_type (*function_pointer_name)(parameter_types) = &function_name;
```

* `return_type`：函数的返回类型。
* `function_pointer_name`：函数指针的名称。
* `parameter_types`：函数的参数类型。
* `function_name`：要指向的函数名。

在提供的代码中，这行代码：

```c
((void (*)(void)) (ELFHDR->e_entry))();
```

首先，我们需要理解 `(ELFHDR->e_entry)` 是一个指向函数的指针。`ELFHDR->e_entry` 是一个变量，其中存储了程序的入口点函数的地址。

接着，我们对这个指针进行了转换：

```c
(void (*)(void))
```

这段代码表示将 `ELFHDR->e_entry` 的地址转换为一个不接受任何参数并返回 `void` 的函数指针。

最后，我们使用括号 `()` 来调用这个函数指针，即执行指向的函数。因为这个函数没有参数，所以括号内为空。

综合起来，这行代码的作用是通过函数指针调用了程序的入口点函数，从而启动了程序的执行。

下面回答实验指导中的四个问题

1. **At what point does the processor start executing 32-bit code? What exactly causes the switch from 16- to 32-bit mode?**
   在地址0x00007c32时候，编译器将程序编译成32位的可执行文件，从这个时候开始，处理器开始执行32-bit code，具体来说是因为，boot程序将CPU的工作模式设置成了保护模式，来访问1MB以上的内存，而为了寻址，处理器得采用32位模式。
2. **What is the last instruction of the boot loader executed, and what is the first instruction of the kernel it just loaded?**
   boot loader最后执行的指令是在上面提到的 `((void (*)(void)) (ELFHDR->e_entry))();`这句话用于执行位于磁盘第一个页的ELF文件中的程序。内核刚运行时第一个加载的指令是 `movw   $0x1234,0x472` 我也不知道这是干什么的
3. **Where is the first instruction of the kernel?**
   内核的第一条指令位于 `0x10000c`的内存地址处
4. **How does the boot loader decide how many sectors it must read in order to fetch the entire kernel from disk? Where does it find this information?**
   在 `main.c`文件中定义了 `#define  SECTSIZE    512 `应该是在这里申明了区块大小为512字节，但是这个信息应该是包含在磁盘最开始的ELF文件中的
