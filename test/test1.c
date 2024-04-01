#include "../dfp_lib.h"
#include <stdint.h>
#include <stdio.h>

int my_puts(const char *s);

int main(int argc, const char **argv) {
	int a;
	long long b;
	float c;

	__dfp_printf_init();
	__dfp_register_puts(my_puts);

	a = printf("hello, world %u %llu ~", 0, (unsigned long long)10);
	printf("\tsize: %d\n", a);

	a = __dfp_printf("hello, world %u %llu ~", 0, (unsigned long long)10);
	__dfp_printf("\tsize: %d\n", a);

	/// 0x1234567812345678 == 1311768465173141112
	b = 0x1234567812345678;
	a = printf("%d, %lld,\t", (int)b, b);
	printf("size: %d\n", a);

	a = __dfp_printf("%d, %lld,\t", (int)b, b);
	__dfp_printf("size: %d\n", a);

	c = 3.1415926;
	printf("%f\n", c);
	__dfp_printf("%f\n", c);

	printf("<*> %p\t(%llu) %c\n", &c, (uintptr_t)&c, '!');
	__dfp_printf("<*> %p\t(%lu) %c\n", &c, &c, '!');

	printf("long value: %llu\n", 140723709965000LU);
	__dfp_printf("long value: %llu\n", 140723709965000LU);

	return 0;
}

int my_puts(const char *s) {
	return fputs(s, stdout);
}
