/*
    Autores:
        Francisco Rosal
        Gian Luca Rivera
*/

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/socket.h>


using namespace std;

int choice;
char *username;
struct sockaddr_in server_address;
struct hostent *host;
long port;
static int socket_fd;

void connect_to_server(int socket_fd, struct sockaddr_in *server_address, struct hostent *host, long port);

int main(int argc, char *argv[]) {

    username = argv[1];
    host = gethostbyname(argv[2]);
    port = strtol(argv[3], NULL, 0);

    if(host == NULL) {
        cout << "\nNo se pudo obtener el host" << endl
             << stderr << endl;
        exit(1);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "Socket: " << endl
         << socket_fd << endl;

    if(socket_fd == -1) {
        cout << "\nNo se pudo crear el socket" << endl
             << stderr << endl;
        exit(1);
    }

    connect_to_server(socket_fd, &server_address, host, port);

    do {
        cout << "\n1. Chat con todos los usuarios" << endl
             << "2. Chat privado" << endl
             << "3. Cambiar estado" << endl
             << "4. Usuarios conectados" << endl
             << "5. Información de un usuario" << endl
             << "6. Ayuda" << endl
             << "7. Salir\n" << endl;
        cout << "Ingresa una opción" << endl;
        cin >> choice;

        switch(choice) {
            case 1:
                cout << "Opcion 1\n" << endl;
                break;
            case 2:
                cout << "Opcion 2\n" << endl;
                break;
            case 3:
                cout << "Opcion 3\n" << endl;
                break;
            case 4:
                cout << "Opcion 4\n" << endl;
                break;
            case 5:
                cout << "Opcion 5\n" << endl;
                break;
            case 6:
                cout << "Opcion 6\n" << endl;
                break;
            case 7:
                cout << "Adios :)" << endl;
                return 1;
        }
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
        perror("No se ha podido conectar con el servidor");
        exit(1);
    }
}
