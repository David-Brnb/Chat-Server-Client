#include <iostream> 
#include "Sala.h" 

Sala::Sala(){
    cantidadUsuarios = 0;
}

void Sala::agregarUsuario(std::shared_ptr<User> usr){
    cantidadUsuarios+=1;
    clientNamesStatus[usr->getNombre()] = usr;

}

void Sala::eliminarUsuario(std::string username){
    if (clientNamesStatus.erase(username)) {
        cantidadUsuarios--;
    }
    
}

std::map<std::string, std::shared_ptr<User>> Sala::verUsuarios(){
    return clientNamesStatus;

}

bool Sala::contieneUsuario(std::string username){
    return clientNamesStatus.find(username) != clientNamesStatus.end();
}

int Sala::cantidadDeUsuarios(){
    return cantidadUsuarios;
}

std::map<std::string, std::string> Sala::obtenerEstatusUsuarios() {
    std::map<std::string, std::string> userStatuses;
    
    for (const auto& [username, userPtr] : clientNamesStatus) {
        userStatuses[username] = userPtr->getEstuatus();  // Extrae solo el estatus del usuario
    }

    return userStatuses;
}
