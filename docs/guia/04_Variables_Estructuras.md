# Variables y Estructuras Clave en ft_irc

Para entender cómo `ft_irc` maneja los datos concurrentemente sin bases de datos, es crucial revisar cómo guarda el estado en memoria usando contenedores de la Standard Template Library (STL) de C++98.

## 1. Dentro de la clase `Server` (`Server.hpp`)

El objeto `Server` es el dueño de casi todo el estado del programa.

### `std::vector<struct pollfd> polls`
*   **Qué es:** El array dinámico que se le pasa directamente a la función del sistema `poll()`.
*   **Uso:** Cada vez que se acepta un cliente nuevo, su descriptor de socket (`fd`) se añade a este vector. Cuando un cliente se va, se elimina de aquí. `poll()` se encarga de vigilar todos los sockets de esta lista simultáneamente.

### `std::map<int, Client> clients`
*   **Qué es:** Un diccionario que asocia el descriptor de archivo (el socket, de tipo `int`) con su correspondiente objeto `Client`.
*   **Uso:** Búsqueda ultrarrápida (O(log n)). Cuando `poll()` avisa que el socket número `5` tiene datos nuevos, el servidor accede instantáneamente al cliente haciendo `this->clients[5]`.

### `std::vector<Channel> channels`
*   **Qué es:** Una lista de todos los canales activos en el servidor.
*   **Uso:** Cuando se ejecuta un comando como `JOIN #general`, el servidor recorre este vector buscando si ya existe un canal con el nombre `#general`. Si no existe, crea un nuevo objeto `Channel` y lo añade con `push_back`.

### `std::map<ERR, std::string> errorMessages` (Global en `main.cpp`)
*   **Qué es:** Un diccionario estático (inicializado al arrancar) que mapea un código de error numérico de IRC (definido en el `enum ERR` en `irc.hpp`) con su mensaje de texto correspondiente.
*   **Uso:** Facilita la construcción de respuestas. Cuando una función llama a `err(ERR_NOSUCHNICK, ...)`, el sistema busca en este mapa el texto `"No such nick/channel"`.

---

## 2. Dentro de la clase `Client` (`Client.hpp`)

Cada usuario conectado tiene su propia instancia de `Client`.

### `std::string message`
*   **Qué es:** Un buffer temporal de entrada.
*   **Uso:** Como `recv()` no garantiza leer un comando completo en una sola pasada, los datos crudos se van acumulando en este `std::string`. Solo cuando se encuentra un `\n`, se extrae la línea y se procesa.

### `std::vector<std::string> fullmsg`
*   **Qué es:** El comando actual ya parseado y dividido en palabras.
*   **Uso:** Si el cliente envía `JOIN #test`, `fullmsg[0]` será `"JOIN"` y `fullmsg[1]` será `"#test"`. Esto facilita muchísimo el trabajo de los comandos (por ejemplo en `_join.cpp`).

### `std::string response`
*   **Qué es:** El buffer de salida.
*   **Uso:** En lugar de enviar la respuesta inmediatamente por la red cada vez que se detecta un error o un éxito, los mensajes se concatenan en este string. Al final del ciclo de procesamiento del comando (`parseMessage`), se llama a `sendResponse()`, que envía todo el bloque usando `send()` y luego vacía la variable.

---

## 3. Dentro de la clase `Channel` (`Channel.hpp`)

Cada sala de chat se modela con un objeto `Channel`.

### `std::vector<Client> clients`
*   **Qué es:** La lista de usuarios "normales" que están actualmente dentro del canal.
*   **Uso:** Cuando alguien hace un `PRIVMSG #canal`, el canal itera sobre este vector reenviando el mensaje a cada uno de sus miembros.

### `std::vector<Client> moderators`
*   **Qué es:** La lista de usuarios que tienen privilegios de operador (modo `+o`) en este canal.
*   **Uso:** Funciones como `KICK` o cambiar el `TOPIC` primero iteran sobre este vector para comprobar si el usuario que solicita la acción está presente en él.

### `std::vector<std::string> invitedClients`
*   **Qué es:** Una lista de *apodos* (`nick`) de los usuarios que han recibido una invitación para entrar al canal.
*   **Uso:** Crucial para el modo `+i` (Invite-Only). Si un canal tiene este modo activo, solo permitirá el `JOIN` si el nick del cliente se encuentra en este vector.

### Variables de Modos Booleanas
*   `bool modeLimit;`
*   `bool modeKey;`
*   `bool modeInvite;`
*   `bool modeTopic;`
*   **Uso:** Banderas simples que determinan si una regla del canal está activada o no. Se modifican a través del comando `MODE`. Si `modeKey` es `true`, el servidor requerirá la contraseña almacenada en `std::string key`. Si `modeLimit` es `true`, el servidor verificará que el tamaño de `clients` no supere a `size_t maxsize`.