/*
    Autores:
        Francisco Rosal
        Gian Luca Rivera
*/

#include <vector>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>

// ProtoBuff
#include "petition.pb.h"


#define MAX_CLIENT_BUFFER 2048
using namespace std;


struct Client {
    int socket_fd;
    string username;
    string ip;
};

struct ChatroomsData {
    int socket_fd;
    vector<int> client_sockets;
    fd_set read_fds;
    pthread_mutex_t *client_list_mutex;

    ChatroomsData(int sfd, pthread_mutex_t *clm) {
        socket_fd = sfd;
        client_list_mutex = clm;
    }
};

struct CurrentClientData {
    int socket_fd;
    ChatroomsData *chatrooms_data;

    CurrentClientData(int csfd, ChatroomsData *data) {
        socket_fd = csfd;
        chatrooms_data = data;
    }
};


void bind_socket(struct sockaddr_in *server_address, int socket_fd, long port);
void startChatrooms(int socket_fd);
void *new_clients_handler(void *data);
void *client_listener(void *client_data);
void disconnect_client(ChatroomsData *chatrooms_data, int current_client_socket_fd);


int main(int argc, char *argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    struct sockaddr_in server_address;
    int socket_fd;
    long port;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    port = strtol(argv[1], NULL, 0);

    if(socket_fd == -1) {
        cout << "No se pudo crear el socket" << endl;
        exit(1);
    }

    bind_socket(&server_address, socket_fd, port);

    if(listen(socket_fd, 1) == -1) {
        cout << "Union fallida" << endl;
        exit(1);
    }

    startChatrooms(socket_fd);
    close(socket_fd);
}

// Enlazar el socket
void bind_socket(struct sockaddr_in *server_address, int socket_fd, long port) {
    memset(server_address, 0, sizeof(*server_address));

    server_address -> sin_family = AF_INET;
    server_address -> sin_addr.s_addr = htonl(INADDR_ANY);
    server_address -> sin_port = htons(port);

    if(::bind(socket_fd, (struct sockaddr *)server_address, sizeof(struct sockaddr_in)) == -1) {
        cout <<  "Enlazado de los sockets fallido" << endl;
        exit(1);
    }
}

void startChatrooms(int socket_fd) {
    ChatroomsData data(socket_fd, (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t)));
    pthread_mutex_init(data.client_list_mutex, NULL);

    // Thread que maneja las nuevas conexiones
    pthread_t connectionThread;

    if((pthread_create(&connectionThread, NULL, new_clients_handler, (void *)&data)) == 0) {
        cout << "Server listo" << endl;
    }

    FD_ZERO(&(data.read_fds));
    FD_SET(socket_fd, &(data.read_fds));

    pthread_join(connectionThread, NULL);

    pthread_mutex_destroy(data.client_list_mutex);
    free(data.client_list_mutex);
}

// Agrega el client_fd a la lista y crea un hilo de manejador de cliente
void *new_clients_handler(void *data) {
    ChatroomsData *chatrooms_data = (ChatroomsData *) data;

    while(true) {
        int new_client_socket_fd = accept(chatrooms_data -> socket_fd, NULL, NULL);

        if(new_client_socket_fd > 0) {
            cout << "\nSe ha conectado un nuevo cliente: " << new_client_socket_fd << endl;

            // Obtener lock en la lista de clientes
            pthread_mutex_lock(chatrooms_data -> client_list_mutex);

            bool is_defined_already = false;

            // Verificar que no exista el file descriptor en la lista de file descriptors leidos
            for(int i = 0; i < chatrooms_data -> client_sockets.size(); i++) {
                if(FD_ISSET(chatrooms_data -> client_sockets[i], &(chatrooms_data -> read_fds))) {
                    is_defined_already = true;
                    break;
                }
            }

            // Si no estaba definido se agrega
            if (!is_defined_already) {
                chatrooms_data -> client_sockets.push_back(new_client_socket_fd);
            }

            // Agregar un nuevo socket fd a la lista de fds leidos
            FD_SET(new_client_socket_fd, &(chatrooms_data -> read_fds));

            // Creacion de un hilo para las peticiones del cliente
            CurrentClientData client_data(new_client_socket_fd, chatrooms_data);
            pthread_t client_thread;

            // cout << "Inicio: " << chatrooms_data -> client_sockets.size() << endl;
            // for (vector<int>::iterator i = chatrooms_data -> client_sockets.begin(); i != chatrooms_data -> client_sockets.end(); i++) {
            //     cout << "Puntero" << *i << endl;
            //     cout << "&" << &i << endl;
            //     cout << "valor?" << chatrooms_data -> client_sockets[*i] << endl;
            // }

            if((pthread_create(&client_thread, NULL, client_listener, (void *)&client_data)) == 0) {
                cout << "Escuchando a cliente por socket: " << new_client_socket_fd << endl;
            } else {
                close(new_client_socket_fd);
            }

            pthread_mutex_unlock(chatrooms_data -> client_list_mutex);
        }
    }
}

void *client_listener(void *client_data) {
    CurrentClientData *current_client_data = (CurrentClientData *) client_data;
    ChatroomsData *chatrooms_data = (ChatroomsData *) current_client_data -> chatrooms_data;
    int current_client_socket_fd = current_client_data -> socket_fd;

    // queue *q = data->queue;
    char client_buffer[MAX_CLIENT_BUFFER];
    chat::ClientPetition client_petition;
    string petition;

    while(true) {
        int len_read = read(current_client_socket_fd, &petition, MAX_CLIENT_BUFFER - 1);
        // client_buffer[len_read] = '\0';

        client_petition.ParseFromString(petition);

        // cout << "Cliente1: " << client_buffer << endl;
        cout << "Cliente: " << client_petition.option() << endl;
        // cout << "Cliente2: " << petition << endl;
        // cout << "Cliente3: " << &petition << endl;

        if(strcmp(client_buffer, "/exit") == 0) {
            cout << "El cliente se ha desconectado, socket: " << current_client_socket_fd << endl;
            disconnect_client(chatrooms_data, current_client_socket_fd);
            return NULL;
        } else if (strcmp(client_buffer, "/users") == 0) {
            write(current_client_socket_fd, "Usuarios: ", MAX_CLIENT_BUFFER - 1);
        }

        // If the client sent /exit\n, remove them from the client list and close their socket
        // if(strcmp(msgBuffer, "/exit\n") == 0) {
        //     fprintf(stderr, "Client on socket %d has disconnected.\n", current_client_socket_fd);
        //     removeClient(chatrooms_data, current_client_socket_fd);
        //     return NULL;
        // } else {
        //     Wait for queue to not be full before pushing message
        //     while(q->full) {
        //         pthread_cond_wait(q->notFull, q->mutex);
        //     }

        //     // Obtain lock, push message to queue, unlock, set condition variable
        //     pthread_mutex_lock(q->mutex);
        //     fprintf(stderr, "Pushing message to queue: %s\n", msgBuffer);
        //     queuePush(q, msgBuffer);
        //     pthread_mutex_unlock(q->mutex);
        //     pthread_cond_signal(q->notEmpty);
        // }
    }
}

// Removes the socket from the list of active client sockets and closes it
void disconnect_client(ChatroomsData *chatrooms_data, int current_client_socket_fd) {
    pthread_mutex_lock(chatrooms_data -> client_list_mutex);

    // for(vector<int>::iterator i = chatrooms_data -> client_sockets.begin(); i != chatrooms_data -> client_sockets.end(); i++) {
    //     cout << "Este va a morir: " << current_client_socket_fd << endl;
    //     cout << "Pos actual: " << chatrooms_data -> client_sockets[*i] << endl;
    //     if(chatrooms_data -> client_sockets[i] == current_client_socket_fd) {
    //         chatrooms_data -> client_sockets.erase(i);
    //         close(current_client_socket_fd);
    //         cout << "Este muere: " << current_client_socket_fd << endl;
    //         break;
    //     }
    // }

    // cout << "Este va a morir: " << current_client_socket_fd << endl;
    vector<int>::iterator it = chatrooms_data -> client_sockets.begin();

    while (it != chatrooms_data -> client_sockets.end()) {
        // cout << "Pos actual: " << *it << endl;
        if (*it == current_client_socket_fd) {
            // cout << "Este muere: " << current_client_socket_fd << endl;
            it = chatrooms_data -> client_sockets.erase(it);
        } else {
            ++it;
        }
    }

    // cout << "Fin: " << chatrooms_data -> client_sockets.size() << endl;
    // for (vector<int>::iterator i = chatrooms_data -> client_sockets.begin(); i != chatrooms_data -> client_sockets.end(); i++) {
    //     cout << "Puntero" << *i << endl;
    //     cout << "&" << &i << endl;
    //     cout << "valor?" << chatrooms_data -> client_sockets[*i] << endl;
    // }

    pthread_mutex_unlock(chatrooms_data -> client_list_mutex);
}
