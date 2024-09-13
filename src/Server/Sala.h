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
        std::map<std::string, std::shared_ptr<User> > clientNamesStatus;

    public:
    //Constructor, destructor y metodo de inicialización
    Sala();

    ~Sala(){
        
    }

    void agregarUsuario(std::shared_ptr<User> usr);
    void eliminarUsuario(std::string username);

    std::map<std::string, std::shared_ptr<User>> verUsuarios();
    std::map<std::string, std::string> obtenerEstatusUsuarios();
    bool contieneUsuario(std::string username);
    int cantidadDeUsuarios();


};

#endif //SALA_H