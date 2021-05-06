/*
    Autores:
        Francisco Rosal
        Gian Luca Rivera
*/

#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sys/socket.h>

// ProtoBuff
#include "petition.pb.h"

#define MAX_CLIENT_BUFFER 2048
using namespace std;

int choice;
char *username;
struct sockaddr_in server_address;
struct hostent *host;
long port;
static int socket_fd;


void connect_to_server(int socket_fd, struct sockaddr_in *server_address, struct hostent *host, long port);


int main(int argc, char *argv[]) {
    // GOOGLE_PROTOBUF_VERIFY_VERSION;
    username = argv[1];
    host = gethostbyname(argv[2]);
    port = strtol(argv[3], NULL, 0);
    char server_buffer[MAX_CLIENT_BUFFER];

    if(host == NULL) {
        cout << "\nNo se pudo obtener el host" << stderr << endl;
        exit(1);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_fd == -1) {
        cout << "\nNo se pudo crear el socket" << stderr << endl;
        exit(1);
    }

    cout << "Socket: " << socket_fd << endl;

    connect_to_server(socket_fd, &server_address, host, port);

    do {
        // chat::ClientPetition client_petition;

        cout << "\n1. Chat" << endl
             << "2. Cambiar estado" << endl
             << "3. Info de usuario/s conectado/s" << endl
             << "4. Ayuda" << endl
             << "5. Salir\n" << endl;
        cout << "Ingresa una opción" << endl;
        cin >> choice;

        // client_petition.set_option(choice);

        switch(choice) {
            case 1:
                cout << "Opcion 1\n" << endl;
                break;
            case 2:
                cout << "Opcion 3\n" << endl;
                break;
            case 3:
                if(write(socket_fd, "/users", MAX_CLIENT_BUFFER - 1) == -1) {
                    cout << "La conexion fallo, vuelva a intentar" << endl;
                }
                break;
            case 4:
                cout << "Opcion 5\n" << endl;
                break;
            case 5:
                if(write(socket_fd, "/exit", MAX_CLIENT_BUFFER - 1) == -1) {
                    cout << "La conexion fallo, vuelva a intentar" << endl;
                } else {
                    cout << "Adios :)" << endl;
                    close(socket_fd);
                    return 1;
                }
        }

        int len_read = read(socket_fd, server_buffer, MAX_CLIENT_BUFFER - 1);
        server_buffer[len_read] = '\0';
        cout << "Server:\n" << server_buffer << endl;

    } while(choice != 7);
}


void connect_to_server(int socket_fd, struct sockaddr_in *server_address, struct hostent *host, long port) {
    // hostent estructura que definine un host de internet contiene info del nombre del server
    // sockaddr_in estructura que administra direcciones de internet
    // socket_fd es un socket file descriptor

    // Se asigna el valor 0 a los valores de server_address
    memset(server_address, 0, sizeof(server_address));

    server_address -> sin_family = AF_INET;
    server_address -> sin_addr = *((struct in_addr *)host -> h_addr_list[0]);
    server_address -> sin_port = htons(port);

    // Conectar el socket
    if(connect(socket_fd, (struct sockaddr *) server_address, sizeof(struct sockaddr)) < 0) {
        cout << "No se ha podido conectar con el servidor" << endl;
        exit(1);
    }
}
