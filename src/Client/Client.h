#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // Operaciones básicas con sockets
#include <arpa/inet.h> // inet_addr
#include <unistd.h> // close
#include <nlohmann/json.hpp> // Para la manipulación de JSON
#include <chrono>

class Client {
private:
    bool registered = false; // Indica si el usuario fue registrado exitosamente
    std::condition_variable cv; // Variable de condición para sincronizar
    int clientSocket;
    struct sockaddr_in serverAddress;
    bool connected;
    std::thread listenerThread;
    std::mutex socketMutex;

    void listenForMessages(); // Método para escuchar los mensajes del servidor

public:
    Client(std::string ip, int port);
    ~Client();

    void connectToServer();
    void sendMessage(const nlohmann::json& message);
    void run(); // Método principal que maneja la entrada del usuario
    void disconnect(); // Método para desconectar el cliente

    bool isConnected() const;
};

#endif // CLIENT_H
