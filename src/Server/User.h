#ifndef USER_H
#define USER_H

#include <string>
#include <set>
#include <nlohmann/json.hpp>

struct User {
private:
    std::string nombre;
    std::string estatus;
    int clientSocket;
    std::set<std::string> roomInvitations;

public:
    User() : nombre(""), estatus("inactivo"), clientSocket(-1) {}
    
    User(std::string nombre,std::string estatus, int clientSocket){
        this->nombre = nombre;
        this->estatus = estatus;
        this->clientSocket = clientSocket;
        
    }

    void setEstatus(std::string estatus);
    void insertaInvitacionSala(std::string sala);


    std::string getNombre();
    std::string getEstuatus();
    int getClientSocket();

    bool contieneInvitacion(std::string sala);

};

#endif // USER_H
