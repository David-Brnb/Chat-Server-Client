#include <iostream> 
#include "Server.h" 

// using namespace std; 

Server::Server(int port){
    serverSocket = std::make_unique<int>(socket(AF_INET, SOCK_STREAM, 0));
    
    if(*serverSocket < 0){
        std::cerr << "Error al crear el socket\n" << std::endl;
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
        std::cerr << "Error al vincular el socket\n" << std::endl;
        exit(EXIT_FAILURE);
    }

    int listener = listen(*serverSocket, 5);

    if(listener < 0){
        close(*serverSocket);
        std::cerr << "Error al escuchar en el socket\n" << std::endl;
        exit(EXIT_FAILURE);
    }
 
}

void Server::start() {
    std::cout << "Servidor iniciado, esperando conexiones..." << std::endl;
    acceptClients();  // Inicia la aceptación de clientes
}

bool Server::registerUser(int clientSocket, std::string username, std::string status) {
    // Usamos mutex para tener seguridad en el manejo de los hilos
    std::lock_guard<std::mutex> lock(clientsMutex);

    // Verifica si el nombre de usuario ya existe
    if (clientNames.find(username) != clientNames.end()) {
        std::string message = "Error: El nombre de usuario ya está en uso.\n";
        send(clientSocket, message.c_str(), message.size(), 0);
        return false;
    }

    // Crear un nuevo usuario y agregarlo al mapa
    User newUser(username, status, clientSocket, 0);  // Sala inicial 0

    clientSocketUser[clientSocket] = newUser;
    clientNames.insert(username);

    return true;
}

void Server::acceptClients(){
    char buffer[1024];
    while(true){
        int clientSocket = accept(*serverSocket, (struct sockaddr*)&address, (socklen_t *)&addrLen);

        if(clientSocket < 0){
            std::cerr << "Error al aceptar cliente\n" << std::endl;
            continue;
        }

        std::cout << "Cliente Conectado!" << std::endl;


        

        char bufferName[1024];
        int nameBytesReceived = recv(clientSocket, bufferName, sizeof(bufferName) - 1, 0);
        std::string username;

        if (nameBytesReceived > 0) {
            bufferName[nameBytesReceived] = '\0';
            username = bufferName;

            if(registerUser(clientSocket, username, "Activo")){
                // Si el nombre es válido, se añade el cliente a la lista
                clients.emplace_back(&Server::handleClient, this, clientSocket);

                std::string promptName = "Usuario registrado con exito!\n";
                send(clientSocket, promptName.c_str(), promptName.size(), 0);

            } else {
                close(clientSocket);
            }
            
        } else {
            // Cierra la conexión si no se recibe el nombre de usuario
            close(clientSocket);
        }

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
                clientNames.erase(clientSocketUser[clientSocket].getNombre());
                clientSocketUser.erase(clientSocket);
            }
            break;
        }

        //Procesar y responder
        buffer[bites] = '\0'; //
        std::string mensaje = buffer;

        //Imprimos lo que dice dentro de nuestro server
        std::cout << clientSocketUser[clientSocket].getNombre() << ": " << mensaje << std::endl;

        // Responder al cliente
        std::string response = clientSocketUser[clientSocket].getNombre() + ": " + mensaje + "\n";

        for(auto &socket: clientSocketUser){
            if(socket.first != clientSocket){
                send(socket.first, response.c_str(), response.size(), 0);
            }
        }
    }

}

