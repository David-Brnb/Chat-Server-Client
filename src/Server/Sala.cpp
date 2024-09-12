#include <iostream> 
#include "Sala.h" 

Sala::Sala(){
    cantidadUsuarios = 0;
}

void Sala::agregarUsuario(User usr){
    cantidadUsuarios+=1;
    clientNamesStatus[usr.getNombre()] = usr.getEstuatus();

}

void Sala::eliminarUsuario(std::string username){
    cantidadUsuarios-=1;
    clientNamesStatus.erase(username);
    
}

std::map<std::string, std::string> Sala::verUsuarios(){
    return clientNamesStatus;

}

bool Sala::contieneUsuario(std::string username){
    return clientNamesStatus.find(username) != clientNamesStatus.end();
}