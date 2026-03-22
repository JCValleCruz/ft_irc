# Bot IRC - Documentación de Implementación

## Resumen

Se ha implementado un bot IRC integrado (`IRCBot`) que funciona como un cliente visible en los canales. El bot responde a comandos específicos y se gestiona automáticamente:
- Se une automáticamente a cada canal nuevo creado
- Se va automáticamente cuando queda solo en un canal
- Está protegido contra kicks
- Responde a 4 comandos: `!help`, `!ping`, `!info`, `!users`

## Archivos Creados

### Bot.hpp
Declara el namespace `Bot` con todas las funciones necesarias para el manejo de comandos:
```cpp
namespace Bot {
    void handleCommand(Client &client, Server &server);
    void cmd_help(Client &client, Server &server, const std::string &channelName);
    void cmd_ping(Client &client, Server &server, const std::string &channelName);
    void cmd_info(Client &client, Server &server, const std::string &channelName);
    void cmd_users(Client &client, Server &server, const std::string &channelName);
    bool isCommand(const std::string &msg);
    std::string getCommand(const std::string &msg);
    std::string getChannelFromMessage(Client &client);
    void sendToChannel(Server &server, const std::string &channelName, const std::string &message);
}
```

### Bot.cpp
Implementa toda la lógica del bot:
- **`handleCommand()`**: Punto de entrada que detecta y ejecuta comandos
- **`cmd_*`**: Implementaciones específicas de cada comando
- **`sendToChannel()`**: Envía mensajes al canal como `:IRCBot!bot@hostname PRIVMSG #channel :message`

## Archivos Modificados

### Server.hpp
**Línea 29**: `Client* botClient;` - Puntero al cliente bot
**Línea 35**: `void initBotClient();` - Función de inicialización
**Línea 39**: `~Server();` - Destructor declarado
**Líneas 81-82**:
- `Client* getBotClient();`
- `void checkBotShouldLeave(Channel &channel);`

### Server.cpp
**Constructor**: Llama a `initBotClient()` después de `initHostName()`

**Destructor añadido**:
```cpp
Server::~Server() {
    delete botClient;
}
```

**Función `initBotClient()` añadida**:
```cpp
void Server::initBotClient() {
    sockaddr_in fake_addr;
    memset(&fake_addr, 0, sizeof(fake_addr));
    botClient = new Client(-1, fake_addr);
    botClient->setNick("IRCBot");
    botClient->setUsername("bot");
    botClient->setVerify(true);
    std::string botHostname = "IRCBot!bot@" + this->hostname;
    botClient->setHostnameManual(botHostname);
}
```

**Función `checkBotShouldLeave()` añadida**:
Elimina el bot del canal si queda solo (solo 1 usuario = el bot).

### _join.cpp
**Líneas 40-44**: Auto-añade el bot a canales nuevos
```cpp
int chanIndex = findChannelNumber(names[i]);
this->channels[chanIndex].addClient(*botClient);
std::string botJoin = ":IRCBot!bot@" + this->hostname + " JOIN " + names[i] + "\r\n";
this->channels[chanIndex].sendResponseChannel(botJoin, *botClient, 0);
```

### _privmsg.cpp
**Línea 14**: `#include "Bot.hpp"`

**Líneas 31-35**: Detección de comandos
```cpp
if (client.getFullmsg()[2].size() > 1 && client.getFullmsg()[2][1] == '!') {
    Bot::handleCommand(client, *this);
    return;
}
```

### _part.cpp
**Línea 37**: `checkBotShouldLeave(this->channels[n]);`

### _quit.cpp
**Línea 50**: `checkBotShouldLeave(this->channels[i]);`

### _kick.cpp
**Líneas 28-29**: Protección del bot
```cpp
if(fullmsg[2] == "IRCBot")
    throw (ERR_CHANOPRIVSNEEDED);
```

**Línea 39**: `checkBotShouldLeave(this->channels[n]);`

### Client.cpp
**`setHostname()` modificada**: Añadido NULL check
```cpp
void Client::setHostname() {
    if (this->host == NULL)
        return;
    this->hostname = this->nick + "!" + this->user + "@" + this->host->h_name;
}
```

**Nueva función `setHostnameManual()`**:
```cpp
void Client::setHostnameManual(std::string hostname) {
    this->hostname = hostname;
}
```
*Necesaria porque el bot (socket=-1) no puede hacer lookup DNS.*

### Client.hpp
Añadida declaración: `void setHostnameManual(std::string hostname);`

### Makefile
`Bot.cpp` añadido a la variable `SRCS`

## Cómo Funciona

### Flujo de Auto-Join (Canales Nuevos)
1. Usuario crea canal con `/join #nuevo`
2. En `_join.cpp` se detecta que el canal no existe
3. Se crea el canal y se añade al creador como moderador
4. **Automáticamente** se añade el bot al canal
5. Se envía mensaje JOIN del bot a todos en el canal

### Flujo de Comandos
1. Usuario escribe `!help` en el canal
2. PRIVMSG llega a `_privmsg.cpp`
3. Se detecta `!` en posición `[2][1]` del mensaje
4. Se llama a `Bot::handleCommand()`
5. Se parsea el comando y se ejecuta la función correspondiente
6. El bot responde al canal con `sendToChannel()`

### Flujo de Auto-Leave
1. Usuario hace `/part #canal` o `/quit` o `/kick usuario`
2. En `_part.cpp`, `_quit.cpp` o `_kick.cpp` se llama a `checkBotShouldLeave()`
3. Si solo queda el bot (`getClients().size() == 1`), se elimina del canal
4. Si el canal queda vacío, se destruye el canal completamente

## Comandos Disponibles

| Comando | Descripción |
|---------|-------------|
| `!help` | Muestra la lista de comandos disponibles |
| `!ping` | Responde con "Pong!" |
| `!info` | Muestra información sobre el servidor IRC |
| `!users` | Lista todos los usuarios en el canal actual |

## Características Técnicas

- **Socket especial**: El bot usa `socket = -1` como marcador de cliente virtual
- **Hostname manual**: `IRCBot!bot@hostname` establecido manualmente (no puede hacer DNS lookup)
- **Namespace en vez de clase**: Más simple para funciones estáticas que no necesitan estado
- **Protección contra kicks**: Lanza error `ERR_CHANOPRIVSNEEDED` si intentan kickear al bot
- **Gestión automática**: Se une y sale de canales sin intervención manual

## Debugging

Si el bot aparece como "ente vacío":
- Verificar que `setHostnameManual()` se llama en `initBotClient()`
- Verificar que `setHostname()` tiene el NULL check
- En logs ver que aparece `IRCBot!bot@hostname JOIN #canal`, no `:: JOIN #canal`

Si los comandos no responden:
- Verificar que el include `#include "Bot.hpp"` está en `_privmsg.cpp`
- Verificar que la detección está **después** de las validaciones de errores
- Verificar que el mensaje tiene formato correcto: `PRIVMSG #canal :!comando`

## Estadísticas de Implementación

- **Archivos nuevos**: 2 (Bot.hpp, Bot.cpp)
- **Archivos modificados**: 9 (Server.hpp, Server.cpp, Client.hpp, Client.cpp, _join.cpp, _privmsg.cpp, _part.cpp, _quit.cpp, _kick.cpp, Makefile)
- **Líneas de código nuevas**: ~150 líneas
- **Funciones nuevas**: 13 funciones en namespace Bot + 3 métodos en Server/Client
