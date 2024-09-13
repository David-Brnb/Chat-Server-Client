#include <iostream> 
#include "Server.h"

using namespace std; 

int main(){
    // Crear una instancia del servidor en el puerto 8080
    Server myServer(1025);

    // Iniciar el servidor
    myServer.start();

    return 0;

    // cout << "HI" << endl;
}