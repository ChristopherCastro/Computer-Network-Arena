CC=gcc

robomaker: ./robots/$(robot).c ./libs/common.c
	cat ./libs/robot.h ./robots/$(robot).c ./libs/common.c ./libs/robot.c | $(CC) -o $(robot) -xc -
