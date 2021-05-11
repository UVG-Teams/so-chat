/*
    Autores:
        Francisco Rosal
        Gian Luca Rivera
*/

#include <netdb.h>
#include <stdio.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


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
void get_my_ip(string *my_ip);


int main(int argc, char *argv[]) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    username = argv[1];
    host = gethostbyname(argv[2]);
    port = strtol(argv[3], NULL, 0);
    string my_ip;
    get_my_ip(&my_ip);

    cout << "MY IP: " << my_ip << endl;

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
        char server_buffer[MAX_CLIENT_BUFFER];
        char client_buffer[MAX_CLIENT_BUFFER];
        chat::ClientPetition client_petition;
        chat::ServerResponse server_response;
        string petition;
        string response;

        cout << "\n1. Registro de usuario" << endl
             << "2. Lista de usuarios conectados" << endl
             << "3. Cambiar estado" << endl
             << "4. Chat" << endl
             << "5. Info de usuario" << endl
             << "6. Ayuda" << endl
             << "7. Salir\n" << endl;
        cout << "Ingresa una opción" << endl;
        cin >> choice;

        client_petition.set_option(choice);

        switch(choice) {
            case 1:
                client_petition.mutable_registration() -> set_username(username);
                client_petition.mutable_registration() -> set_ip(my_ip);
                break;
            case 2:
                break;
            case 3: {
                int status_choice;
                cout << "\n1. Activo" << endl
                    << "2. Inactivo" << endl
                    << "3. Ocupado\n" << endl;
                cout << "Ingresa una opción" << endl;
                cin >> status_choice;

                string status;
                if (status_choice == 1) {
                    status = "Activo";
                } else if (status_choice == 2) {
                    status = "Inactivo";
                } else if (status_choice == 3) {
                    status = "Ocupado";
                } else {
                    status = "Activo";
                }

                client_petition.mutable_change() -> set_username(username);
                client_petition.mutable_change() -> set_status(status);
                break;
            }
            case 4:
                break;
            case 5: {
                string username_requested;
                cout << "Ingresa un nombre de usuario (username || everyone): " << endl;
                cin >> username_requested;
                client_petition.mutable_users() -> set_user(username_requested);
                break;
            }
            case 6:
                break;
            case 7:
                client_petition.SerializeToString(&petition);
                strcpy(client_buffer, petition.c_str());
                if(write(socket_fd, client_buffer, MAX_CLIENT_BUFFER - 1) == -1) {
                    cout << "La conexion fallo, vuelva a intentar" << endl;
                } else {
                    cout << "Adios :)" << endl;
                    close(socket_fd);
                    google::protobuf::ShutdownProtobufLibrary();
                    return 1;
                }
            default:
                break;
        }

        client_petition.SerializeToString(&petition);
        strcpy(client_buffer, petition.c_str());
        if(write(socket_fd, client_buffer, MAX_CLIENT_BUFFER - 1) == -1) {
            cout << "La conexion fallo, vuelva a intentar" << endl;
        }

        int len_read = read(socket_fd, &server_buffer, MAX_CLIENT_BUFFER - 1);
        server_buffer[len_read] = '\0';
        response = (string)server_buffer;
        server_response.ParseFromString(response);
        cout << "Server:\n"
             << server_response.option() << " " << endl
             << server_response.code() << endl;

        if (server_response.has_servermessage()) {
            cout << "\n" << server_response.servermessage() << endl;
        }

        if (server_response.has_connectedusers()) {
            chat::ConnectedUsersResponse connected_users = server_response.connectedusers();

            cout << "Connected Users: \n" << endl;
            for (int i = 0; i < connected_users.connectedusers_size(); i++) {
                chat::UserInfo user_info = connected_users.connectedusers(i);
                cout << user_info.username() << " " << user_info.ip() << " " << user_info.status() << "\n" << endl;
            }
        }

        if (server_response.has_userinforesponse()) {
            chat::UserInfo user_info = server_response.userinforesponse();

            cout << "User Info: \n" << endl;
            cout << user_info.username() << " " << user_info.ip() << " " << user_info.status() << "\n" << endl;
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
        cout << "No se ha podido conectar con el servidor" << endl;
        exit(1);
    }
}

void get_my_ip(string *my_ip) {
    // https://stackoverflow.com/questions/49335001/get-local-ip-address-in-c
    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;
    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if(sock < 0) {
        cout << "Socket error" << endl;
    }

    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(google_dns_server);
    serv.sin_port = htons(dns_port);

    int err = connect(sock, (const struct sockaddr*)&serv, sizeof(serv));
    if (err < 0) {
        cout << "Error" << endl;
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*)&name, &namelen);

    char buffer[80];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, 80);
    if(p != NULL) {
        *my_ip = (string)buffer;
    } else {
        cout << "Error" << endl;
    }

    close(sock);
    return;
}