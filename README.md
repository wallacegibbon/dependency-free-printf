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

## Example

Here is a sample of `puts` used on STM32 (or similar devices):

```c
int my_puts(const char *s)
{
	int n = 0, c;
	for (c = *s; c; c = *s++, n++) {
		while (UART1->SR & USART_FLAG_TXE) == 0);
		UART1->DR = c;
	}
	while (UART1->SR & USART_FLAG_TC) == 0);
	return n;
}
```
