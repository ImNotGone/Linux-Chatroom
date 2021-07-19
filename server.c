#include "common.h"

typedef struct{
    SAIN addr;
    int socket;
    int id;
    char alias[ALIAS_LEN];
} tClient;

/* clients es una variable global, donde tengo registrados los clientes actuales */
tClient * clients[SERVER_BACKLOG] = {0};
/* variable global para cantidad de conectados */
int connected = 0;
/* inicio el lock */
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

/* recibe el puerto y la cantidad de conexiones que admite en simultanio */
int setUpServer(short port, int backlog);
/* accepta conexiones */
int recvConn(int serverSocket, SAIN * clientAddr);
/* se encarga de manejar las conecxiones activas */
void * handleConnection(void * arg);
/* agrega un cliente al array de clientes */
void addToArr(tClient * client);
/* saca a un cliente del array de clientes */
void rmFromArr(int clientId);
/* le envia a todos los clientes, menos el que envio el mensaje, el mensaje a enviar */
void sendAll(char * msg, int clientId);

int main() {
    int serverSocket = setUpServer(SERVER_PORT, SERVER_BACKLOG);
    puts("[Server started]");
    pthread_t threadId;
    int ids = 0;
    // SIGPIPE es el error, si intento escribir a un socket que ya cerro
    // SIG_IGN es ignorar la senial
    signal(SIGPIPE, SIG_IGN);

    while(true) {
        SAIN clientAddr;
        int clientSocket = recvConn(serverSocket, &clientAddr);

        printf("[Connection from ");
        printAddr(clientAddr.sin_addr.s_addr);

        if(connected + 1 > SERVER_BACKLOG) {
            puts(" rejected] : Backlog = FULL");
            close(clientSocket);
        } else {

            puts(" recieved]");
            tClient *client = malloc(sizeof(tClient));
            client->addr = clientAddr;
            client->socket = clientSocket;
            client->id = ids++;

            addToArr(client);
            connected++;
            pthread_create(&threadId, NULL, &handleConnection, (void *) client);
        }
    }

}

int setUpServer(short port, int backlog) {
    int serverSocket;
    SAIN serverAddr;
    check4Err((serverSocket = socket(AF_INET, SOCK_STREAM, TCP)), "Socket creation err.");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = LOCALIP;
    serverAddr.sin_port = htons(port);

    check4Err(bind(serverSocket, (SA*)&serverAddr, sizeof(serverAddr)), "Bind err.");
    check4Err(listen(serverSocket, backlog), "Listen err.");
    return serverSocket;
}

int recvConn(int serverSocket, SAIN * clientAddr) {
    int addrSize = sizeof(SAIN);
    int clientSocket;
    check4Err(clientSocket = accept(serverSocket, (SA*)clientAddr, (socklen_t*)&addrSize), "Accept err.");
    return clientSocket;
}

void addToArr(tClient * client) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0, added = 0; i < SERVER_BACKLOG && !added; i++) {
        if(clients[i] == NULL) {
            clients[i] = client;
            added = 1;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void rmFromArr(int clientId) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0, rm = 0; i < SERVER_BACKLOG && !rm; i++) {
        if(clients[i] != NULL && clients[i]->id == clientId) {
            clients[i] == NULL;
            rm = 1;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void * handleConnection(void * arg) {
    char SEND[MAXSERVERSENDLINE] = {0}, RECV[MAXCLIENTSENDLINE] = {0}, alias[ALIAS_LEN];
    int active = 1;
    tClient * client = (tClient *)arg;

    if(recv(client->socket, alias, ALIAS_LEN, 0) == SOCKETERR) {
        puts("Alias err.");
        active = 0;
    } else {
        trim(alias, ALIAS_LEN);
        strcpy(client->alias, alias);
        sprintf(SEND,"[\"%s\" has joined the server]", client->alias);
        printf("%s\n", SEND);
        sendAll(SEND, client->id);
    }

    while(active) {
        int Recvd;
        memset(RECV, 0, MAXCLIENTSENDLINE);
        if((Recvd = recv(client->socket, RECV, MAXCLIENTSENDLINE, 0)) >= 0) {
            trim(RECV, strlen(RECV));
            if(Recvd == 0 || strcmp(RECV, EXIT_MSG) == 0) {
                sprintf(RECV, "[%s is leaving]\n", client->alias);
                sendAll(RECV, client->id);
                printf("%s", RECV);
                active = 0;
            }else if(RECV[0] != '\0') {
                memset(SEND, 0, MAXSERVERSENDLINE);
                sprintf(SEND, "%s: %s\n", client->alias, RECV);
                sendAll(SEND, client->id);
                printf("%s", SEND);
            }
        }

    }
    printf("[Closing %s's connection]\n", client->alias);
    connected--;
    close(client->socket);
    rmFromArr(client->id);
    free(client);
    pthread_detach(pthread_self());
    return NULL;
}

void sendAll(char * msg, int clientId) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < SERVER_BACKLOG; i++) {
        if(clients[i] != NULL && clients[i]->id != clientId) {
            if(send(clients[i]->socket, msg, MAXSERVERSENDLINE, 0) < 0) {
                printf("[!] ERROR SENDING MSG TO %s, addr = ", clients[i]->alias);
                printAddr(clients[i]->addr.sin_addr.s_addr);
                putchar('\n');
            }
        }
    }
    pthread_mutex_unlock(&client_mutex);
}
