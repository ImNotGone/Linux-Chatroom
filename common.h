#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>

#define TCP 0 /* Protocolo TCP */
#define SERVER_PORT 4545 /* puerto de acceso */
#define LOCALIP INADDR_ANY /* ip local */
#define MAXCLIENTSENDLINE 4096 /* maximo largo de informacion para enviar */
#define ALIAS_LEN 32 /* max user alias len */
#define MAXSERVERSENDLINE (MAXCLIENTSENDLINE + ALIAS_LEN + 8) /* el largo maximo de linea q manda el servidor */
#define EXIT_MSG "exit" /* mensaje de salida del usuario */
#define SERVER_BACKLOG 20 /* Maximmo de connecciones simultaneas que accepta el servidor */
#define SOCKETERR (-1) /* Para usar en check4err */

/* auxliliares */
typedef struct sockaddr_in SAIN;
typedef struct sockaddr SA;

/* si hubo un socket error imprime a salida de error y aborta */
int check4Err(int value, const char * msg);
/* elimina el \r y \n de un string */
void trim(char * str, int len);
/* imprime la ip que se le pasa */
void printAddr(unsigned int addr);

#endif //SERVER_COMMON_H
