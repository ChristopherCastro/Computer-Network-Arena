
int
    ROBOT_ID,
	ROBOT_PAUSED = 0,
    CONTROLLER_ID,
    ACTIONS_LEFT = ACTIONS_PER_TURN,
    pipeFD[2],
	FORCE_END = 0,
	PREVIOUS_ACTIONS[3] = {0, 0, 0}; // 0: mover, 1: escanear, 2:disparar

/**
 * Descriptor de fichero socket para la comunicación con el servidor
 *
 */
FILE *FD;

/**
 * Estructura de datos del robot.
 *
 * @var RobotStruct
 */
struct RobotStruct {
	char name[20];	// nombre del Robot
	int life;		// puntos de vida
	int ammo;		// munición disponible
	int pos[2];		// posicón actual del Robot
	int turn;		// numero del turno actual
	int opponents;	// número de oponentes en pie
};

struct RobotStruct Robot;

/**
 * Devuelve un array de cadenas, siendo cada una de ellas una subcadena del parámetro
 * `str` formado por la división según los delimitadores indicados en el parámetro `delimiter`.
 *
 * @param char *** arr_ptr, array de salida
 * @param char * str, La cadena de entrada
 * @param char delimiter, La cadena delimitadora. 
 * @return int, número de subcadenas encontradas
 */
int explode(char ***arr_ptr, char *str, char delimiter);

/**
 * Reempleza un único caracter de la cadena indicada.
 *
 * @param char from, Caracter que debe ser reemplazado
 * @param char to, Nuevo caracter
 * @param char *str, Cadena de texto original en dónde reemplazar
 * @return void
 */
void charReplace(char from, char to, char *str);

/**
 * Método alternativo a fgets().
 *
 * Remueve los retornos de carro `\n` de todos los mensaje.
 *
 * @param char *s, Buffer en donde almacenar la lectura
 * @param int size, Número de caracteres a leer
 * @param FILE *stream, Descriptor de fichero
 * @return char*
 */
char * _fgets(char *s, int size, FILE *stream);

/**
 * Inicializa la estructura del robot con sus valores por defecto.
 *
 */
void initRobot();

/**
 * Desde una dirección dada retorna la dirección correspondiente al angulo de giro indicado.
 * 
 * Ángulos admitidos: 180, 90, -90, 45, -45. Donde los negativos son rotaciones en sentido antihorario,
 * y en sentido horario para los positivios.
 *
 * Por ejemplo si el robot está mirando a la dirección "8", una rotacion en "-45" producirá la dirección "7":
 *
 *  7   [8]   9
 *  4   [5]   6
 *  1    2    3
 *
 *  [7]  8    9
 *  4   [5]   6
 *  1    2    3
 *
 * @param int dir, Dirección actual
 * @param int d, Ángulo de rotación (180, 90, -90, 45, -45)
 * @return int Nueva dirección
 */
int rotate(int dir, int d);

/**
 * Representación gráfica de un disparo en el tablero.
 *
 */
void drawFire(int dir);

/**
 * Imprime por pantalla las variables de estado del robot.
 *
 */
void printRobot();

/**
 * Encuentra la posición numérica de la primera ocurrencia del
 * needle (aguja) en el string haystack (pajar).
 * 
 * @param haystack
 * @param needle
 * @return -1 si no se encuentra
 */
int strpos(char *haystack, char *needle);


/*******************************************************
 * 
 * Definición de funciones
 * 
 *******************************************************/

void initRobot() {
	strcpy(Robot.name, ROBOT_NAME);

	Robot.life = DEFAULT_LIFE;
	Robot.ammo = DEFAULT_AMMO;
	Robot.pos[0] = -1;
	Robot.pos[1] = -1;
	Robot.turn = 0;
}

void charReplace(char from, char to, char *str) {
    int i = 0;
    int len = strlen(str) + 1;

    for(i = 0; i < len; i++) {
        if(str[i] == from) {
            str[i] = to;
        }
    }
}

char * _fgets(char *s, int size, FILE *stream) {
    char *r;

    r = fgets(s, size, stream);
    charReplace('\n', '\0', s);

    return r;
}

int explode(char ***arr_ptr, char *str, char delimiter) {
	char *src = str, *end, *dst;
	char **arr;
	int size = 1, i;

	while ((end = strchr(src, delimiter)) != NULL) {
		++size;
		src = end + 1;
	}

	arr = malloc(size * sizeof(char *) + (strlen(str) + 1) * sizeof(char));
	src = str;
	dst = (char *) arr + size * sizeof(char *);

	for (i = 0; i < size; ++i) {
		if ((end = strchr(src, delimiter)) == NULL) {
			end = src + strlen(src);
		}

		arr[i] = dst;
		strncpy(dst, src, end - src);
		dst[end - src] = '\0';
		dst += end - src + 1;
		src = end + 1;
	}

	*arr_ptr = arr;

	return size;
}

// TODO: posicion de enemigo en una direccion dada y conocida su distancia.
int * targetPosition(int dir, int dist) {
	int rx, ry;
	static int tPos[2];

	// posición actual del robot
	rx = getX();
	ry = getY();

	switch (dir) {
		case 4:
			tPos[0] = rx - dist;
			tPos[1] = ry;
		break;

		case 7:
			tPos[0] = rx - dist;
			tPos[1] = ry + dist;
		break;

		case 8:
			tPos[0] = rx;
			tPos[1] = ry + dist;
		break;

		case 9:
			tPos[0] = rx + dist;
			tPos[1] = ry + dist;
		break;

		case 6:
			tPos[0] = rx + dist;
			tPos[1] = ry;
		break;
		
		case 3:
			tPos[0] = rx + dist;
			tPos[1] = ry - dist;
		break;
		
		case 2:
			tPos[0] = rx;
			tPos[1] = ry - dist;
		break;
		
		case 1:
			tPos[0] = rx - dist;
			tPos[1] = ry - dist;
		break;
	}

	return tPos;
}

int rotate(int dir, int d) {
	int new = 1;

	if (d == 180) {
		switch (dir) {
			case 8:	new = 2; break;
			case 9:	new = 1; break;
			case 6:	new = 4; break;
			case 3:	new = 7; break;

			case 2:	new = 8; break;
			case 1:	new = 9; break;
			case 4:	new = 6; break;
			case 7:	new = 3; break;
		}
	} else if (d == 90) {
		switch (dir) {
			case 8:	new = 6; break;
			case 9:	new = 3; break;
			case 6:	new = 2; break;
			case 3:	new = 1; break;

			case 2:	new = 4; break;
			case 1:	new = 7; break;
			case 4:	new = 8; break;
			case 7:	new = 9; break;
		}
	} else if (d == -90) {
		switch (dir) {
			case 8:	new = 4; break;
			case 9:	new = 7; break;
			case 6:	new = 8; break;
			case 3:	new = 9; break;

			case 2:	new = 6; break;
			case 1:	new = 3; break;
			case 4:	new = 2; break;
			case 7:	new = 1; break;
		}
	} else if (d = 45) {
		switch (dir) {
			case 8:	new = 9; break;
			case 9:	new = 6; break;
			case 6:	new = 3; break;
			case 3:	new = 2; break;

			case 2:	new = 1; break;
			case 1:	new = 4; break;
			case 4:	new = 7; break;
			case 7:	new = 8; break;
		}
	} else if (d = -45) {
		switch (dir) {
			case 8:	new = 7; break;
			case 9:	new = 8; break;
			case 6:	new = 9; break;
			case 3:	new = 6; break;

			case 2:	new = 3; break;
			case 1:	new = 2; break;
			case 4:	new = 1; break;
			case 7:	new = 4; break;
		}
	}

	return new;
}

int strpos(char *haystack, char *needle) {
	char *p = strstr(haystack, needle);

	if (p) {
		return p - haystack;
	}

	return -1;
}

void s_cont(int s) {
    if (ROBOT_ID == getpid()) {
		if (ROBOT_PAUSED) {
			Robot.turn++;
			ACTIONS_LEFT = ACTIONS_PER_TURN;
			PREVIOUS_ACTIONS[0] = 0;
			PREVIOUS_ACTIONS[1] = 0;
			PREVIOUS_ACTIONS[2] = 0; 

	        readTurnPipe();
		}

		ROBOT_PAUSED = 0;
    }

    return;
}

void s_accepted(int s) {
	return;
}

void s_win(int s) {
	onWin();
}

void s_lose(int s) {
	onLose();
}

void s_draw(int s) {
	onDraw();
}

void readTurnPipe() {
    char
        buff[2000],
        **parts;

    read(pipeFD[0], buff, sizeof(buff));
    explode(&parts, buff, ' ');

    Robot.life = atoi(parts[2]);
    Robot.pos[0] = atoi(parts[3]);
    Robot.pos[1] = atoi(parts[4]);
    Robot.ammo = atoi(parts[5]);
    Robot.opponents = atoi(parts[6]);
}

void doTurn() {
	int i = 0, j;

    ACTIONS_LEFT = ACTIONS_PER_TURN;

    readTurnPipe();
	Robot.turn++;

	while (getActionsLeft() && !FORCE_END) {
		i++;

		if (!getActionsLeft()) {
			break;
		}

        debug printf("Turno %d.%d\n", Robot.turn, i);

		j = getActionsLeft();

		robotMain();

		if (getActionsLeft() == j) {
			ACTIONS_LEFT--;
		}
	}

	if (FORCE_END) {
		FORCE_END = 0;
	}

	endRound();
	debug printf("[Robot %d]: end of doTurn()\n", getpid());
}

void endRound() {
	fprintf(FD, "R FINISH\n");
    onRoundEnded();

	PREVIOUS_ACTIONS[0] = 0;
	PREVIOUS_ACTIONS[1] = 0;
	PREVIOUS_ACTIONS[2] = 0;

    resumeController();
}

int resumeController() {
	return kill(CONTROLLER_ID, SIGCONT);
}

int resumeRobot() {
    int r = 0;

    if (ROBOT_PAUSED == 1) {
        ROBOT_PAUSED = 0;
    	r = kill(ROBOT_ID, SIGCONT);
	}

	return r;
}

// FIX: signal
void pausedRobot(int s) {
    ROBOT_PAUSED = 1;
    debug printf("####### ##################");
    exit(-1);
}

void pauseTree() {
    debug printf("### Arbol de estados pausado ###\n");
	ROBOT_PAUSED = 1;
	kill(getppid(), NTFY_ROBOT_PAUSE);
    pause();
    ACTIONS_LEFT = ACTIONS_PER_TURN; //innecesario
    debug printf("### Arbol de estados continua ###\n");
}

void doScan(int dir) {
	if (PREVIOUS_ACTIONS[1] == 1) {
		FORCE_END = 1;
		return;
	} else {
		PREVIOUS_ACTIONS[1] = 1;
	}

	if (ACTIONS_LEFT <= 0) {
	    endRound();
		pauseTree();
	}

    ACTIONS_LEFT--;

    char buff[2000];

    fprintf(FD, "R SEEK %d\n", dir);
    _fgets(buff, 2000, FD);

    if (strpos(buff, "S SEEK RETURNED") != -1) {
	    char **parts;

	    explode(&parts, buff, ' ');
	    onScannedRobot(parts[3], atoi(parts[4]), dir);
    }
}

/**
 * Disparar en la dirección y con la potencia indicada.
 *
 */
void doFire(int dir, int dmg) {
	if (PREVIOUS_ACTIONS[2] == 1) {
		FORCE_END = 1;
		return;
	} else {
		PREVIOUS_ACTIONS[2] = 1;
	}

	if (ACTIONS_LEFT <= 0) {
	    endRound();
		pauseTree();
	}

	debug drawFire(dir);

    ACTIONS_LEFT--;

    char buff[50], **parts;

    fprintf(FD, "R SHOOT %d %d\n", dir, dmg);
    _fgets(buff, 50, FD);

    if (strpos(buff, "S SHOOT MISS") != -1) {
	    onBulletMissed(dir);
    } else if (strpos(buff, "S SHOOT HIT") != -1) {
	    explode(&parts, buff, ' ');
	    onBulletHit(parts[3], atoi(parts[4]), dir);			
    } else {
		printf("ERROR: Respuesta de servidor inesperada!!\n");
	}
}

/**
 * Moverse en la dirección indicada.
 *
 */
void doMove(int dir) {
	if (PREVIOUS_ACTIONS[0] == 1) {
		FORCE_END = 1;
		return;
	} else {
		PREVIOUS_ACTIONS[0] = 1;
	}

	if (ACTIONS_LEFT <= 0) {
	    endRound();
		pauseTree();
	}

    ACTIONS_LEFT--;

    char buff[2000];

    fprintf(FD, "R MOVE %d\n", dir);
    _fgets(buff, 2000, FD);

    if (strpos(buff, "S MOVE BOUNDARY") != -1) {
	    onHitWall();
    } else if (strpos(buff, "S MOVE COLLISION") != -1) {
	    onHitRobot(dir);
    } else if (strpos(buff, "S MOVE OK") != -1) {
	    switch (dir) {
		    case 8:
			    Robot.pos[1]++;
		    break;

		    case 9:
			    Robot.pos[0]++;
			    Robot.pos[1]++;
		    break;

		    case 6:
			    Robot.pos[0]++;
		    break;

		    case 3:
			    Robot.pos[0]++;
			    Robot.pos[1]--;
		    break;

		    case 2:
			    Robot.pos[1]--;
		    break;

		    case 1:
			    Robot.pos[0]--;
			    Robot.pos[1]--;
		    break;
		
		    case 4:
			    Robot.pos[0]--;
		    break;
		
		    case 7:
			    Robot.pos[0]--;
			    Robot.pos[1]++;
		    break;			
	    }
    }

    debug drawPosition();	
}


/**
 * GET METHODS
 */

int getActionsLeft() {
    return ACTIONS_LEFT;
}

int *getPos() {
	return Robot.pos;
}

int getX() {
	return Robot.pos[0];
}

int getY() {
	return Robot.pos[1];
}

char *getName() {
	return Robot.name;
}

int getLife() {
	return Robot.life;
}

int getAmmo() {
	return Robot.ammo;
}

int getOpponents() {
	return Robot.opponents;
}

int getTurn() {
	return Robot.turn;
}

void drawAmmo() {
	int i, municion = getAmmo();

	printf("AMMO: [");

	for (i = 0; i < 10; i++){
		if (i < municion){
			printf("\033[1;33m▮\033[0m");
		} else {
			printf("▯");
		}
	}

	printf("]\n");
}

void drawLife() {
	int i, vida;
	vida = getLife();

	printf("LIFE: [");

	for (i = 0; i < 20; i++){
		if (i < vida){
			printf("\033[31m♥\033[0m");
		} else {
			printf("♡");
		}
	}

	printf("]\n");
}

void drawFire(int dir) {
	int x, y, m;

	if (dir == 9 || dir == 1) {
		m = 1;
	} else if (dir == 7 || dir == 3) {
		m = -1;
	}

	for (y = TABLE_HEIGHT; y >= 0; y--) {
		printf("|");

		for (x = 0; x <= TABLE_WIDTH; x++) {
			if (getX() == x && getY() == y) {
				printf("  R  |");
			} else {
				switch(dir) {
					//DIAGONAL ABAJO IZQUIERDA
					case 1:
						if (getX() > x && getY() > y && (y - getY()) == (m * (x - getX()))) {
							printf("  *  |");
						} else {
							printf("     |");
						}
					break;

					//HORIZONTAL IZQUIERDA
					case 4:
						if (getY() == y && getX() > x ) {
							printf("  *  |");
						} else {
							printf("     |");
						}
					break;

					//DIAGONAL ARRIBA IZQUIERDA
					case 7:
						if (getX() > x && getY() < y && (y - getY()) == (m * (x - getX()))) {
							printf("  *  |");
						} else {
							printf("     |");
						}
					break;

					//VERTICAL ARRIBA
					case 8:
						if (getX() == x && getY() < y ) {
							printf("  *  |");
						} else{
							printf("     |");
						}
					break;

					//DIAGONAL ARRIBA DERECHA
					case 9:
						if (getX() < x && getY() < y && (y - getY()) == (m * (x - getX()))) {
							printf("  *  |");
						} else {
							printf("     |");
						}
					break;

					//HORIZONTAL DERECHA
					case 6:
						if (getY() == y && getX() < x){
							printf("  *  |");
						} else {
							printf("     |");
						}
					break;

					//DIAGONAL ABAJO DERECHA
					case 3:
						if (getX() < x && getY() > y && (y - getY()) == (m * (x - getX()))) {
							printf("  *  |");
						} else {
							printf("     |");
						}
					break;

					//VERTICAL ABAJO
					case 2:
						if (getX() == x && getY() > y) {
							printf("  *  |");
						} else {
							printf("     |");
						}
					break;
				}
			}
		}

		printf("\n");
	}
}

void drawPosition() {
	int x, y;

	for (y = TABLE_HEIGHT; y >= 0; y--) {
		printf("|");

		for (x = 0; x <= TABLE_WIDTH; x++) {
			if (getX() == x && getY() == y) {
				printf("  R  |");
			} else {
				printf("     |");
			}
		}

		printf("\n");
	}
}

