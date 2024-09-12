#include <iostream> 
#include "User.h" 

void User::setEstatus(std::string estatus){
    this->estatus = estatus;
}

std::string User::getNombre(){
    return nombre;
}

std::string User::getEstuatus(){
    return estatus;
}

int User::getClientSocket(){
    return clientSocket;
}

void User::agregarSala(std::string sala){
    this->rooms.insert(sala);

}

void User::salirDeSala(std::string sala){
    this->rooms.erase(sala);

}

bool User::estaEnSala(std::string sala){
    return rooms.count(sala) != 0;
}


void User::insertaInvitacionSala(std::string sala){
    roomInvitations.insert(sala);
}

void User::borraSalaInvitacion(std::string sala){
    roomInvitations.erase(sala);
}

bool User::contieneInvitacion(std::string sala){
    return roomInvitations.count(sala) != 0;
}
