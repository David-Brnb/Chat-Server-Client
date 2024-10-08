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
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "User.h"
#include "Sala.h"


class Server{
    private:
        int port;
        int addrLen;
        struct sockaddr_in address;
        std::unique_ptr<int> serverSocket;
        std::vector<std::thread> clients;

        std::unordered_map<int, std::shared_ptr<User> > clientSocketUser;
        std::unordered_map<std::string, int> clientUserSocket;
        std::unordered_set<std::string> rooms;
        std::unordered_map<std::string, Sala> nameRoom;
        

        //Nos sirve para manejar los clientes en los hilos. 
        std::mutex clientsMutex;

    /*
        Estos métodos son privados dado que el servidor 
        será el único que los utilice internamente.
    */  

        void acceptClients();
        void handleClient(int clientSocket);
        void registerUsername();
        bool registerUser(int clientSocket, std::string username, std::string status);
        void removeUserInformation(int clientSocket);
        void handleInvalidRequest(int clientSocket);
        void sendMessage(const nlohmann::json& message, int destinitySocket);

    public:
    //Constructor, destructor y metodo de inicialización
        Server(int port);

        ~Server(){
            if(serverSocket && *serverSocket>=0){
                close(*serverSocket);
            }

            this->port = 0;
        }

        void start();
};

#endif //SERVER_H