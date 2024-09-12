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
    std::set<std::string> rooms;

public:
    User() : nombre(""), estatus("inactivo"), clientSocket(-1) {}
    
    User(std::string nombre,std::string estatus, int clientSocket){
        this->nombre = nombre;
        this->estatus = estatus;
        this->clientSocket = clientSocket;
        
    }

    void setEstatus(std::string estatus);
    void agregarSala(std::string sala);
    void salirDeSala(std::string sala);
    void insertaInvitacionSala(std::string sala);
    void borraSalaInvitacion(std::string sala);


    std::string getNombre();
    std::string getEstuatus();
    int getClientSocket();

    bool estaEnSala(std::string sala);
    bool contieneInvitacion(std::string sala);

};

#endif // USER_H
