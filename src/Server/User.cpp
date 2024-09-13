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


void User::insertaInvitacionSala(std::string sala){
    roomInvitations.insert(sala);
}

bool User::contieneInvitacion(std::string sala){
    return roomInvitations.count(sala) != 0;
}
