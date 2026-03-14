# Arquitectura del Servidor ft_irc

El proyecto está diseñado bajo un paradigma orientado a objetos (C++98), organizando el sistema en tres clases principales que interactúan entre sí.

## 1. Clase `Server` (`Server.hpp` / `Server.cpp`)

Es el corazón de la aplicación. Gestiona la red y las conexiones, orquestando toda la comunicación.

**Responsabilidades principales:**
*   **Inicialización de Red**: Crea el socket de escucha (`initServerSocket`), configura la dirección IP y el puerto (`initServerAddress`), y prepara el array de estructuras para `poll()` (`initPolls`).
*   **Gestión de Conexiones**: Acepta nuevos clientes (`newClient`), almacena sus descriptores de archivo (sockets) y reacciona ante datos entrantes.
*   **Multiplexación con `poll()`**: Usa la función del sistema `poll()` para esperar actividad en cualquiera de los sockets sin bloquear el proceso completo (`checkConnections`, `updateConnections`).
*   **Enrutamiento de Mensajes**: Cuando recibe un mensaje (`manageClientMessage`), lo formatea (limpiando `\r\n`), lo divide (`parseMessage`) y luego delega la ejecución al comando específico (ej. `parseNick`, `joinChannel`, `parsePrivmsg`).
*   **Gestión Global**: Mantiene una lista central de todos los `clients` (como un `std::map`) y todos los `channels` (como un `std::vector`).

## 2. Clase `Client` (`Client.hpp` / `Client.cpp`)

Representa la conexión y estado de un único usuario.

**Responsabilidades principales:**
*   **Datos de Identidad**: Almacena el socket de la conexión, el `nick` (apodo), el `user` (nombre de usuario) y el `realname` (nombre real).
*   **Estados de Autenticación**: Lleva control de si el cliente ha enviado la contraseña correcta (`verified`) y si ha registrado su usuario (`username_verified`).
*   **Buffer de Mensajes**: Mantiene partes del comando recibido si este llega fragmentado a través de la red (`message`). También guarda la respuesta que se va a enviar (`response`).
*   **Información de Conexión**: Conserva datos de la IP de origen y gestiona su propio `hostname`.

## 3. Clase `Channel` (`Channel.hpp` / `Channel.cpp`)

Modela una sala de chat grupal y encapsula sus propias reglas.

**Responsabilidades principales:**
*   **Identificación y Miembros**: Guarda el nombre del canal y mantiene dos listas separadas: usuarios normales (`clients`) y operadores/moderadores del canal (`moderators`).
*   **Modos del Canal**: Implementa los modos específicos requeridos por IRC:
    *   `modeLimit` (`+l`): Límite de capacidad.
    *   `modeKey` (`+k`): Contraseña requerida para entrar.
    *   `modeInvite` (`+i`): Canal solo por invitación.
    *   `modeTopic` (`+t`): Sólo operadores pueden cambiar el tema (`topic`).
*   **Manejo Interno**: Permite añadir usuarios (`addClient`), removerlos (`removeClient`), expulsarlos (`kickClient`), e identificar su estatus (si es moderador, si está invitado).
*   **Broadcast**: Dispone de la capacidad de enviar un mensaje a todos sus miembros (`sendResponseChannel`).

## Relaciones entre Clases

La arquitectura forma un ecosistema centralizado:
*   El **`Server`** posee la memoria principal de todos los **`Client`**s y **`Channel`**s.
*   Cuando un **`Client`** quiere ejecutar un comando en un **`Channel`** (por ejemplo, `JOIN`), la solicitud la recibe y procesa el **`Server`**, el cual extrae el objeto **`Channel`** adecuado de su lista, evalúa si el **`Client`** tiene permiso, y de ser así, inserta la referencia o copia del **`Client`** dentro de las estructuras del **`Channel`**.