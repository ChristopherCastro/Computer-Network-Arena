/**
 * Constantes de información del juego
 */
#define DEBUG 0
#define DEFAULT_LIFE 20
#define DEFAULT_AMMO 10
#define ACTIONS_PER_TURN 3
#define TABLE_WIDTH 7
#define TABLE_HEIGHT 7
#define TRUE 1
#define FALSE 0

/**
 * Macro de depuración.
 * Modo de uso:
 * debug printf(...);
 */
#if DEBUG > 0
#define debug if(1)
#else
#define debug if(0)
#endif 

/**
 * Definición de códigos de señales personalizadas
 */
#define S_ACCEPTED SIGRTMIN+1
#define S_WIN SIGRTMIN+2
#define S_LOSE SIGRTMIN+3
#define S_DRAW SIGRTMAX-1
#define S_URTURN SIGRTMAX-2
#define NTFY_ROBOT_PAUSE SIGRTMIN+4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

void s_cont(int s);
void s_accepted(int s);
void s_win(int s);
void s_lose(int s);
void s_draw(int s);

void robotMain();
void doTurn();
void endRound();
void pauseTree();
void readTurnPipe();
int resumeController();
int resumeRobot();
int * targetPosition(int dir, int dist);

/**
 * Drawing methods
 */
void drawAmmo();
void drawLife();
void drawRobotPosition();
void drawPosition();
void drawFire(int dir);

/**
 * DO methods
 */
void doScan(int dir);
void doFire(int dir, int dmg);
void doMove(int dir);

/**
 * GET methods
 */
int *getPos();
int getX();
int getY();
char *getName();
int getLife();
int getAmmo();
int getOpponents();

/**
 * ON methods
 */
void onScannedRobot(char *name, int dist, int dir);
void onBulletHit(char *name, int dmg, int dir);
void onBulletMissed(int dir);
void onDeath();
void onRobotDeath(char *name);
void onHitWall();
void onHitRobot(int dir);
void onRoundEnded();
void onWin();
void onLose();
void onDraw();


