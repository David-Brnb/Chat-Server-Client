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

bool Server::registerUser(int clientSocket, std::string username, std::string status) {
    // Usamos mutex para tener seguridad en el manejo de los hilos
    std::lock_guard<std::mutex> lock(clientsMutex);

    // Verifica si el nombre de usuario ya existe
    if (clientUserSocket.find(username) != clientUserSocket.end()) {
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

void Server::removeUserFromRooms(int clientSocket) {
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

void Server::handleClient(int clientSocket){
    char buffer[1024];
    
    while(true){
        // Leemos del socket del cliente
        int bites = recv(clientSocket, buffer, sizeof(buffer), 0); //0

        // si existe error entonces avisamos y cerramos el socket del cliente
        if(bites <= 0){
            std::cout << "Cliente desconectado." << std::endl;
            close(clientSocket);

            removeUserFromRooms(clientSocket);
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

            removeUserFromRooms(clientSocket);
            break;
        }

        // Cadena auxiliar para respuestas del servidor
        std::string requestAnswer = "";
        std::string type = jsonMessage["type"];

        /* 
        Cadena de condicionales donde se realiza el procesamiento de requests por parte del server
        */
        if(type == "IDENTIFY"){
            std::string username = jsonMessage["username"], promptName;

            if(registerUser(clientSocket, username, "Active")){

                promptName = Message::createIdentifyResponse("SUCCESS", username).dump()+'\0';
                send(clientSocket, promptName.c_str(), promptName.size(), 0);

                // Mensaje de nuevo usuario
                std::string response = Message::createNewUserMessage(username).dump()+'\0';

                for(auto &socket: clientSocketUser){
                    if(socket.first != clientSocket){
                        send(socket.first, response.c_str(), response.size(), 0);
                    }
                }

            } else {
                std::cerr << "El usuario '" << username << "' ya existe. Respondiendo al cliente." << std::endl;
                promptName = Message::createIdentifyResponse("USER_ALREADY_EXISTS", username).dump() + '\0';
                send(clientSocket, promptName.c_str(), promptName.size(), 0);

            }


        } else if(clientSocketUser.find(clientSocket) == clientSocketUser.end()){
            std::cout << "Cliente desconectado." << std::endl;
            close(clientSocket);
            break; 

        } else if(type == "STATUS"){
            /* cambiarlo en el usuario
            */ 
            std::shared_ptr<User> usr = clientSocketUser[clientSocket];

            bool diferentStatus = usr->getEstuatus() != jsonMessage["status"];
            bool validStatus = jsonMessage["status"] == "ACTIVE" || jsonMessage["status"] == "AWAY" || jsonMessage["status"] == "BUSY";

            if(validStatus && diferentStatus){
                usr->setEstatus(jsonMessage["status"]);
                requestAnswer = Message::createNewStatusMessage(usr->getNombre(), usr->getEstuatus()).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);
            }
            

        } else if(type == "USERS") {
            std::map<std::string, std::string> users = nameRoom[""].obtenerEstatusUsuarios();
            
            requestAnswer = Message::createUserListMessage(users).dump()+'\0';
            send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

        } else if(type == "TEXT"){
            std::string username = jsonMessage["username"];
            std::string message = jsonMessage["text"];

            //Si el usuario no existe
            if(clientUserSocket.find(username) == clientUserSocket.end()){
                requestAnswer = Message::createNoSuchUserResponse(username).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            } else {
                requestAnswer = Message::createTextFromMessage(clientSocketUser[clientSocket]->getNombre(), message).dump()+'\0';
                send(clientUserSocket[username], requestAnswer.c_str(), requestAnswer.size(), 0);

            }


        } else if(type == "PUBLIC_TEXT"){
            std::string username = clientSocketUser[clientSocket]->getNombre();
            std::string message = jsonMessage["text"];

            requestAnswer = Message::createPublicTextFromMessage(username, message).dump()+'\0';

            for(auto &socket: clientSocketUser){
                if(socket.first != clientSocket){
                    send(socket.first, requestAnswer.c_str(), requestAnswer.size(), 0);
                }
            }


        } else if(type == "NEW_ROOM"){
            std::string roomname = jsonMessage["roomname"];
            std::shared_ptr<User> usr = clientSocketUser[clientSocket];

            if(rooms.count(roomname) <= 0){
                rooms.insert(roomname);

                nameRoom[roomname].agregarUsuario(usr);

                requestAnswer = Message::createNewRoomResponse(roomname, "SUCCESS").dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            } else {
                requestAnswer = Message::createNewRoomResponse(roomname, "ROOM_ALREADY_EXISTS").dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);
                
            }



        } else if(type == "INVITE"){
            std::string roomname = jsonMessage["roomname"];
            std::vector<std::string> usernames = jsonMessage["usernames"];

            if(rooms.count(roomname) <= 0 || !nameRoom[roomname].contieneUsuario(clientSocketUser[clientSocket]->getNombre())){
                requestAnswer = Message::createInviteResponse(roomname, "NO_SUCH_ROOM").dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            } else {
                bool success = true; 

                for(auto user: usernames){
                    if(clientUserSocket[user] == 0){
                        requestAnswer = Message::createInviteResponse(user, "NO_SUCH_USER").dump()+'\0';
                        send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);
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
                            requestAnswer = Message::createInvitationMessage(roomCreator->getNombre(), roomname).dump()+'\0';
                            send(userSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

                        }
                    }
                } 
            }

        } else if(type == "JOIN_ROOM"){
            std::string roomname = jsonMessage["roomname"];
            std::shared_ptr<User> user = clientSocketUser[clientSocket];

            if(rooms.count(roomname)>0 && user->contieneInvitacion(roomname)){
                nameRoom[roomname].agregarUsuario(user);

                requestAnswer = Message::createJoinRoomResponse(roomname, "SUCCESS").dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);
                
                for(auto elem: nameRoom[roomname].verUsuarios()){
                    requestAnswer = Message::createJoinedRoomMessage(roomname, user->getNombre()).dump()+'\0';
                    send(clientUserSocket[elem.first], requestAnswer.c_str(), requestAnswer.size(), 0);

                }

            } else if(rooms.count(roomname) <= 0){
                requestAnswer = Message::createJoinRoomResponse(roomname, "NO_SUCH_ROOM").dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            } else if(!user->contieneInvitacion(roomname)){
                requestAnswer = Message::createJoinRoomResponse(roomname, "NOT_INVITED").dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            }

        } else if(type == "ROOM_USERS"){
            std::string roomname = jsonMessage["roomname"];
            std::string username = clientSocketUser[clientSocket]->getNombre();

            if(rooms.count(roomname) > 0 && nameRoom[roomname].contieneUsuario(username)){

                requestAnswer = Message::createRoomUserListMessage(roomname, nameRoom[roomname].obtenerEstatusUsuarios()).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            } else if(rooms.count(roomname) <= 0){
                requestAnswer = Message::createRoomUsersResponseNo("NO_SUCH_ROOM", roomname).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);
                
            } else if(!nameRoom[roomname].contieneUsuario(username)){
                requestAnswer = Message::createRoomUsersResponseNo("NOT_JOINED", roomname).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            }
            

        } else if(type == "ROOM_TEXT"){
            std::string roomname = jsonMessage["roomname"];
            std::string text = jsonMessage["text"];
            std::string username = clientSocketUser[clientSocket]->getNombre();
            Sala currentRoom = nameRoom[roomname];

            if(rooms.count(roomname) > 0 && currentRoom.contieneUsuario(username)){

                for(auto usr: currentRoom.verUsuarios()){
                    int currentSocket = clientUserSocket[usr.first];

                    requestAnswer = Message::createRoomText(roomname, username, text).dump()+'\0';
                    send(currentSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

                }

            } else if(rooms.count(roomname) <= 0){
                requestAnswer = Message::createRoomTextResponse("NO_SUCH_ROOM", roomname).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);
                
            } else if(!nameRoom[roomname].contieneUsuario(username)){
                requestAnswer = Message::createRoomTextResponse("NOT_JOINED", roomname).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            }


        } else if(type == "LEAVE_ROOM"){
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

                    requestAnswer = Message::createRoomText(roomname, username).dump()+'\0';
                    send(currentSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

                }

            } else if(rooms.count(roomname) <= 0){
                requestAnswer = Message::leaveRoomResponse("NO_SUCH_ROOM", roomname).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);
                
            } else if(!nameRoom[roomname].contieneUsuario(username)){
                requestAnswer = Message::leaveRoomResponse("NOT_JOINED", roomname).dump()+'\0';
                send(clientSocket, requestAnswer.c_str(), requestAnswer.size(), 0);

            }



        } else if(type == "DISCONNECT"){
            requestAnswer = Message::disconnectMessage(clientSocketUser[clientSocket]->getNombre()).dump()+'\0';

            //add: desconexion de salas

            for(auto &socket: clientSocketUser){
                if(socket.first != clientSocket){
                    send(socket.first, requestAnswer.c_str(), requestAnswer.size(), 0);
                }
            }

            std::cout << "Cliente desconectado." << std::endl;
            close(clientSocket);

            removeUserFromRooms(clientSocket);
            break;


        }

        //Imprimos lo que dice dentro de nuestro server
        if(clientSocketUser[clientSocket]) std::cout << clientSocketUser[clientSocket]->getNombre() << ": " << mensaje << std::endl;
        else std::cout << "Anonimo" << ": " << mensaje << std::endl;

    }

}

