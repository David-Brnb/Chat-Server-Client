/*
    Header de la clase servidor
    Se definen atributos y comportamientos del servidor
*/
#ifndef SALA_H
#define SALA_H

#include <map>
#include "User.h"


class Sala{
    private:
        int cantidadUsuarios;
        std::map<std::string, std::string> clientNamesStatus;

    public:
    //Constructor, destructor y metodo de inicialización
    Sala();

    ~Sala(){
        
    }

    void agregarUsuario(User usr);
    void eliminarUsuario(std::string username);

    std::map<std::string, std::string> verUsuarios();
    bool contieneUsuario(std::string username);


};

#endif //SALA_H