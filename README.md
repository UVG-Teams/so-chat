# so-chat

## Server:
    IP: 3.9.13.68
    PORT: 8000

## Ejecutar en server:
    g++ -o server server.cpp -lpthread
    clang++ -std=c++11 -stdlib=libstdc++ -o server server.cpp petition.pb.cc -pthread -I/usr/local/include -L/usr/local/lib -lprotobuf -lpthread
    ./server 8000

## Ejecutar el cliente:
    g++ -o client client.cpp
    g++ -std=c++17 -o client client.cpp
    g++-10 -o client client.cpp -lprotobuf
    g++-10 -o client client.cpp petition.pb.cc `pkg-config --cflags --libs protobuf`
    clang++ -std=c++11 -stdlib=libc++ -o client client.cpp petition.pb.cc -pthread -I/usr/local/include -L/usr/local/lib -lprotobuf -lpthread
    ./client username 3.9.13.68 8000

## Para compilar el proto
    protoc --cpp_out=./ ./petition.proto
