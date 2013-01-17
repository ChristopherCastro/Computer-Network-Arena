CC=gcc

robomaker: ./robots/Waller.c ./libs/common.c
	cat ./libs/robot.h ./robots/Waller.c ./libs/common.c ./libs/robot.c | $(CC) -o robot -xc -
