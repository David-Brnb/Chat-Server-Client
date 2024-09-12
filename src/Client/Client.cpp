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
        std::cerr << "Error al conectarse al servidor\n";
        exit(EXIT_FAILURE);
    }

    connected = true;
    listenerThread = std::thread(&Client::listenForMessages, this);
    std::cout << "Conectado al servidor!\n"; // Borrar
}

void Client::sendMessage(const nlohmann::json& message) {
    std::lock_guard<std::mutex> lock(socketMutex);
    std::string msgString = message.dump();
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

        if(jsonMessage["type"] == "RESPONSE"){
            std::string user = jsonMessage["extra"];
            
            if(jsonMessage["request"] == "IDENTIFY" && jsonMessage["result"] == "SUCCESS"){
                answer = "El usuario " + user + " ha sido registrado con exito!";
                std::cout << answer << std::endl;

                registered = true;

            } else if(jsonMessage["request"] == "IDENTIFY" && jsonMessage["result"] == "USER_ALREADY_EXISTS"){
                answer = "El usuario " + user + " ya existe, ingrese otro nombre.";
                std::cout << answer << std::endl;
                registered = false;

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

        } else {
            std::cout << mensaje << "\n";

        }



    }
}

void Client::run() {
    std::string input;

    while (connected) {

        if(!registered){
            std::cout << "Ingrese su nombre de identificación: ";
            //revisar el numerp de caracteres
            std::cin >> input;


            nlohmann::json message = Message::createIdentifyMessage(input);
            sendMessage(message);

            std::this_thread::sleep_for(std::chrono::milliseconds(700));

            continue;

        } 

        std::cin >> input;

        waiting = true; 

        if(input == "/Status"){
            std::string newStatus; 
            std::cout << "Eliga un estatus de los listados: \n";
            std::cout << " * ACTIVE \n";
            std::cout << " * AWAY \n";
            std::cout << " * BUSY \n";
            std::cout << "Ingrese su nuevo estatus (escrito de la misma forma): ";

            std::cin >> newStatus; 

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

        } else if(input == "/Disconnect"){
            nlohmann::json message = Message::disconnectRequest();
            sendMessage(message);
            disconnect();
            break;

        }

        if (input == "/exit") {
            disconnect();
            break;
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
