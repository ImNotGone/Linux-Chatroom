#include "common.h"

/* global connection status variable */
int alive = 1;
/* para atrapar el '\n' */
#define CLEAN_BUFFER {while(getchar() != '\n');}
#define inIpRange(NUM) (((NUM) >= 0 && (NUM) <= 255)? 1:0)
/* crea el socket para conectarme al servidor */
int setUpSocket(short port, unsigned int addr, SAIN * serverAddr);
/* fancy looking console :)*/
void overwriteStdout();
/* getchar hasta \n o hasta len */
char * getMsg(char * msg, int len);
/* funcion invocada en un thread para enviar mensajes al servidor */
void * sendToServ(void * arg);
/* funcion invocada en un thread para imprimir el mensaje del resto a la consola */
void * recvFromServ(void * arg);
/* cambia el estatus de mi variable global "alive" */
void killConn();

int main (int argC, char * argV[]) {
    if (argC != 2) {
        perror("Error en cantidad de parametros.\nDebe llamar usando `./client \"Ip del servidor al que desea conectarse\"`");
        exit(1);
    }
    SAIN serverAddr;
    int addr = inet_addr(argV[1]);
    if (addr < 0) {
        perror("La ip provista es invalida");
        exit(1);
    }
    int servSocket = setUpSocket(SERVER_PORT, addr, &serverAddr);
    int pServerSocket[] = {servSocket};
    char alias[ALIAS_LEN];

    /* si presiono cntrl+c (SIGINT), llamo a killConn en vez de terminar el programa */
    signal(SIGINT, killConn);

    do {
        printf("Enter an alias [MAX LEN = %d]: ", ALIAS_LEN);
        getMsg(alias, ALIAS_LEN);
        putchar('\n');
    } while (strlen(alias) >= ALIAS_LEN);

    check4Err(connect(servSocket, (SA*)&serverAddr, sizeof(serverAddr)), "connect err.");
    printf("[Connection to ");
    printAddr(serverAddr.sin_addr.s_addr);
    printf(" established]\n");
    send(servSocket, alias, ALIAS_LEN, 0);
    puts("=== YOU JOINED THE CHAT ROOM ===");

    pthread_t sendThread;
    if(pthread_create(&sendThread, NULL, &sendToServ, (void*)pServerSocket) != 0) {
        perror("send pthread err.");
        killConn();
    }

    pthread_t recvThread;
    if(pthread_create(&recvThread, NULL, &recvFromServ, (void*)pServerSocket) != 0) {
        perror("recv pthread err.");
        killConn();
    }

    while (alive) {};
    send(servSocket, EXIT_MSG, sizeof(EXIT_MSG)+1, 0);
    close(servSocket);
    return 1;
}

int setUpSocket(short port, unsigned int addr, SAIN * serverAddr) {
    int clientSocket;
    check4Err((clientSocket = socket(AF_INET, SOCK_STREAM, TCP)), "Socket creation err.");

    serverAddr->sin_family = AF_INET;
    serverAddr->sin_addr.s_addr = addr;
    serverAddr->sin_port = htons(port);

    return clientSocket;
}

void overwriteStdout() {
    printf("\r%s","> ");
    fflush(stdout);
}

char * getMsg(char * msg, int len) {
    int c, dim = 0;
    while((c = getchar()) != '\n' && dim < len) {
        msg[dim++] = c;
    }
    msg[dim] = '\0';
    return msg;
}

void killConn() {
    alive = 0;
}

void * recvFromServ(void * arg) {
    char RECV[MAXSERVERSENDLINE] = {0};
    int serverSocket = *((int *)arg);
    int Recvd;
    while(alive) {
        if ((Recvd = recv(serverSocket, RECV, MAXSERVERSENDLINE, 0) >= 0)) {
            trim(RECV, MAXSERVERSENDLINE);
            if(Recvd > 0) {
                printf("%s\n", RECV);
                overwriteStdout();
            } else {
                perror("Recv err.");
                killConn();
            }
            memset(RECV, 0, MAXSERVERSENDLINE);
        }
    }
    return NULL;
}

void * sendToServ(void * arg) {
    char MSG[MAXCLIENTSENDLINE] = {0};
    int serverSocket = *((int *)arg);

    while(alive) {
        overwriteStdout();
        if(strcmp(getMsg(MSG, MAXCLIENTSENDLINE), EXIT_MSG) == 0) {
            killConn();
        } else {
            send(serverSocket, MSG, MAXCLIENTSENDLINE, 0);
        }
        memset(MSG, 0, MAXCLIENTSENDLINE);
    }
    return NULL;
}