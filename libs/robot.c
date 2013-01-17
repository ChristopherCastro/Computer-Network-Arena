int main(int argc,char *argv[]) {
	struct sockaddr_in server;
	int
		sock,
		port,
		err;
	char
		buff[2000];

	if (argc != 3) {
		printf("Numero de argumentos invalido\n");
		printf("Modo de uso:\n\n");
		printf("\t robot [IP] [PORT]\n\n");
		printf("\t IP: dirección ip del servidor. e.j. 127.0.0.1\n");
		printf("\t PORT: número del puerto del servidor. e.j. 5600\n\n");
		exit(-1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);
	port = atoi(argv[2]);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = inet_addr(argv[1]);

	err = connect(sock, (struct sockaddr *)&server, sizeof(server));

	if (err == -1) {
		printf("No se pudo establecer la conexión con el servidor\n");
		exit(-1);
	}

	FD = fdopen(sock, "w+");

    signal(SIGCONT, s_cont);

    if (pipe(pipeFD) == -1) {
        printf("Error al inicial el pipe. pipe() returned -1\n");
        exit(-1);
    }
    
	setbuf(FD, NULL);
	initRobot();

	buff[0] = '\0';

	if (login() == FALSE) {
		printf("Error al iniciar la sesion\n");
		fclose(FD);
		exit(-1);
	}

	CONTROLLER_ID = getpid();
	ROBOT_ID = fork();

    if (ROBOT_ID == -1) {
        printf("Error al iniciar, fork() returned -1\n");
        exit(-1);
    }

	if (ROBOT_ID > 0) {
	    signal(NTFY_ROBOT_PAUSE, pausedRobot);

		// CONTROLLER
		while (_fgets(buff, 2000, FD) != NULL) {
		    debug printf("[Controller %d] msg from server: %s\n", getpid(), buff);

			if (strpos(buff, "S ACCEPTED") != -1) {
				kill(ROBOT_ID, S_ACCEPTED);
			} else if (strpos(buff, "S WIN") != -1) {
				kill(ROBOT_ID, S_WIN);
				break;
			} else if (strpos(buff, "S LOSE") != -1) {
				kill(ROBOT_ID, S_LOSE);
				break;
			} else if (strpos(buff, "S DRAW") != -1) {
				kill(ROBOT_ID, S_DRAW);
				break;
			} else if (strpos(buff, "S URTURN") != -1) {
			    if (resumeRobot() == 0) {
                    write(pipeFD[1], buff, strlen(buff) + 1);
			    }

                debug printf("[Controller %d]: Durmiendo\n", getpid());
                pause();
                debug printf("[Controller %d]: Despertado\n", getpid());
			}
		}

        kill(ROBOT_ID, SIGTERM);
		wait(ROBOT_ID);
		shutdown(sock, 2);
		fclose(FD);
		close(pipeFD[0]);
		close(pipeFD[1]);
		debug printf("## Fin del programa ##\n");
		exit(0);
	} else if (ROBOT_ID == 0) {
		// ROBOT
		ROBOT_ID = getpid();

		signal(S_ACCEPTED, s_accepted);
		signal(S_WIN, s_win);
		signal(S_LOSE, s_lose);
		signal(S_DRAW, s_draw);

    	while(1) {
			// se bloquea a la espera del tubo
    	    doTurn();
    	}
	}
}

int login() {
	char buff[20];

	fprintf(FD, "R NAME %s\n", Robot.name);
	_fgets(buff, 20, FD);

	if (strcmp(buff, "S ERROR FULL") == 0) {
		debug printf("Arena is full, disconnect\n");

		return FALSE;
	} else if (strcmp(buff, "S ERROR NAME") == 0) {
		debug printf("name not given or invalid, disconnect\n");

		return FALSE;
	} else if (strcmp(buff, "S WAIT") == 0) {
		debug printf("Accepted, waiting for challenge\n");

		return TRUE;
	} else {
		return FALSE;
	}
}
