#include "Client.h"
#include "Message.h"

Client::Client(std::string ip, int port) : connected(false) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error al crear el socket del cliente\n";
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr);
}

Client::~Client() {
    disconnect();
}

void Client::connectToServer() {
    int connectionStatus = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    
    if (connectionStatus < 0) {
        std::cerr << "Error al conectarse al servidor. Código de error: " << errno << std::endl;
        exit(EXIT_FAILURE);  // Verifica si realmente necesitas salir aquí
    }

    connected = true;
    listenerThread = std::thread(&Client::listenForMessages, this);
}

void Client::sendMessage(const nlohmann::json& message) {
    std::lock_guard<std::mutex> lock(socketMutex);
    std::string msgString = message.dump()+'\0';
    send(clientSocket, msgString.c_str(), msgString.size(), 0);
}

void Client::listenForMessages() {
    char buffer[1024];
    while (connected) {
        int bites = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bites <= 0) {
            std::cerr << "Desconectado del servidor\n";
            disconnect();
            connected = false;
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
            disconnect();
            connected = false;
            break;
        }

        std::string answer;
        // std::cout << mensaje << std::endl;

        if(jsonMessage["type"] == "RESPONSE"){
        std::cout << mensaje << std::endl;

            std::string user = jsonMessage["extra"];
            
            if(jsonMessage.contains("operation") && jsonMessage["operation"] == "IDENTIFY" && jsonMessage["result"] == "SUCCESS"){
                answer = "El usuario " + user + " ha sido registrado con exito!";
                std::cout << answer << std::endl;

                registered = true;

            } else if(jsonMessage.contains("operation") && jsonMessage["operation"] == "IDENTIFY" && jsonMessage["result"] == "USER_ALREADY_EXISTS"){
                answer = "El usuario " + user + " ya existe, ingrese otro nombre.";
                std::cout << answer << std::endl;
                registered = false;

            } else if(jsonMessage.contains("request") && jsonMessage["request"] == "INVALID"){
                if(jsonMessage["result"] == "NOT_IDENTIFIED"){
                    answer = "cerr: El usuario hizo request sin identificarse primero\n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "INVALID"){
                    answer = "cerr: El usuario mando parametros incompletos\n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;
                }
                
            } else if(jsonMessage.contains("request") && jsonMessage["request"] == "NEW_ROOM"){
                std::string sala = jsonMessage["extra"];

                if(jsonMessage["result"] == "SUCCESS"){
                    answer = "Sala " + sala + " creada con exito!\n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "ROOM_ALREADY_EXISTS"){
                    answer = "La sala " + sala + " ya existe. \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                }

            } else if(jsonMessage.contains("request") && jsonMessage["request"] == "INVITE"){
                std::string extra = jsonMessage["extra"];

                if(jsonMessage["result"] == "NO_SUCH_ROOM"){
                    answer = "La sala " + extra + " no existe o no estras dentro. \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "NO_SUCH_USER"){
                    answer = "El usuario " + extra + " no existe. \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                }

            } else if(jsonMessage.contains("request") && jsonMessage["request"] == "JOIN_ROOM"){
                std::string extra = jsonMessage["extra"];

                if(jsonMessage["result"] == "SUCCESS"){
                    answer = "has ingresado a la sala " + extra + "! \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "NO_SUCH_ROOM"){
                    answer = "La sala " + extra + " no existe. \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "NOT_INVITED"){
                    answer = "No tienes invitacion para entrar a la sala: " + extra + ". \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                }

            } else if(jsonMessage.contains("request") && jsonMessage["request"] == "ROOM_USERS"){
                std::string extra = jsonMessage["extra"];

                if(jsonMessage["result"] == "NO_SUCH_ROOM"){
                    answer = "La sala " + extra + " no existe. \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "NOT_JOINED"){
                    answer = "No estas en la sala: " + extra + ". \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                }

            } else if(jsonMessage.contains("request") && jsonMessage["request"] == "ROOM_TEXT"){
                std::string extra = jsonMessage["extra"];

                if(jsonMessage["result"] == "NO_SUCH_ROOM"){
                    answer = "La sala " + extra + " no existe. \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "NOT_JOINED"){
                    answer = "No estas en la sala: " + extra + ". \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                }

            } else if(jsonMessage.contains("request") && jsonMessage["request"] == "LEAVE_ROOM"){
                std::string extra = jsonMessage["extra"];

                if(jsonMessage["result"] == "NO_SUCH_ROOM"){
                    answer = "La sala " + extra + " no existe. \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                } else if(jsonMessage["result"] == "NOT_JOINED"){
                    answer = "No estas en la sala: " + extra + ". \n";
                    if(waiting) waitedMessages.push_back(answer);
                    else std::cout << answer;

                }
            }
            

        } else if(jsonMessage["type"] == "NEW_USER"){

            std::string user = jsonMessage["username"];
            answer = "Chat (general): da la bienvenida a " + user + " en el chat!\n";
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;
        
        } else if(jsonMessage["type"] == "NEW_STATUS") {
            std::string user = jsonMessage["username"];
            std::string status = jsonMessage["status"];
            answer = "Ahora " + user + " esta " + status + "\n";
            
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "USER_LIST"){
            answer = "-- Usuarios en el servidor --\n";
            std::map<std::string, std::string> clientNamesStatus;
            clientNamesStatus = jsonMessage["users"];

            for(auto usr: clientNamesStatus){
                answer+= "  " + usr.first + " : " + usr.second + "\n";
            }

            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "TEXT_FROM"){
            answer = jsonMessage["username"].get<std::string>() + " (directo): " + jsonMessage["text"].get<std::string>();
            answer += "\n";
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if (jsonMessage["type"] == "PUBLIC_TEXT_FROM"){
            answer = jsonMessage["username"].get<std::string>() + " (general): " + jsonMessage["text"].get<std::string>();
            answer += "\n";
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "JOINED_ROOM") {
            answer = jsonMessage["username"].get<std::string>() + " ingreso a la sala " + jsonMessage["roomname"].get<std::string>();
            answer += "\n";
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "INVITATION") {
            answer = jsonMessage["username"].get<std::string>() + " (Invitation): Unete a mi sala " + jsonMessage["roomname"].get<std::string>() + "!";
            answer += "\n";
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "ROOM_TEXT_FROM") {
            answer = jsonMessage["username"].get<std::string>() + " (Sala - " + jsonMessage["roomname"].get<std::string>() + "): " + jsonMessage["text"].get<std::string>();
            answer += "\n";
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "LEFT_ROOM") {
            answer =  "Sala (" + jsonMessage["roomname"].get<std::string>() + "): " + jsonMessage["username"].get<std::string>() + " dejo la sala :(";
            answer += "\n";
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "ROOM_USER_LIST"){
            answer = "-- Usuarios en la sala: " + jsonMessage["roomname"].get<std::string>() + " \n";
            std::map<std::string, std::string> clientNamesStatus;
            clientNamesStatus = jsonMessage["users"];

            for(auto usr: clientNamesStatus){
                answer+= "  " + usr.first + " : " + usr.second + "\n";
            }

            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        } else if(jsonMessage["type"] == "DISCONNECTED"){
            answer = "Chat (general): " + jsonMessage["username"].get<std::string>() + " dejo el servidor. \n";
            
            if(waiting) waitedMessages.push_back(answer);
            else std::cout << answer;

        }

    }
}

void Client::run() {

    std::string input;

    while (connected) {

        std::cin >> input;

        waiting = true; 
        if(input == "/Identify" && !registered){
            while(true){
                std::cout << "Ingrese su nombre de identificación: ";
                std::cin >> input;

                if(input.size() > 8){
                    std::cout << "err: Max 8 caracteres \n";

                } else {
                    break;
                }
            }

            nlohmann::json message = Message::createIdentifyMessage(input);
            sendMessage(message);

        } else if(input == "/Status"){
            std::string newStatus; 
            std::cout << "Eliga un estatus de los listados: \n";
            std::cout << " * ACTIVE \n";
            std::cout << " * AWAY \n";
            std::cout << " * BUSY \n";
            while(true){
                std::cout << "Ingrese su nuevo estatus (escrito de la misma forma): ";
                std::cin >> newStatus; 

                if(!(newStatus == "ACTIVE" || newStatus == "AWAY" || newStatus == "BUSY")){
                    std::cout << "err: Status desconocido \n";

                } else {
                    break;
                }
            }
            

            nlohmann::json message = Message::createStatusMessage(newStatus);
            sendMessage(message);

        } else if(input == "/Users"){
            nlohmann::json message = Message::createUsersRequest();
            sendMessage(message);

        } else if(input == "/Whisper"){
            std::string seeUsers;
            std::string destinyUser;
            std::string fullMessage;
            std::string line; 

            std::cout << "Indique si desea ver el listado de usuarios (SI - NO): ";
            std::cin >> seeUsers;

            waiting = false;
            if(seeUsers == "SI"){
                sendMessage(Message::createUsersRequest());
            } 

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            waiting = true;

            std::cout << "Indique el usuario al que enviará el mensaje: ";
            std::cin >> destinyUser;

            
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Escribe tu mensaje (presiona ENTER en una línea vacía para finalizar): " << std::endl;

            
            while (true) {
                std::getline(std::cin, line);

                if (line.empty()) {  
                    break;
                }

                fullMessage += line + '\n';
            }

            nlohmann::json message = Message::createTextMessage(destinyUser, fullMessage);
            sendMessage(message);

        } else if(input == "/Public_Whisper") {
            std::string fullMessage;
            std::string line; 

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Escribe tu mensaje (presiona ENTER en una línea vacía para finalizar):" << std::endl;

            
            while (true) {
                std::getline(std::cin, line);

                if (line.empty()) {  
                    break;
                }

                fullMessage += line + '\n';
            }

            nlohmann::json message = Message::createPublicTextMessage(fullMessage);
            sendMessage(message);

        } else if(input == "/Create_Room"){
            while(true){
                std::cout << "Ingrese el nombre de la Sala: ";
                std::cin >> input;

                if(input.size() > 16){
                    std::cout << "err: Max 16 caracteres \n";

                } else {
                    break;
                }
            }

            nlohmann::json message = Message::createNewRoomMessage(input);
            sendMessage(message);


        } else if(input == "/Invite_Room"){
            std::string sala, line; 
            std::vector<std::string> users;

            std::cout << "Ingrese el nombre de la Sala: ";
            std::cin >> sala;

            // Limpiar el buffer de entrada después de usar std::cin
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Ingrese (en lineas diferentes) el nombre de los Usuarios a invitar presiona ENTER en una línea vacía para finalizar): \n";
            
            while (true) {
                std::getline(std::cin, line);

                if (line.empty()) {  
                    break;
                }

                if(line != "")users.push_back(line);
            }

            nlohmann::json message = Message::createInviteMessage(sala, users);
            sendMessage(message);

            
        } else if(input == "/Join_Room"){
            std::string sala; 

            std::cout << "Ingrese el nombre de la Sala a unirse: ";
            std::cin >> sala;

            nlohmann::json message = Message::createJoinRoomMessage(sala);
            sendMessage(message);

            
        } else if(input == "/Room_Users"){
            std::string sala; 

            std::cout << "Ingrese el nombre de la Sala a consultar: ";
            std::cin >> sala;

            nlohmann::json message = Message::createRoomUsersMessage(sala);
            sendMessage(message);
            
        } else if(input == "/Room_Text"){
            std::string sala, line, fullMessage;

            std::cout << "Ingrese el nombre de la Sala: ";
            std::cin >> sala;

            // Limpiar el buffer de entrada después de usar std::cin
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::cout << "Escribe tu mensaje (presiona ENTER en una línea vacía para finalizar):" << std::endl;

            
            while (true) {
                std::getline(std::cin, line);

                if (line.empty()) {  
                    break;
                }

                if(line != "") fullMessage += line + '\n';
            }

            nlohmann::json message = Message::createRoomTextRequest(sala, fullMessage);
            sendMessage(message);


            
        } else if(input == "/Leave_Room"){
            std::string sala;

            std::cout << "Ingrese el nombre de la Sala: ";
            std::cin >> sala;

            nlohmann::json message = Message::leaveRoom(sala);
            sendMessage(message);

        } else if(input == "/Disconnect"){
            nlohmann::json message = Message::disconnectRequest();
            sendMessage(message);
            disconnect();
            break;

        } else {
            std::cout << "Ingresa un comando valido " << std::endl;
        }

        waiting = false; 

        for(auto ans: waitedMessages){
            std::cout << ans;
        }
        waitedMessages.clear();
    }
}

void Client::disconnect() {
    if (connected) {
        connected = false;
        close(clientSocket);
        if (listenerThread.joinable()) {
            listenerThread.join();
        }
        std::cout << "Cliente desconectado\n";
    }
}

bool Client::isConnected() const {
    return connected;
}
