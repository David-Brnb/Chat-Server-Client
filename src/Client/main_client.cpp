#include <iostream>
#include "Client.h"

int main() {
    std::string ip = "10.48.251.200"; // IP del servidor
    int port = 1024; // Puerto del servidor

    Client client(ip, port);

    // Intentar conectar al servidor
    client.connectToServer();

    if (client.isConnected()) {
        // Ejecutar la lógica del cliente
        client.run();
    }

    return 0;
}
