## Introduction

This is a simple `printf` implementation without any library dependencies.

Some decisions were made to make it portable by sacrificing some speed.


## Usage

Implement your own `puts`, then you can use it just like normal printf.

```c
int my_puts(const char *s);

DFP_PRINTF_INIT(my_puts);
DFP_PRINTF("%s", "hello");
```

You should check the return values on serious programs.

## Float

Float number support can be disabled by set env variable

e.g.
```sh
NO_FLOAT=1 ./run-tests.sh
```
