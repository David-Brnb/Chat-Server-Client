#ifndef USER_H
#define USER_H

#include <string>
#include <nlohmann/json.hpp>

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

    // Método para convertir el objeto User a JSON
    std::string toJSON() const;

    // Método para inicializar un objeto User a partir de JSON
    void fromJSON(std::string& jsonString);

};

#endif // USER_H
