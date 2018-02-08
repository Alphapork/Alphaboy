
output: Alphaboy.o gfx.o helper.o CPU_LR35902.o OPCODES.o
	gcc Alphaboy.o gfx.o helper.o CPU_LR35902.o OPCODES.o -o Alphaboy
Alphaboy.o: Alphaboy.c
	gcc -c Alphaboy.c
gfx.o: gfx.c
	gcc -c gfx.c
helper.o: helper.c
	gcc -c helper.c
CPU_LR35902.o: CPU_LR35902.c
	gcc -c CPU_LR35902.c
OPCODES.o: OPCODES.c
	gcc -c OPCODES.c
clean:
	rm *.o
