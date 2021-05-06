# so-chat

## Server:
    IP: 3.9.146.73
    PORT: 8000

## Ejecutar en server:
    g++ -o server server.cpp -lpthread
    ./server 8000

## Ejecutar el cliente:
    g++ -o client client.cpp
    g++ -std=c++17 -o client client.cpp
    g++-10 -o client client.cpp -lprotobuf
    ./client username 3.9.146.73 8000

## Para que es ayuda? debe responder el server o el cliente?

## Para compilar el proto
    protoc --cpp_out=./ ./petition.proto
