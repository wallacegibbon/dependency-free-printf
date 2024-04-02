mkdir -p build

gcc -c -o build/dfp_lib.o dfp_lib.c

compile_and_run_test() {
	F=$(basename $1)
	gcc -I. -Itest/ -o build/$F.elf $1 build/dfp_lib.o
	./build/$F.elf
}

for i in test/*.c; do
	compile_and_run_test $i
done
