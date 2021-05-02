/*
    Autores:
        Francisco Rosal
        Gian Luca Rivera
*/

#include <string.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define MAX_BUFFER 1024

using namespace std;

/*
Struct containing important data for the server to work.
Namely the list of client sockets, that list's mutex,
the server's socket for new connections, and the message queue
*/
struct ChatroomsData {
    int socket_fd;
    pthread_mutex_t *clientListMutex;
};

void bind_socket(struct sockaddr_in *server_address, int socket_fd, long port);
void startChatrooms(int socket_fd);
void *newClientHandler(void *data);

int main(int argc, char *argv[]) {

    struct sockaddr_in server_address;
    int socket_fd;
    long port;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    port = strtol(argv[1], NULL, 0);

    if(socket_fd == -1) {
        perror("No se pudo crear el socket");
        exit(1);
    }

    bind_socket(&server_address, socket_fd, port);

    if(listen(socket_fd, 1) == -1) {
        perror("Union fallida");
        exit(1);
    }

    cout << "Socket: " << endl
         << socket_fd << endl;
    startChatrooms(socket_fd);

    close(socket_fd);

}

// Enlazar el socket
void bind_socket(struct sockaddr_in *server_address, int socket_fd, long port) {
    memset(server_address, 0, sizeof(*server_address));

    server_address -> sin_family = AF_INET;
    server_address -> sin_addr.s_addr = htonl(INADDR_ANY);
    server_address -> sin_port = htons(port);

    if(bind(socket_fd, (struct sockaddr *)server_address, sizeof(struct sockaddr_in)) == -1) {
        perror("Enlazado de los sockets fallido");
        exit(1);
    }
}

void startChatrooms(int socket_fd) {
    struct ChatroomsData data;
    data.socket_fd = socket_fd;
    data.clientListMutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(data.clientListMutex, NULL);

    //Start thread to handle new client connections
    pthread_t connectionThread;
    if((pthread_create(&connectionThread, NULL, newClientHandler, (void *)&data)) == 0) {
        cout << "\nNo se pudo conectar" << endl
             << stderr << endl;
    }

    pthread_join(connectionThread, NULL);


    pthread_mutex_destroy(data.clientListMutex);
    free(data.clientListMutex);
}

// Thread para manejar las nuevas conexiones de clientes.
// Agrega el client_fd a la lista y crea un hilo de manejador de cliente
void *newClientHandler(void *data) {
    ChatroomsData *chatroomsData = (ChatroomsData *) data;

    while(true) {

        int clientSocketFd = accept(chatroomsData -> socket_fd, NULL, NULL);

        if(clientSocketFd > 0) {
            fprintf(stderr, "Server accepted new client. Socket: %d\n", clientSocketFd);

            //Obtain lock on clients list and add new client in
            pthread_mutex_lock(chatroomsData -> clientListMutex);
            pthread_mutex_unlock(chatroomsData -> clientListMutex);
        }
    }
}
