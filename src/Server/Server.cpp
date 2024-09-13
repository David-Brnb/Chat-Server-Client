#include <iostream> 
#include "Server.h" 
#include "Message.h"

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

    nameRoom[""] = Sala();
 
}

void Server::start() {
    std::cout << "Servidor iniciado, esperando conexiones..." << std::endl;
    acceptClients();  // Inicia la aceptación de clientes
}

void Server::sendMessage(const nlohmann::json& message, int destinitySocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    std::string msgString = message.dump()+'\0';
    send(destinitySocket, msgString.c_str(), msgString.size(), 0);
}

bool Server::registerUser(int clientSocket, std::string username, std::string status) {
    // Usamos mutex para tener seguridad en el manejo de los hilos
    std::lock_guard<std::mutex> lock(clientsMutex);

    // Verifica si el nombre de usuario ya existe
    if (clientUserSocket.find(username) != clientUserSocket.end() || clientSocketUser[clientSocket]) {
        return false;
    }

    // Crear un nuevo usuario y agregarlo al mapa
    std::shared_ptr<User> newUser = std::make_shared<User>(username, status, clientSocket);  // Sala inicial 0

    clientSocketUser[clientSocket] = newUser;
    clientUserSocket[username] = clientSocket;
    nameRoom[""].agregarUsuario(newUser);

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

        // Si el nombre es válido, se añade el cliente a la lista
        clients.emplace_back(&Server::handleClient, this, clientSocket);

        std::cout << "Cliente Conectado!" << std::endl;

    }
}

void Server::removeUserInformation(int clientSocket) {
    std::string username = clientSocketUser[clientSocket]->getNombre();
    std::vector<std::string> deletedRooms;

    // Iterar por todas las salas y eliminar al usuario
    for (auto& sala : nameRoom) {
        std::string roomname = sala.first;
        Sala& currentRoom = sala.second;

        currentRoom.eliminarUsuario(username);
        if (currentRoom.cantidadDeUsuarios() <= 0) {
            deletedRooms.push_back(roomname); // Agregar a la lista de salas a eliminar
        }
    }

    // Eliminar las salas donde ya no hay usuarios
    for (const auto& roomname : deletedRooms) {
        rooms.erase(roomname);
        nameRoom.erase(roomname);
    }

    // Eliminar al usuario del mapa global y de la sala por defecto
    std::lock_guard<std::mutex> lock(clientsMutex);
    nameRoom[""].eliminarUsuario(username);
    clientUserSocket.erase(username);
    clientSocketUser.erase(clientSocket);
}

void Server::handleInvalidRequest(int clientSocket) {
    sendMessage(Message::noIdentifiedRequest(), clientSocket);

    // Imprimir mensaje en el servidor
    std::cout << "Enviando mensaje de INVALID a cliente " << clientSocket << " y cerrando la conexión." << std::endl;

    if(clientSocketUser[clientSocket]) removeUserInformation(clientSocket);

    close(clientSocket);
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

            if(clientSocketUser[clientSocket]) removeUserInformation(clientSocket);
            break;
        }

        //Procesar y responder
        buffer[bites] = '\0'; //
        std::string mensaje = buffer;

        // Convertir el string recibido en un objeto JSON
        nlohmann::json jsonMessage;

        try {
            jsonMessage = nlohmann::json::parse(buffer);

        } catch (nlohmann::json::parse_error& e) {
            /*
            * revisar el cierre bien
            */
            std::cerr << "Error al parsear JSON: " << e.what() << std::endl;
            std::cout << "Cliente desconectado." << std::endl;
            close(clientSocket);

            if(clientSocketUser[clientSocket]) removeUserInformation(clientSocket);
            break;
        }

        // Cadena auxiliar para respuestas del servidor
        std::string requestAnswer = "";
        std::string type = jsonMessage["type"];

        /* 
        Cadena de condicionales donde se realiza el procesamiento de requests por parte del server
        */
        if(type == "IDENTIFY"){
            if (!jsonMessage.contains("username") ) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string username = jsonMessage["username"], promptName;

            if(registerUser(clientSocket, username, "Active")){
                sendMessage(Message::createIdentifyResponse("SUCCESS", username), clientSocket);

                for(auto &socket: clientSocketUser){
                    if(socket.first != clientSocket){
                        sendMessage(Message::createNewUserMessage(username), socket.first);
                    }
                }

            } else {
                std::cerr << "El usuario '" << username << "' ya existe. Respondiendo al cliente." << std::endl;
                sendMessage(Message::createIdentifyResponse("USER_ALREADY_EXISTS", username), clientSocket);

            }


        } else if(clientSocketUser.find(clientSocket) == clientSocketUser.end()){
            std::cout << "Cliente desconectado." << std::endl;
            close(clientSocket);
            break; 

        } else if(type == "STATUS"){
            if (!jsonMessage.contains("status") ) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::shared_ptr<User> usr = clientSocketUser[clientSocket];

            bool diferentStatus = usr->getEstuatus() != jsonMessage["status"];
            bool validStatus = jsonMessage["status"] == "ACTIVE" || jsonMessage["status"] == "AWAY" || jsonMessage["status"] == "BUSY";

            if(validStatus && diferentStatus){
                usr->setEstatus(jsonMessage["status"]);
                sendMessage(Message::createNewStatusMessage(usr->getNombre(), usr->getEstuatus()), clientSocket);
            }
            

        } else if(type == "USERS") {
            std::map<std::string, std::string> users = nameRoom[""].obtenerEstatusUsuarios();
            
            sendMessage(Message::createUserListMessage(users), clientSocket);

        } else if(type == "TEXT"){
            if (!jsonMessage.contains("username") || !jsonMessage.contains("text")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string username = jsonMessage["username"];
            std::string message = jsonMessage["text"];

            //Si el usuario no existe
            if(clientUserSocket.find(username) == clientUserSocket.end()){
                sendMessage(Message::createNoSuchUserResponse(username), clientSocket);

            } else {
                sendMessage(Message::createTextFromMessage(clientSocketUser[clientSocket]->getNombre(), message), clientUserSocket[username]);

            }


        } else if(type == "PUBLIC_TEXT"){
            if (!jsonMessage.contains("text")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string username = clientSocketUser[clientSocket]->getNombre();
            std::string message = jsonMessage["text"];

            for(auto &socket: clientSocketUser){
                if(socket.first != clientSocket){
                    sendMessage(Message::createPublicTextFromMessage(username, message), socket.first);
                }
            }


        } else if(type == "NEW_ROOM"){
            if (!jsonMessage.contains("roomname")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string roomname = jsonMessage["roomname"];
            std::shared_ptr<User> usr = clientSocketUser[clientSocket];

            if(rooms.count(roomname) <= 0){
                rooms.insert(roomname);

                nameRoom[roomname].agregarUsuario(usr);

                sendMessage(Message::createNewRoomResponse(roomname, "SUCCESS"), clientSocket);

            } else {
                sendMessage(Message::createNewRoomResponse(roomname, "ROOM_ALREADY_EXISTS"), clientSocket);
                
            }



        } else if(type == "INVITE"){
            if (!jsonMessage.contains("roomname") || !jsonMessage.contains("usernames")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string roomname = jsonMessage["roomname"];
            std::vector<std::string> usernames = jsonMessage["usernames"];

            if(rooms.count(roomname) <= 0 || !nameRoom[roomname].contieneUsuario(clientSocketUser[clientSocket]->getNombre())){
                sendMessage(Message::createInviteResponse(roomname, "NO_SUCH_ROOM"), clientSocket);

            } else {
                bool success = true; 

                for(auto user: usernames){
                    if(clientUserSocket[user] == 0){
                        sendMessage(Message::createInviteResponse(user, "NO_SUCH_USER"), clientSocket);
                        success = false; 
                        break; 
                    }
                }

                if(success){
                    std::shared_ptr<User> roomCreator = clientSocketUser[clientSocket];

                    for(auto user: usernames){
                        int userSocket = clientUserSocket[user];
                        std::shared_ptr<User> userInvited = clientSocketUser[userSocket];

                        if(!nameRoom[roomname].contieneUsuario(clientSocketUser[userSocket]->getNombre())){
                            userInvited->insertaInvitacionSala(roomname);
                            sendMessage(Message::createInvitationMessage(roomCreator->getNombre(), roomname), userSocket);

                        }
                    }
                } 
            }

        } else if(type == "JOIN_ROOM"){
            if (!jsonMessage.contains("roomname")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string roomname = jsonMessage["roomname"];
            std::shared_ptr<User> user = clientSocketUser[clientSocket];

            if(rooms.count(roomname)>0 && user->contieneInvitacion(roomname)){
                nameRoom[roomname].agregarUsuario(user);

                sendMessage(Message::createJoinRoomResponse(roomname, "SUCCESS"), clientSocket);
                
                for(auto elem: nameRoom[roomname].verUsuarios()){
                    sendMessage(Message::createJoinedRoomMessage(roomname, user->getNombre()), clientUserSocket[elem.first]);

                }

            } else if(rooms.count(roomname) <= 0){
                sendMessage(Message::createJoinRoomResponse(roomname, "NO_SUCH_ROOM"), clientSocket);

            } else if(!user->contieneInvitacion(roomname)){
                sendMessage(Message::createJoinRoomResponse(roomname, "NOT_INVITED"), clientSocket);

            }

        } else if(type == "ROOM_USERS"){
            if (!jsonMessage.contains("roomname")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string roomname = jsonMessage["roomname"];
            std::string username = clientSocketUser[clientSocket]->getNombre();

            if(rooms.count(roomname) > 0 && nameRoom[roomname].contieneUsuario(username)){
                sendMessage(Message::createRoomUserListMessage(roomname, nameRoom[roomname].obtenerEstatusUsuarios()), clientSocket);

            } else if(rooms.count(roomname) <= 0){
                sendMessage(Message::createRoomUsersResponseNo("NO_SUCH_ROOM", roomname), clientSocket);
                
            } else if(!nameRoom[roomname].contieneUsuario(username)){
                sendMessage(Message::createRoomUsersResponseNo("NOT_JOINED", roomname), clientSocket);

            }
            

        } else if(type == "ROOM_TEXT"){
            if (!jsonMessage.contains("roomname") || !jsonMessage.contains("text")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string roomname = jsonMessage["roomname"];
            std::string text = jsonMessage["text"];
            std::string username = clientSocketUser[clientSocket]->getNombre();
            Sala currentRoom = nameRoom[roomname];

            if(rooms.count(roomname) > 0 && currentRoom.contieneUsuario(username)){

                for(auto usr: currentRoom.verUsuarios()){
                    int currentSocket = clientUserSocket[usr.first];

                    sendMessage(Message::createRoomText(roomname, username, text), currentSocket);

                }

            } else if(rooms.count(roomname) <= 0){
                sendMessage(Message::createRoomTextResponse("NO_SUCH_ROOM", roomname), clientSocket);
                
            } else if(!nameRoom[roomname].contieneUsuario(username)){
                sendMessage(Message::createRoomTextResponse("NOT_JOINED", roomname), clientSocket);

            }


        } else if(type == "LEAVE_ROOM"){
            if (!jsonMessage.contains("roomname")) {
                handleInvalidRequest(clientSocket);
                break;
            }

            std::string roomname = jsonMessage["roomname"];
            std::string username = clientSocketUser[clientSocket]->getNombre();
            Sala currentRoom = nameRoom[roomname];

            if(rooms.count(roomname) > 0 && currentRoom.contieneUsuario(username)){

                {
                    //BORRAR DE LAS SALA
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    currentRoom.eliminarUsuario(username);
                    if(currentRoom.cantidadDeUsuarios() <= 0){
                        rooms.erase(roomname);
                        nameRoom.erase(roomname);
                    }
                }

                for(auto usr: currentRoom.verUsuarios()){
                    int currentSocket = clientUserSocket[usr.first];
                    sendMessage(Message::createRoomText(roomname, username), currentSocket);

                }

            } else if(rooms.count(roomname) <= 0){
                sendMessage(Message::leaveRoomResponse("NO_SUCH_ROOM", roomname), clientSocket);
                
            } else if(!nameRoom[roomname].contieneUsuario(username)){
                sendMessage(Message::leaveRoomResponse("NOT_JOINED", roomname), clientSocket);

            }



        } else if(type == "DISCONNECT"){

            for(auto &socket: clientSocketUser){
                if(socket.first != clientSocket){
                    sendMessage(Message::disconnectMessage(clientSocketUser[clientSocket]->getNombre()), socket.first);
                }
            }

            std::cout << "Cliente desconectado." << std::endl;
            removeUserInformation(clientSocket);
            close(clientSocket);
            break;


        } else {
            sendMessage(Message::noIdentifiedRequest(), clientSocket);
            
            if(clientSocketUser[clientSocket]) removeUserInformation(clientSocket);
            close(clientSocket);
            break;

        }

        //Imprimos lo que dice dentro de nuestro server
        if(clientSocketUser[clientSocket]) std::cout << clientSocketUser[clientSocket]->getNombre() << ": " << mensaje << std::endl;
        else std::cout << "Anonimo" << ": " << mensaje << std::endl;

    }

}

