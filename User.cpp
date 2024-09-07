#include <iostream> 
#include "User.h" 

void User::setEstatus(std::string estatus){
    this->estatus = estatus;
}

void User::setSala(int sala){
    this->sala = sala;
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

int User::getSala(){
    return sala;
}
