## Introduction

This is a simple `printf` implementation that do not have any library dependencies. (including libc)

## Usage

Implement your own puts, then use it just like normal printf.

```c
int my_puts(const char *s);

DFP_PRINTF_INIT(my_puts);
DFP_PRINTF("%s", "hello");
```

Check the returning value on serious programs.

