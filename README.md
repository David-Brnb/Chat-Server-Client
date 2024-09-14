# Chat-Server-Client

Este proyecto implementa un **chat cliente-servidor** en C++ utilizando sockets. El servidor acepta múltiples clientes y les permite comunicarse a través de diferentes comandos, como la creación de salas de chat, envío de mensajes privados y mensajes públicos.

## Características
- Comunicación entre clientes a través de un servidor centralizado.
- Soporte para crear y unirse a salas de chat.
- Envío de mensajes directos entre usuarios.
- Envío de mensajes públicos en una sala de chat.
- Listado de usuarios conectados en el servidor y dentro de una sala.
- Manejo de invitaciones a salas privadas.
- Soporte para múltiples estados de usuario (`ACTIVE`, `AWAY`, `BUSY`).


## Estructura del Proyecto
- `main_server.cpp`: Archivo principal para el servidor.
- `main_client.cpp`: Archivo principal para el cliente.
- `Server.h` y `Server.cpp`: Implementación de la clase del servidor.
- `Client.h` y `Client.cpp`: Implementación de la clase del cliente.
- `Sala.h` y `Sala.cpp`: Implementación de la clase Sala (contiene información de los interantes).
- `User.h` y `User.cpp`: Implementación de la clase Usuario (manejo de datos del usuario).
- `Message.h`: Implementación de la clase Message (fabrica de tipo JSON).

## Requisitos
- C++17 o superior
- **Biblioteca nlohmann::json** para manejo de JSON.
- [Meson](https://mesonbuild.com/) 
- [Ninja](https://ninja-build.org/) (se instala junto con Meson)
- `make` (para la compilación y ejecución rápida)

## Compilación

Puedes compilar el proyecto mediante el uso de Make:

Utilizando `make`

Ejecuta el siguiente comando para compilar el proyecto completo (desde ~/ChatApp/):

```bash
make

```

## Ejecución

Ejecuta el siguiente comando para compilar y ejecutar el Servidor (desde ~/ChatApp/):

```bash
make run-server
```

Ejecuta el siguiente comando para compilar y ejecutar el Cliente (desde ~/ChatApp/):

```bash
make run-client
```


## Instrucciones del Cliente

Después de ejecutar el cliente y conectarte al servidor, puedes usar los siguientes comandos para interactuar con el sistema de chat (ingresa el comando y presiona ENTER):

### Comandos disponibles

- **/Identify**: Registra tu nombre de usuario en el servidor.
    ```bash
    /Identify
    ```
    Se te pedirá que ingreses un nombre de usuario único (máximo 8 caracteres).

- **/Status**: Cambia tu estado de usuario.
    ```bash
    /Status
    ```
    Te pedirá que elijas entre `ACTIVE`, `AWAY`, o `BUSY`.

- **/Users**: Lista todos los usuarios conectados en el servidor.
    ```bash
    /Users
    ```

- **/Whisper**: Envía un mensaje directo a otro usuario.
    ```bash
    /Whisper
    ```

- **/Public_Whisper**: Envía un mensaje público a todos los usuarios conectados.
    ```bash
    /Public_Whisper
    ```

- **/Create_Room**: Crea una nueva sala de chat.
    ```bash
    /Create_Room
    ```

- **/Invite_Room**: Invita a usuarios a una sala específica.
    ```bash
    /Invite_Room
    ```

- **/Join_Room**: Únete a una sala de chat.
    ```bash
    /Join_Room
    ```

- **/Room_Users**: Muestra los usuarios en una sala específica.
    ```bash
    /Room_Users
    ```

- **/Room_Text**: Envía un mensaje a una sala específica.
    ```bash
    /Room_Text
    ```

- **/Leave_Room**: Deja una sala de chat.
    ```bash
    /Leave_Room
    ```

- **/Disconnect**: Desconecta al cliente del servidor.
    ```bash
    /Disconnect
    ```

## Notas

- El cliente está diseñado para manejar múltiples tipos de mensajes de respuesta del servidor, incluidos errores como "usuario ya registrado" o "sala no existe".
- Se incluye manejo básico de excepciones para errores de red o problemas al parsear JSON.
- El servidor se encarga de gestionar la comunicación entre los clientes y las operaciones en las salas de chat.

## Créditos

Este proyecto fue desarrollado como parte de un ejercicio de implementación de un chat cliente-servidor en C++ utilizando sockets y hilos para manejar múltiples clientes de forma simultánea.

Desarrollado por: David Leónidas Bernabe Torres
