# Flujo Principal del Servidor ft_irc

Este documento describe el ciclo de vida del servidor desde que arranca en `main.cpp` hasta cómo procesa las conexiones usando la función de multiplexación `poll()`.

## 1. Inicialización (`main.cpp`)

1.  **Validación de Argumentos**: Comprueba que se reciben exactamente el puerto y la contraseña (ej. `./ircserv 6667 pass`).
2.  **Manejo de Señales**: Captura `SIGINT` (Ctrl+C) y `SIGQUIT` (Ctrl+\\) para garantizar un cierre ordenado (limpiando `g_signal`).
3.  **Instanciación del Servidor**: Crea el objeto `Server(puerto, contraseña)`. Este proceso de construcción (`Server::Server`) realiza las siguientes tareas de red (`Server.cpp`):
    *   Crea un socket TCP en IPv4 (`AF_INET`, `SOCK_STREAM`).
    *   Le aplica `SO_REUSEADDR` para poder reiniciar el servidor rápidamente tras su caída y sin esperar el timeout del puerto.
    *   Hace el socket NO BLOQUEANTE (`O_NONBLOCK` mediante `fcntl()`). Esto es fundamental para que el servidor no se quede atascado esperando a leer de un único usuario.
    *   Asigna (bind) el socket a cualquier IP disponible (`INADDR_ANY`) en el puerto especificado.
    *   Pone al socket en modo escucha (`listen`).
    *   Inicializa la lista de *polls* con el socket maestro del servidor preparado para leer conexiones entrantes (`POLLIN`).
4.  **Inicialización de Errores**: Rellena un mapa global `errorMessages` con los códigos de error IRC numéricos.

## 2. El Bucle Principal (The Event Loop)

El programa entra en un bucle infinito `while(!g_signal)`. En cada iteración:

1.  **`server.checkConnections()`**: Se llama a la función de la API POSIX `poll()`.
    *   `poll()` toma el vector `polls` (que contiene todos los descriptores de archivo, o sockets, activos) y bloquea la ejecución hasta que haya alguna actividad (nuevos datos, nueva conexión, o desconexión) en cualquiera de ellos.
    *   El timeout `-1` indica que `poll()` esperará indefinidamente a que pase algo antes de retornar.
2.  **`server.updateConnections()`**: Si `poll()` retorna mayor que cero (indicando eventos), se recorre el vector de descriptores `polls` revisando el campo `revents`:
    *   Si hay actividad (`revents > 0`), se verifica **quién** la provocó.

## 3. Manejo de Eventos

Dentro de `updateConnections()`, el servidor distingue entre dos tipos de eventos:

### A. Nueva Conexión (El socket maestro)

*   Si el descriptor que generó la alerta es `this->server_socket`, significa que un nuevo usuario quiere conectarse.
*   Llama a `Server::newClient()`.
    *   Llama a `accept()` para establecer la conexión con el cliente y obtener un nuevo *socket cliente*.
    *   El nuevo socket cliente también se configura como `O_NONBLOCK`.
    *   Se crea un objeto `Client` asociado a este nuevo socket.
    *   Se agrega el nuevo socket al vector `polls` esperando eventos de lectura (`POLLIN`).
    *   Se añade el nuevo objeto `Client` al mapa global `clients` del servidor (indexado por su número de socket).

### B. Mensaje Entrante (Socket de un cliente)

*   Si el descriptor que generó la alerta es de un cliente existente, se llama a `Server::manageClientMessage(client)`.
*   **Lectura**: Usa `recv()` para leer los datos del socket. Como es no bloqueante, leerá lo que esté disponible.
*   **Manejo de Desconexión (`len == 0`)**: Si `recv` devuelve 0, el cliente ha cerrado la conexión (EOF). Se procede a procesar una salida (`parseQuit`).
*   **Fragmentación**: Dado que los mensajes en red pueden llegar por trozos, el servidor concatena los datos recibidos en el buffer interno del cliente (`client.getMessage()`).
*   **Delimitación**: Busca el carácter de nueva línea (`\n`), que en IRC (junto con `\r`) delimita un comando completo.
    *   Por cada línea completa encontrada, se la separa y se pasa a `Server::parseMessage(client)`.
    *   Si queda un fragmento incompleto al final, se mantiene en el buffer del cliente para esperar la siguiente parte en el próximo evento `poll()`.

## 4. El Ciclo de Desconexión

Cuando se identifica que un cliente debe desconectarse (ya sea por error en `recv`, porque cerró la conexión `recv == 0`, o porque envió el comando explícito `QUIT`):

1.  Se limpia cualquier rastro del usuario en el servidor:
    *   Se le elimina de todos los canales a los que estuviera conectado.
    *   Se cierra (mediante `close()`) su descriptor de socket.
    *   Se le borra del vector de monitoreo `polls`.
    *   Se elimina el objeto `Client` del mapa del servidor `clients`.