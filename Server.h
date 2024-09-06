/*
    Header de la clase servidor
    Se definen atributos y comportamientos del servidor
*/
#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <thread> // Permite hilos
#include <vector> // Estructura dinámica para los clientes
#include <memory>
#include <sys/types.h>
#include <sys/socket.h> // Operaciones básicas con sockets
#include <netinet/in.h> // Estructura sockaddr_in
#include <arpa/inet.h>
#include <unistd.h> // Manejo de sockets
#include <cstring> // Para memset
#include <iostream> // Para std::cerr


class Server{
    private:
        int port;
        int addrLen;
        struct sockaddr_in address;
        std::unique_ptr<int> serverSocket;
        std::vector<std::thread> clients;

    /*
        Estos métodos son privados dado que el servidor 
        será el único que los utilice internamente.
    */
        void setUpSocket();
        void acceptClients();
        void handleClient(int clientSocket);
        //void sendMessage(int clientSocket, const std::string& message);

    public:
    //Constructor, destructor y metodo de inicialización
        Server(int port);

        ~Server(){
            this->port = 0;
        }

        void start();
};

#endif //SERVER_H