#ifndef MESSAGE_H
#define MESSAGE_H

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

class Message {
public:
    // Mensaje de identificación
    static nlohmann::json createIdentifyMessage(std::string &username) {
        return {
            {"type", "IDENTIFY"},
            {"username", username}
        };
    }

    // Respuesta a la identificación
    static nlohmann::json createIdentifyResponse(std::string result, std::string &username) {
        return {
            {"type", "RESPONSE"},
            {"request", "IDENTIFY"},
            {"result", result},
            {"extra", username}
        };
    }

    // Mensaje de nuevo usuario
    static nlohmann::json createNewUserMessage(std::string &username) {
        return {
            {"type", "NEW_USER"},
            {"username", username}
        };
    }

    // Mensaje de estado
    static nlohmann::json createStatusMessage(std::string &status) {
        return {
            {"type", "STATUS"},
            {"status", status}
        };
    }

    // Notificación de nuevo estado
    static nlohmann::json createNewStatusMessage(std::string username, std::string status) {
        return {
            {"type", "NEW_STATUS"},
            {"username", username},
            {"status", status}
        };
    }

    // Solicitud de lista de usuarios
    static nlohmann::json createUsersRequest() {
        return {
            {"type", "USERS"}
        };
    }

    // Respuesta con lista de usuarios
    static nlohmann::json createUserListMessage(std::map<std::string, std::string> &users) {
        return {
            {"type", "USER_LIST"},
            {"users", users}
        };
    }

    // Mensaje de texto privado
    static nlohmann::json createTextMessage(std::string &username, std::string &text) {
        return {
            {"type", "TEXT"},
            {"username", username},
            {"text", text}
        };
    }

    // Respuesta a texto privado
    static nlohmann::json createTextFromMessage(std::string username, std::string &text) {
        return {
            {"type", "TEXT_FROM"},
            {"username", username},
            {"text", text}
        };
    }

    // Respuesta cuando el usuario no existe
    static nlohmann::json createNoSuchUserResponse(std::string &username) {
        return {
            {"type", "RESPONSE"},
            {"request", "TEXT"},
            {"result", "NO_SUCH_USER"},
            {"extra", username}
        };
    }

    // Mensaje de texto público
    static nlohmann::json createPublicTextMessage(std::string &text) {
        return {
            {"type", "PUBLIC_TEXT"},
            {"text", text}
        };
    }

    // Notificación de texto público enviado
    static nlohmann::json createPublicTextFromMessage(std::string &username, std::string &text) {
        return {
            {"type", "PUBLIC_TEXT_FROM"},
            {"username", username},
            {"text", text}
        };
    }

    // Solicitud para crear una nueva sala
    static nlohmann::json createNewRoomMessage(std::string &roomName) {
        return {
            {"type", "NEW_ROOM"},
            {"roomname", roomName}
        };
    }

    // Método para crear un mensaje de INVITE
    static nlohmann::json createInviteMessage(std::string &roomName, std::vector<std::string> &userNames) {
        return {
            {"type", "INVITE"},
            {"roomname", roomName},
            {"usernames", userNames}
        };
    }

    // Método para crear un mensaje de INVITATION
    static nlohmann::json createInvitationMessage(std::string &username, std::string &roomName) {
        return {
            {"type", "INVITATION"},
            {"username", username},
            {"roomname", roomName}
        };
    }

    // Método para crear un mensaje de respuesta a INVITE (si la sala no existe)
    static nlohmann::json createInviteResponseNoSuchRoom(std::string &roomName) {
        return {
            {"type", "RESPONSE"},
            {"request", "INVITE"},
            {"result", "NO_SUCH_ROOM"},
            {"extra", roomName}
        };
    }

    // Método para crear un mensaje de respuesta a INVITE (si un usuario no existe)
    static nlohmann::json createInviteResponseNoSuchUser(std::string &username) {
        return {
            {"type", "RESPONSE"},
            {"request", "INVITE"},
            {"result", "NO_SUCH_USER"},
            {"extra", username}
        };
    }

    // Método para crear un mensaje de JOIN_ROOM
    static nlohmann::json createJoinRoomMessage(std::string &roomName) {
        return {
            {"type", "JOIN_ROOM"},
            {"roomname", roomName}
        };
    }

    // Método para crear un mensaje de respuesta a JOIN_ROOM (si el usuario se une con éxito)
    static nlohmann::json createJoinRoomResponseSuccess(std::string &roomName) {
        return {
            {"type", "RESPONSE"},
            {"request", "JOIN_ROOM"},
            {"result", "SUCCESS"},
            {"extra", roomName}
        };
    }

    // Método para crear un mensaje de JOINED_ROOM para notificar a otros usuarios
    static nlohmann::json createJoinedRoomMessage(std::string &roomName, std::string &username) {
        return {
            {"type", "JOINED_ROOM"},
            {"roomname", roomName},
            {"username", username}
        };
    }

    // Método para crear un mensaje de respuesta a JOIN_ROOM (si la sala no existe)
    static nlohmann::json createJoinRoomResponseNo(std::string &roomName, std::string &result) {
        return {
            {"type", "RESPONSE"},
            {"request", "JOIN_ROOM"},
            {"result", result},
            {"extra", roomName}
        };
    }

    // Método para crear un mensaje de ROOM_USERS
    static nlohmann::json createRoomUsersMessage(std::string &roomName) {
        return {
            {"type", "ROOM_USERS"},
            {"roomname", roomName}
        };
    }

    // Método para crear un mensaje de ROOM_USER_LIST
    static nlohmann::json createRoomUserListMessage(std::string &roomName, std::map<std::string, std::string> &users) {
        return {
            {"type", "ROOM_USER_LIST"},
            {"roomname", roomName},
            {"users", users}
        };
    }

    // Método para crear un mensaje de respuesta a ROOM_USERS (si la sala no existe)
    static nlohmann::json createRoomUsersResponseNoSuchRoom(std::string &roomName) {
        return {
            {"type", "RESPONSE"},
            {"request", "ROOM_USERS"},
            {"result", "NO_SUCH_ROOM"},
            {"extra", roomName}
        };
    }

    // Método para crear un mensaje de respuesta a ROOM_USERS (si el usuario no se ha unido)
    static nlohmann::json createRoomUsersResponseNotJoined(std::string &roomName) {
        return {
            {"type", "RESPONSE"},
            {"request", "ROOM_USERS"},
            {"result", "NOT_JOINED"},
            {"extra", roomName}
        };
    }

    // Método para crear un mensaje ROOM_TEXT
    static nlohmann::json createRoomTextRequest(std::string &roomName, std::string &text) {
        return {
            {"type", "ROOM_TEXT"},
            {"roomname", roomName},
            {"text", text}
        };
    }

    // Método para crear un mensaje a los integrantes de la sala
    static nlohmann::json createRoomText(std::string &roomName, std::string &username, std::string &text) {
        return {
            {"type", "ROOM_TEXT_FROM"},
            {"roomname", roomName},
            {"username", username},
            {"text", text}
        };
    }

    // Método para crear un mensaje de respuesta a ROOM_TEXT(ocurrió un error)
    static nlohmann::json createRoomTextResponse(std::string &roomName, std::string &message) {
        return {
            {"type", "RESPONSE"},
            {"request", "ROOM_TEXT"},
            {"result", message},
            {"extra", roomName}
        };
    }

    // Método para crear una peticion de desconexion al servidor
    static nlohmann::json disconnectRequest() {
        return {
            {"type", "DISCONNECT"}
        };
    }

    // Método para crear un mensaje de desconexion a los usuarios
    static nlohmann::json disconnectMessage(std::string username) {
        return {
            {"type", "DISCONNECT"},
            {"username", username}
        };
    }






};

#endif // MESSAGE_H

// Dividir por server y cliente
// Crear set de nombres de salas
