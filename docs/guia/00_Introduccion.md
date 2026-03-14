# Guía del Proyecto ft_irc

Bienvenido a la documentación del proyecto **ft_irc**. Este proyecto consiste en la creación de un servidor de Internet Relay Chat (IRC) en C++98, capaz de manejar múltiples clientes simultáneamente y canales de comunicación.

## Objetivo del Proyecto

El objetivo principal de `ft_irc` es entender el funcionamiento de las redes de comunicación, los sockets y la multiplexación de entrada/salida no bloqueante usando `poll()`. El servidor debe ser compatible con un cliente de IRC estándar (como WeeChat, Irssi, o Netcat) y cumplir con una serie de características específicas del protocolo IRC.

## Índice de la Guía

Para facilitar la comprensión del proyecto, la documentación se ha dividido en los siguientes archivos:

1. **[01_Arquitectura.md](./01_Arquitectura.md)**: Describe la estructura general del proyecto, las clases principales (`Server`, `Client`, `Channel`) y sus responsabilidades.
2. **[02_Flujo_Principal.md](./02_Flujo_Principal.md)**: Explica el ciclo de vida del servidor, desde su inicio, la aceptación de nuevas conexiones, la recepción de mensajes y el manejo de desconexiones usando `poll()`.
3. **[03_Comandos_IRC.md](./03_Comandos_IRC.md)**: Lista y detalla todos los comandos IRC que el servidor soporta y cómo son parseados y ejecutados.
4. **[04_Variables_Estructuras.md](./04_Variables_Estructuras.md)**: Un vistazo profundo a las estructuras de datos clave utilizadas (vectores, mapas) para gestionar clientes y canales.

## Archivos Clave del Código Fuente

*   `main.cpp`: Punto de entrada, inicialización y bucle principal.
*   `Server.hpp` / `Server.cpp`: Clase principal que maneja la red, las conexiones y el enrutamiento de mensajes.
*   `Client.hpp` / `Client.cpp`: Representación de un usuario conectado al servidor.
*   `Channel.hpp` / `Channel.cpp`: Representación de una sala de chat con sus usuarios, modos y características.
*   `irc.hpp`: Definiciones de códigos de error, macros y librerías comunes.
*   Archivos `_<comando>.cpp`: Implementaciones específicas de cada comando IRC (ej. `_join.cpp`, `_privmsg.cpp`).

## Requisitos de Ejecución

El servidor recibe dos parámetros para su ejecución:
```bash
./ircserv <puerto> <contraseña>
```

*   **puerto**: El puerto en el que el servidor escuchará conexiones entrantes (1-65535).
*   **contraseña**: La clave que cualquier cliente necesitará proporcionar (usando el comando `PASS`) para poder registrarse y usar el servidor.