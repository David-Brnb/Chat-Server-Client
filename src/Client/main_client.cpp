#include <iostream>
#include "Client.h"

int main() {
    std::string ipAdress;
    int port;
    std::cout << "Ingrese la direccion ip: ";
    std::cin >> ipAdress;

    std::cout << "Ingrese la el puerto: ";
    std::cin >> port;

    // std::string ip = "172.20.10.7"; // IP del servidor
    // int port = 1024; // Puerto del servidor

    Client client(ipAdress, port);

    // Intentar conectar al servidor
    client.connectToServer();

    if (client.isConnected()) {
        // Ejecutar la lógica del cliente
        client.run();
    }

    return 0;
}
