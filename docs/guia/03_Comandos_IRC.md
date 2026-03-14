# Comandos Soportados por el Servidor ft_irc

Este documento es una referencia de los comandos IRC que el servidor `ft_irc` reconoce, cómo se estructuran y qué hacen. El flujo de enrutamiento comienza en `Server::parseMessage` (dentro de `Server.cpp`).

## A. El Proceso de Parseo (`Server.cpp`)

Cuando `Server::manageClientMessage` lee una línea completa (`\r\n` o `\n` al final) de un cliente, la pasa a `Server::parseMessage`.

1.  **Limpieza:** Se eliminan los saltos de línea finales (`\r` y `\n`).
2.  **División:** La línea completa (que está en `client.getMessage()`) se divide en partes usando espacios (` `) y dos puntos (`:`) como delimitadores usando una función `ft_split` (por ejemplo, `PRIVMSG #canal :Hola mundo` se divide en `["PRIVMSG", "#canal", "Hola mundo"]`). Estas partes se guardan en un vector dentro del cliente: `client.setFullmsg()`.
3.  **Enrutamiento Inicial (Sistema):** Se comprueba si el comando pertenece al conjunto de "comandos de sistema" que no requieren registro completo (`PASS`, `NICK`, `USER`, `CAP`, `QUIT`).
    *   **Si coincide**, se llama a `system_switch` (y luego al correspondiente `parse<Comando>`).
    *   **Envío de Respuesta:** Independientemente de si fue válido o de si generó error, se llama a `client.sendResponse()` para vaciar y enviar el buffer de respuesta que el comando haya generado al usuario por la red.
    *   **Comprobación de Hostname:** Si el usuario tiene `nick` y `user` (es decir, completó la parte inicial del registro), se actualiza su `hostname`.
4.  **Verificación de Registro:** Si el cliente aún **NO** ha completado el registro completo (`client.getUserVerified() == false` - necesita `PASS`, `NICK` y `USER` válidos), el servidor **ignora** cualquier comando que no sea del sistema. Se sale de la función.
5.  **Enrutamiento Final (Usuario):** Si el usuario está verificado, se comprueba si el comando pertenece al conjunto de "comandos de usuario" (`JOIN`, `KICK`, `NICK`, `PART`, `PRIVMSG`, `MODE`, `TOPIC`, `INVITE`, `QUIT`).
    *   **Si coincide**, se llama a `user_switch` (que luego derivará al `parse<Comando>` o al método de `Channel` apropiado).

## B. Referencia de Comandos Soportados

### 1. Comandos de Sistema y Registro

Estos comandos son los primeros que un cliente (como WeeChat o Netcat) debe enviar para establecerse como usuario válido en el servidor.

*   **`CAP` (`_cap.cpp`)**
    *   **Propósito:** Negociación de capacidades del cliente (Capability Negotiation). Se utiliza por clientes modernos para acordar extensiones del protocolo antes de registrarse.
    *   **Implementación:** En `ft_irc`, la implementación de `parseCap` generalmente se limita a confirmar que el servidor soporta las capacidades mínimas y rechazar el resto o simplemente enviar un `CAP * LS` para indicar la lista de capacidades si fuera necesario.
*   **`PASS` (`_pass.cpp`)**
    *   **Uso:** `PASS <contraseña>`
    *   **Propósito:** Proveer la contraseña requerida por el servidor.
    *   **Comportamiento:**
        *   Si el usuario no está verificado aún y la contraseña coincide con la del servidor, marca al cliente como `verified = true`.
        *   Si ya está verificado, devuelve `ERR_ALREADYREGISTERED`.
        *   Si falla, devuelve `ERR_PASSWDMISMATCH` y reduce los reintentos (`retries`) del cliente, desconectándolo si llega a 0.
*   **`NICK` (`_nick.cpp`)**
    *   **Uso:** `NICK <apodo>`
    *   **Propósito:** Establecer o cambiar el apodo del usuario.
    *   **Comportamiento:**
        *   Si el apodo ya existe (`nickInUse`), devuelve `ERR_NICKNAMEINUSE`.
        *   Si contiene caracteres inválidos (`nickInvalidChars`), devuelve `ERR_ERRONEUSNICKNAME`.
        *   Si es válido y el usuario aún no estaba verificado, guarda el `nick`.
        *   Si el usuario ya estaba verificado, se procesa como un *cambio de apodo* (`parseChangeNick`), actualizando su identidad y notificando a otros.
*   **`USER` (`_user.cpp`)**
    *   **Uso:** `USER <username> <hostname> <servername> :<realname>`
    *   **Propósito:** Proveer la información final de identidad del usuario.
    *   **Comportamiento:**
        *   Requiere que el usuario ya tenga un `nick` y esté `verified` (por `PASS`). Si no, falla con error de acceso o `ERR_NOTREGISTERED`.
        *   Si ya estaba registrado, da `ERR_ALREADYREGISTERED`.
        *   Si todo es correcto, marca `username_verified = true`, guarda la info y el usuario se considera formalmente **registrado** en el servidor IRC, habilitando el resto de comandos.
*   **`QUIT` (`_quit.cpp`)**
    *   **Uso:** `QUIT [:<mensaje de salida>]`
    *   **Propósito:** Desconectar al cliente del servidor de forma voluntaria.
    *   **Comportamiento:**
        *   Llama a `disconnectClient` para limpiar la conexión, sacarlo de los canales y cerrar su socket.

### 2. Comandos de Usuario (Canales y Comunicación)

Estos comandos sólo están disponibles para usuarios que ya completaron `PASS`, `NICK` y `USER`.

*   **`JOIN` (`_join.cpp`)**
    *   **Uso:** `JOIN <#canal>[,<#canal2>] [<clave>[,<clave2>]]`
    *   **Propósito:** Entrar a uno o varios canales.
    *   **Comportamiento:**
        *   Si el canal no existe, lo crea y el usuario se convierte en el moderador/operador original.
        *   Si el canal existe, verifica permisos:
            *   Si el canal tiene clave (`+k`), requiere la contraseña correcta (`ERR_BADCHANNELKEY`).
            *   Si el canal tiene límite (`+l`), comprueba que no esté lleno (`ERR_CHANNELISFULL`).
            *   Si el canal es por invitación (`+i`), comprueba si el usuario está en la lista de invitados (`ERR_INVITEONLYCHAN`).
        *   Si entra, envía las notificaciones (`RPL_TOPIC`, lista de usuarios).
*   **`PART` (`_part.cpp`)**
    *   **Uso:** `PART <#canal>[,<#canal2>] [:<razón>]`
    *   **Propósito:** Abandonar uno o varios canales.
    *   **Comportamiento:**
        *   Si no está en el canal, da `ERR_NOTONCHANNEL`.
        *   Lo elimina del canal, avisa a los demás miembros y si el canal queda vacío, puede destruirlo (`deleteChannel`).
*   **`PRIVMSG` (`_privmsg.cpp`)**
    *   **Uso:** `PRIVMSG <objetivo> :<mensaje>`
    *   **Propósito:** Enviar un mensaje a un usuario específico o a un canal completo.
    *   **Comportamiento:**
        *   **A un usuario:** Busca al cliente destino por su `nick` (usando `getClientSocket` o iterando sobre `clients`). Si no lo encuentra: `ERR_NOSUCHNICK`. Si lo encuentra, le envía el mensaje formateado.
        *   **A un canal:** Verifica si el canal existe y si el usuario está en él (o tiene permisos, dependiendo de los modos). Luego envía el mensaje a todos los miembros *excepto* al que lo envió usando `sendResponseChannel`.
*   **`KICK` (`_kick.cpp`)**
    *   **Uso:** `KICK <#canal> <usuario> [:<razón>]`
    *   **Propósito:** Expulsar a la fuerza a un usuario de un canal.
    *   **Comportamiento:**
        *   Requiere que quien lo ejecuta sea moderador del canal (`isAMod` o comprobando la lista `moderators`). Si no: `ERR_CHANOPRIVSNEEDED`.
        *   Si el objetivo no está en el canal: `ERR_USERNOTINCHANNEL`.
        *   Expulsa al usuario y notifica al canal de la acción.
*   **`INVITE` (`_invite.cpp`)**
    *   **Uso:** `INVITE <usuario> <#canal>`
    *   **Propósito:** Invitar a un usuario a un canal, especialmente útil si el canal está en modo de solo invitación (`+i`).
    *   **Comportamiento:**
        *   Si el canal existe y el que invita es miembro (y moderador si es `+i`), busca al usuario destino.
        *   Si el destino ya está en el canal: `ERR_USERONCHANNEL`.
        *   Añade al usuario a la lista de `invitedClients` del canal y le envía una notificación personal de invitación.
*   **`TOPIC` (`_topic.cpp`)**
    *   **Uso:** `TOPIC <#canal> [:<nuevo_tema>]`
    *   **Propósito:** Ver o cambiar el tema (descripción) de un canal.
    *   **Comportamiento:**
        *   Si se proporciona sólo el canal, devuelve el tema actual (`RPL_TOPIC` o `RPL_NOTOPIC`).
        *   Si se proporciona un nuevo tema, intenta cambiarlo.
        *   Si el canal tiene el modo `+t` activado, sólo los moderadores pueden cambiar el tema. Si no tiene permisos: `ERR_CHANOPRIVSNEEDED`.
*   **`MODE` (`_mode.cpp` / `_mode1.cpp`)**
    *   **Uso (Canal):** `MODE <#canal> <+|- ><o|i|t|k|l> [<parámetro>]`
    *   **Propósito:** Cambiar la configuración de un canal.
    *   **Comportamiento:**
        *   Requiere privilegios de operador (`isAMod`). Si no los tiene: `ERR_CHANOPRIVSNEEDED`.
        *   El servidor parsea el signo (`+` activar, `-` desactivar) y la bandera:
            *   **`o` (Operador):** Da o quita privilegios de moderador a otro usuario (`setModeModerator`). *Ej: `MODE #canal +o pepe`*.
            *   **`i` (Invitar):** Activa/Desactiva que el canal sea sólo por invitación (`setModeInvite`). *Ej: `MODE #canal +i`*.
            *   **`t` (Tema):** Activa/Desactiva la restricción de que solo operadores cambien el tema (`setModeTopic`). *Ej: `MODE #canal +t`*.
            *   **`k` (Clave):** Establece o elimina una contraseña para el canal (`setModeKey`). Requiere un parámetro. *Ej: `MODE #canal +k secreto`*.
            *   **`l` (Límite):** Establece o quita un límite máximo de usuarios en el canal (`setModeLimit`). Requiere un número. *Ej: `MODE #canal +l 50`*.