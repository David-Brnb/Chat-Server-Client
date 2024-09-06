#include <iostream> 
#include "Server.h" 

// using namespace std; 

Server::Server(int port){
    serverSocket = std::make_unique<int>(socket(AF_INET, SOCK_STREAM, 0));
    
    if(*serverSocket < 0){
        std::cerr << "Error al crear el socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    this->port = port;

    // Se inicializa la estructura
    address.sin_port = htons(port); //cambiamos little endian a big endian
    address.sin_addr.s_addr = INADDR_ANY; // Asigna la IP a la dirección
    address.sin_family = AF_INET;  // asignamos el socket a IPv4
    memset(address.sin_zero, '\0', sizeof(address.sin_zero)); // iniciamos a address.sin_zero a 0, evitando futuros problemas
    addrLen = sizeof(address);

    //vincular el socket
    int conexion = bind(*serverSocket, (struct sockaddr*)&address, sizeof(address));
    
    if(conexion < 0){
        close(*serverSocket);
        std::cerr << "Error al vincular el socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    int listener = listen(*serverSocket, 5);

    if(listener < 0){
        close(*serverSocket);
        std::cerr << "Error al escuchar en el socket" << std::endl;
        exit(EXIT_FAILURE);
    }
 
}

void Server::start() {
    std::cout << "Servidor iniciado, esperando conexiones..." << std::endl;
    acceptClients();  // Inicia la aceptación de clientes
}

void Server::acceptClients(){
    char buffer[1024];
    while(true){
        int clientSocket = accept(*serverSocket, (struct sockaddr*)&address, (socklen_t *)&addrLen);

        if(clientSocket < 0){
            std::cerr << "Error al aceptar cliente" << std::endl;
            continue;
        }

        std::cout << "Cliente Conectado!" << std::endl;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets[clientSocket] = "";

        }

        //Crearmos un nuevo hilo para manejar al cliente
        clients.emplace_back(&Server::handleClient, this, clientSocket);
    }
}

void Server::handleClient(int clientSocket){
    char buffer[1024];
    
    while(true){
        // Leemos del socket del cliente
        int bites = recv(clientSocket, buffer, sizeof(buffer), 0); //0

        // si existe error entonces avisamos y cerramos el socket del cliente
        if(bites <= 0){
            std::cout << "Cliente desconectado." << std::endl;
            close(clientSocket);

            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clientSockets.erase(clientSocket);
            }
            break;
        }

        //Procesar y responder
        buffer[bites] = '\0'; //
        std::string mensaje = buffer;

        //Imprimos lo que dice dentro de nuestro server
        std::cout << "Cliente " << clientSocket << " dice: " << mensaje << std::endl;

        // Responder al cliente
        std::string response = "Mensaje recibido del cliente " + std::to_string(clientSocket) + ": " + mensaje + "\n";

        for(auto &socket: clientSockets){
            if(socket.first != clientSocket){
                send(socket.first, response.c_str(), response.size(), 0);
            }
        }
    }

}

