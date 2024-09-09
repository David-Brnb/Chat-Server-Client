#ifndef USER_H
#define USER_H

#include <string>

struct User {
private:
    std::string nombre;
    std::string estatus;
    int clientSocket;
    int sala; 

public:
    User() : nombre(""), estatus("inactivo"), clientSocket(-1), sala(0) {}
    
    User(std::string nombre,std::string estatus, int clientSocket, int sala){
        this->nombre = nombre;
        this->estatus = estatus;
        this->clientSocket = clientSocket;
        this->sala = sala;
    }

    void setEstatus(std::string estatus);
    void setSala(int sala);


    std::string getNombre();
    std::string getEstuatus();
    int getClientSocket();
    int getSala();




};

#endif // USER_H
