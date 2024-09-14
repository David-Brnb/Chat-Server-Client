#include <iostream>
#include "Client.h"

int main() {
    std::string ipAdress;
    int port;
    std::cout << "Ingrese la direccion ip: ";
    std::cin >> ipAdress;

    std::cout << "Ingrese la el puerto: ";
    std::cin >> port;

    Client client(ipAdress, port);

    // Intentar conectar al servidor
    client.connectToServer();

    if (client.isConnected()) {
        // Ejecutar la lógica del cliente
        client.run();
    }

    return 0;
}
