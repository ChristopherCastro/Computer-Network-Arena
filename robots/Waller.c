#define ROBOT_NAME "Waller"

/**
 * Realiza un escaneno con direción variable en función
 * de la posición del robot.
 *
 */
void wallerScan();

/**
 * Retorna la potencia de daño optima para el ataque,
 * varía en función de las decisiones y eventos anteriores.
 *
 * @return int, Daño entre 0 y 10
 */
int wallerFireDmg();

/**
 *
 */
void wallerFire();

/**
 *
 */
void wallerMove();

/**
 * Calcula la dirección de movimiento hacia
 * la pared más proxima desde la posición actual del robot.
 *
 * @return int, Dirección 
 */
int closestWallDir();

int wallReached = 0;		// Indica si se ha alcanzado el borde del tablero o no.
int movingType = 1;			// 1 sentido horario, 0 antihorario.
int look = 0;				// conmutador, mirar o moverse en el turno actual.
int LAST_TURN_FIRE = -1;	// número del último turno en que se realizó un disparo.

/**
 * Método principal invocado al comenzar cada turno.
 *
 * @return void
 */
void robotMain() {
	int movingTo = 0;

	debug drawAmmo();
	debug drawLife();

	if (getY() == 7 || getY() == 0 || getX() == 0 || getX() == 7) {
		wallReached = 1;
	}

	if (!wallReached) {
		movingTo = closestWallDir();

		debug printf("[Robot %d] moviendo a: %d\n", getpid(), movingTo);
		doMove(movingTo);
	} else {
	    if (look) {
	        look = 0;
    		wallerScan(); // escanea en la dirección opuesta
		} else {
		    look = 1;
		    wallerMove();
		}
	}
}

/**
 * Cuando el robot "encuentra" a un robot enemigo, ej. Cuando el
 * radar "impacta" sobre un enemigo.
 *
 * @param char* name, Nombre del enemigo localiza
 * @param int dist, Distancia a la que se encuentra el enemigo, relativa a la posición actual de robot
 * @param int dir, Dirección en la que se encuentra el enemigo
 * @return void
 */
void onScannedRobot(char *name, int dist, int dir) {
	// no disparar si no hay más acciones en el turno
	if (getActionsLeft() != 0) {
		wallerFire(dir, wallerFireDmg());
	} else {
		// whatever
	}
}

/**
 * Cuando el disparo realizado por el robot impacta de
 * forma efectiva sobre un enemigo.
 *
 * @param char* name, Nombre del enemigo
 * @param int dmg, Daño infligido (0-10)
 * @param int dir, Dirección de disparo efectuado
 * @return void 
 */
void onBulletHit(char *name, int dmg, int dir) {
}

/**
 * Cuando el disparao realizado falla al impactar un enemigo.
 *
 * @param int dir, Dirección de disparo efectuado 
 * @return void 
 */
void onBulletMissed(int dir) {
}

/**
 * Cuando el robot muere.
 *
 * @return void 
 */
void onDeath() {
}

/**
 * Cuando un enemigo muere.
 *
 * @return void 
 */
void onRobotDeath(char *name) {
}

/**
 * Cuando el robot impacta con un muro, es decir los límites del tablero.
 *
 * @return void 
 */
void onHitWall() {
	wallReached = 1;
}

/**
 * Cuando el robot impacta con otro tras moverse.
 *
 * @param int dir, Dirección en donde se encuentra el enemigo 
 * @return void 
 */
void onHitRobot(int dir) {
	if (getActionsLeft() > 0) {
		wallerFire(dir, wallerFireDmg());
	}

    // si se encuentra con alguien mientras gira por la pared,
    // comenzar a girar hacia el otro lado y dispararle
    if (wallReached) {
        movingType = movingType == 1 ? 0 : 1;
    } else {
		// si choca con alguien mientras busca el borde -> esquivarlo diagonalmente
		switch (dir) {
			case 8:
				doMove(7);
			break;

			case 6:
				doMove(9);
			break;

			case 2:
				doMove(3);
			break;

			case 4:
				doMove(1);
			break;
		}				
	}
}

/**
 * Cuando el robot finaliza su turno con `R FINISH`
 *
 * @return void 
 */
void onRoundEnded() {
}

/**
 * Se invoca si el robot gana la partida.
 *
 * @return void 
 */
void onWin() {
}

/**
 * Se invoca si el robot pierde la partida.
 *
 * @return void 
 */
void onLose() {
}

/**
 * Se cuando nadie gana la partida, empate.
 *
 * @return void 
 */
void onDraw() {
}

/**
 * Retorna la dirección hacia la pared mas cercana
 * relativa a la posición actual del robot.
 *
 * @return int, Direccón hacia la pared más cercana (8, 6, 2 ó 4).
 */
int closestWallDir() {
	int t = TABLE_HEIGHT - getY();	// top
	int r = TABLE_WIDTH - getX();	// right
	int b = getY();					// bottom
	int l = getX();					// left

	int
        i,
        v[4] = {t, r, b, l},
        min = v[0];

	// buscar el valor mas bajo
	for (i = 1; i < 4; i++) {
		if (v[i] < min) {
			min = v[i];
		}
	}

	// obtener el indice del valor mas bajo
	for (i = 0; i < 4; i++) {
		if (v[i] == min) {
			break;
		}
	}

	switch (i) {
		case 0: return 8; break;
		case 1: return 6; break;
		case 2: return 2; break;
		case 3: return 4; break;
		
	}
}

void wallerMove() {
    if (!wallReached) {
        return;
    }

    int x = getX(), y = getY();

    if (!movingType) {
        if (x >= 0 && 7 > x && y == 0) {
            doMove(6);
        } else if (x > 0 && 7 >= x && y == 7) {
            doMove(4);
        } else if (y > 0 && 7 >= y && x == 0) {
            doMove(2);
        } else if (y >= 0 && 7 > y && x == 7) {
            doMove(8);
        }
    } else {
        if (x > 0 && 7 >= x && y == 0) {
            doMove(4);
        } else if (x >= 0 && 7 > x && y == 7) {
            doMove(6);
        } else if (y >= 0 && 7 > y && x == 0) {
            doMove(8);
        } else if (y > 0 && 7 >= y && x == 7) {
            doMove(2);
        }
    }
}

int wallerFireDmg() {
	// aumentar potencia de daño si la vida es igual o inferior al porcentaje indicado
	int FURY_PERCENT = 15;
	int dmg;
	int opponents = getOpponents();

	if (opponents > 2) {
		// 3 o 4 oponentes -> daño = 3
		dmg = 3;
	} else {
		// 1 o 2 oponentes -> daño = 4
		dmg = 4;
	}

	if (LAST_TURN_FIRE != -1 && LAST_TURN_FIRE != getTurn()) {
		// 4 (o más) turnos sin disparar -> aumentar en +2 el daño
		// 3 (o más) turnos sin disparar -> aumentar en +1 el daño
		if ((getTurn() - LAST_TURN_FIRE) >= 4) {
			dmg += 2;
		} else if ((getTurn() - LAST_TURN_FIRE) >= 3) {
			dmg += 1;
		}
	}

	// aumentar potencia de daño si la vida es inferior a FURY_PERCENT%
	if (getLife() <= (FURY_PERCENT * DEFAULT_LIFE / 100)) {
		// si está en 1 vs 1 -> aumentar en +2 el daño
		// aumentar en +1 en cualquier otro caso.
		if (opponents == 1) {
			dmg += 2;
		} else {
			dmg += 1;
		}
	}

	return dmg;
}

void wallerFire(int dir, int dmg) {
	doFire(dir, wallerFireDmg());

	LAST_TURN_FIRE = getTurn();
}

void wallerScan() {
    if (!wallReached) {
        return;
    }

    int x = getX(), y = getY();

    if (x == y && y == 0) {
        doScan(9);
    } else if (x == 0 && y == 7) {
        doScan(3);
    } else if (x == y && y == 7) {
        doScan(1);
    } else if (y == 0 && x == 7) {
        doScan(7);
    } else if (x > 0 && 7 > x && y == 0) {
        doScan(8);
    } else if (x > 0 && 7 > x && y == 7) {
        doScan(2);
    } else if (y > 0 && 7 > y && x == 0) {
        doScan(6);
    } else if (y > 0 && 7 > y && x == 7) {
        doScan(4);
    }
}
