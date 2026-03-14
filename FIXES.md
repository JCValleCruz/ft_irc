# FIXES.md — ft_irc correcciones y notas de estudio

---

## FIX 1 — El servidor no se bindea a ninguna interfaz

**Archivo:** `Server.cpp`
**Función:** `initServerAddress()`

### Problema

`INADDR_ANY` estaba comentado durante experimentos con la red local, dejando
`address.sin_addr` sin inicializar. El socket del servidor se creaba y se
bindeaba pero no escuchaba en ninguna interfaz real — ningún cliente podía
conectarse.

### Diff

```diff
-    //address.sin_addr.s_addr = INADDR_ANY;
-/* //this->ip_address = "10.11.5.4";
-    if(inet_pton(AF_INET, ip_address.c_str(), &address.sin_addr) <= 0)
-        errorPrint("Invalid IP address format"); */
+    address.sin_addr.s_addr = INADDR_ANY;
```

### Explicación

`INADDR_ANY` (valor 0) le indica al kernel que se bindee a todas las
interfaces de red disponibles en la máquina. Sin él, `sin_addr` queda
inicializado a cero por `memset` — casualmente el mismo valor — pero la
intención se perdía y el código experimental de IP específica era código
muerto que podría haber causado problemas si se descomenntaba.

---

## FIX 2 — poll() en busy loop (100% de CPU)

**Archivo:** `Server.cpp`
**Función:** `checkConnections()`

### Problema

`poll()` se llamaba con timeout `0`, lo que hace que retorne inmediatamente
sin importar si algún fd tiene actividad. Combinado con el bucle
`while(!g_signal)` en `main()`, el proceso giraba al 100% de CPU sin hacer
nada útil cuando no había clientes conectados.

El subject además indica que leer/escribir sin un poll previo da nota 0 —
usar timeout 0 anula el propósito de poll como puerta bloqueante.

### Diff

```diff
-    int result = poll(&this->polls[0], polls.size(), 0);
+    int result = poll(&this->polls[0], polls.size(), -1);
```

### Explicación

Con `-1` como timeout, `poll()` se bloquea indefinidamente hasta que al
menos un fd tenga un evento (nueva conexión, datos disponibles, socket
cerrado). El proceso duerme sin consumir CPU mientras está inactivo y solo
despierta cuando hay trabajo real que hacer.

---

## FIX 3 — Bucle infinito en getChannel()

**Archivo:** `Server_Utils.cpp`
**Función:** `getChannel()`

### Problema

El bucle `while` buscaba un canal por nombre pero nunca incrementaba `i`.
Si el canal no estaba en el índice 0, el bucle giraba para siempre
comparando `channels[0]` contra el nombre buscado.

### Diff

```diff
 while(i < this->channels.size())
 {
     if(this->channels[i].getName() == chaname)
         break;
+    i++;
 }
```

### Explicación

Sin `i++`, la variable del bucle nunca avanza. Si `channels[0]` no es el
buscado, la condición `channels[0].getName() == chaname` es siempre false
y el bucle nunca termina.

---

## FIX 4 — Paquetes fragmentados no se ensamblan correctamente

**Archivo:** `Server.cpp`
**Función:** `manageClientMessage()`

### Problema

Los clientes IRC (y herramientas como `nc`) pueden enviar comandos divididos
en múltiples paquetes TCP. El subject lo prueba explícitamente:

```
$> nc 127.0.0.1 6667
com^Dman^Dd\n
```

La lógica anterior tenía tres ramas separadas:
- 2 o más saltos de línea → dividir y procesar cada uno
- exactamente 1 salto de línea → concatenar buffer + procesar
- ningún salto de línea → acumular

El fallo: cuando un comando llega dividido como
`"PASS testpass\r\nNICK fra"` + `"gmented\r\nUSER ...\r\n"`,
el segundo recv contiene 2 saltos de línea. Entra por la primera rama que
divide el nuevo chunk solo — descartando cualquier fragmento acumulado
previamente en `client.getMessage()` porque llama a
`ft_split(message, '\n', 0)` sobre el chunk nuevo, no sobre
`client.getMessage() + message`.

### Solución

Reemplazar las tres ramas por un único flujo unificado: siempre anteponer
el buffer acumulado del cliente al nuevo chunk, luego extraer comandos
completos uno a uno, dejando cualquier fragmento incompleto en el buffer
para el siguiente recv.

### Diff

```diff
-	std::string message(buffer);
-	if (len > 0)
-	{
-		if(count_char(message, '\n') >= 2)
-		{
-			std::vector<std::string> temp = ft_split(message, '\n', 0);
-			size_t i = 0;
-			while(i < temp.size())
-			{
-				client.setMessage(temp[i]);
-				parseMessage(client);
-				client.setMessage("");
-				i++;
-			}
-		}
-		else if(message.find("\n") != std::string::npos)
-		{
-			client.setMessage(client.getMessage() + message);
-			parseMessage(client);
-			client.setMessage("");
-		}
-		else if(message.find("\n") == std::string::npos)
-		{
-			client.setMessage(client.getMessage() + message);
-			message = "";
-		}
-	}
+	if (len > 0)
+	{
+		std::string accumulated = client.getMessage() + std::string(buffer);
+		size_t pos;
+		while ((pos = accumulated.find('\n')) != std::string::npos)
+		{
+			client.setMessage(accumulated.substr(0, pos + 1));
+			parseMessage(client);
+			accumulated = accumulated.substr(pos + 1);
+		}
+		client.setMessage(accumulated);
+	}
```

### Explicación

El código anterior tenía tres ramas basadas en cuántos `\n` había en el
nuevo chunk solo — nunca consideraba que `client.getMessage()` pudiera
contener ya un fragmento de comando de un recv anterior. El nuevo código
siempre fusiona primero y luego procesa líneas completas en bucle. Lo que
queda sin `\n` se mantiene en el buffer del cliente para el siguiente recv.

**Estado: implementado y testeado**

---

## Bugs detectados durante los tests (pendiente de corrección)

Descubiertos lanzando conexiones simultáneas contra el servidor y comparando
las respuestas esperadas con las del protocolo IRC.

---

## FIX 5 — PRIVMSG usuario-a-usuario sin prefijo ni hostname

**Archivo:** `_privmsg.cpp`
**Función:** `parsePrivmsg()`

### Problema

El PRIVMSG de usuario a usuario construía el mensaje usando solo el nick del
emisor sin el prefijo `:` ni el hostname completo. El protocolo IRC exige que
todo mensaje tenga el formato `:<nick>!<user>@<host>` como prefijo para que
el cliente receptor sepa quién lo envía.

```
recibido:  alice PRIVMSG bob :hey bob
esperado:  :alice!alice@localhost PRIVMSG bob :hey bob
```

La rama de canal (unas líneas más abajo en el mismo archivo) ya lo hacía
correctamente con `":" + client.getHostname()`. Solo la rama de usuario
a usuario usaba `client.getNick()` sin prefijo.

### Diff

```diff
-                    response += client.getNick() + " PRIVMSG " + name_vec[i];
+                    response += ":" + client.getHostname() + " PRIVMSG " + name_vec[i];
                     response += " " + client.getFullmsg()[2] + "\r\n";
```

### Explicación

En el protocolo IRC (RFC 1459), todos los mensajes que el servidor retransmite
a los clientes deben llevar el prefijo del origen con el formato:

```
:<nick>!<user>@<host> COMANDO destino :texto
```

`client.getHostname()` devuelve exactamente esa cadena completa
(`nick!user@host`). Sin el prefijo `:`, los clientes IRC (irssi, weechat, etc.)
no reconocen la línea como un mensaje válido y la descartan o muestran de forma
incorrecta.

**Estado: implementado y testeado**

---

### BUG A — ~~PRIVMSG sin prefijo `:`~~ → corregido como FIX 5

---

## FIX 6 — MODE con múltiples grupos de flags y key faltante en respuesta +k

**Archivo:** `_mode.cpp`, `_join.cpp`
**Función:** `parseMode()`, `joinChannel()`

### Problema

Dos bugs relacionados con el manejo de modos de canal:

**Parte A — BUG C: múltiples grupos de flags**

El parser de MODE solo leía `fullmsg[2]` como el string de modos. Si el cliente
enviaba `MODE #test -i -k` (dos grupos separados), `fullmsg[2]` era `"-i"` y
`fullmsg[3]` era `"-k"`. El código trataba `fullmsg[3...]` como argumentos de
flags, no como grupos de modos adicionales. Resultado: `-i` se procesaba, `-k`
se ignoraba.

```
enviado:   MODE #test -i -k
recibido:  :alice!... MODE #test -i   (solo -i, -k ignorado)
esperado:  :alice!... MODE #test -i -k
```

**Parte B — BUG B: key no aparece en respuesta MODE +k**

Al procesar `+k`, el servidor llamaba `setModeKey()` correctamente pero no
añadía el valor de la key al string de respuesta. Para `+l` y `+o` sí se hacía
(`response += " " + cmap['l']`), pero para `+k` faltaba esa línea.

```
enviado:   MODE #test +k secretkey
recibido:  :alice!... MODE #test +k            (falta el valor)
esperado:  :alice!... MODE #test +k secretkey
```

**Parte C — BUG B en JOIN: código 461 en lugar de 475**

En `_join.cpp`, cuando el canal tiene `+k` y el cliente intenta entrar sin
proporcionar ninguna key (vector `key` vacío), la condición
`x + 1 > key.size()` era verdadera (1 > 0) y lanzaba `ERR_NEEDMOREPARAMS`
(461). El código correcto del protocolo IRC es `ERR_BADCHANNELKEY` (475).

```
recibido:  461 Not enough parameters
esperado:  475 Cannot join channel (+k)
```

### Nota sobre BUG D

El bug reportado "canal lleno devuelve 461 en lugar de 471" era un efecto
secundario de BUG C. Si `MODE #test -i -k` no quitaba `+k`, cuando eve
intentaba entrar sin key, caía en el BUG B de JOIN (461 en lugar de 475).
El código de canal lleno (`ERR_CHANNELISFULL = 471`) ya era correcto.

### Diff — `_mode.cpp`

```diff
-            std::vector<std::string> args;
-            for(size_t i = 3; i < client.getFullmsg().size(); i++)
-                args.push_back(client.getFullmsg()[i]);
-
-            std::map<char, std::string> cmap;
-            std::string commands = client.getFullmsg()[2];
-            char sign = commands[0];
-            for(size_t i = 1; i < commands.size(); i++)
-            {
-                cmap[commands[i]] = "";
-                if(args.size() > 0)
-                {
-                    if((commands[i] == 'o') || (sign == '+' && (commands[i] == 'k' || commands[i] == 'l')))
-                    {
-                        cmap[commands[i]] = args[0];
-                        args.erase(args.begin());
-                    }
-                }
-            }
-            int i = 1;
-            response += client.getHostname() + " MODE " + chan.getName() + " " + commands;
-            int flag = 1;
-            while(commands[i])
-            {
-                ...
-                else if(commands[i] == 'k')
-                {
-                    if(cmap['k'] == "" && sign == '+')
-                        throw(ERR_NEEDMOREPARAMS);
-                    setModeKey(sign, chan, cmap['k']);
-                    // (no había response += para la key)
-                }
-                ...
-            }
+            std::vector<std::string> mode_tokens;
+            std::vector<std::string> args;
+            for(size_t i = 2; i < client.getFullmsg().size(); i++)
+            {
+                if(client.getFullmsg()[i][0] == '+' || client.getFullmsg()[i][0] == '-')
+                    mode_tokens.push_back(client.getFullmsg()[i]);
+                else
+                    args.push_back(client.getFullmsg()[i]);
+            }
+            response += client.getHostname() + " MODE " + chan.getName();
+            int flag = 1;
+            for(size_t mt = 0; mt < mode_tokens.size(); mt++)
+            {
+                std::map<char, std::string> cmap;
+                std::string commands = mode_tokens[mt];
+                char sign = commands[0];
+                // ... (asignación de args igual que antes)
+                response += " " + commands;
+                // bucle while igual que antes, más:
+                else if(commands[i] == 'k')
+                {
+                    if(cmap['k'] == "" && sign == '+')
+                        throw(ERR_NEEDMOREPARAMS);
+                    setModeKey(sign, chan, cmap['k']);
+                    if(sign == '+')
+                        response += " " + cmap['k'];  // <- añadido
+                }
+            }
```

### Diff — `_join.cpp`

```diff
-                               else if(x + 1 > key.size())
-                                   throw ERR_NEEDMOREPARAMS;
+                               else if(x + 1 > key.size())
+                                   throw ERR_BADCHANNELKEY;
```

### Explicación

**BUG C**: La solución separa la lista de argumentos en dos vectores: los
"grupos de modos" (tokens que empiezan con `+` o `-`) y los "argumentos de
modos" (el resto). Luego itera sobre cada grupo de modos con un bucle externo,
reutilizando la misma lógica de asignación de argumentos. Los argumentos se
consumen en orden entre todos los grupos.

**BUG B (mode)**: Para `+k`, `+l` y `+o` ya se añadían sus argumentos al
response. A `+k` le faltaba esa línea. Se añade `response += " " + cmap['k']`
condicionado a `sign == '+'` porque al quitar la key (`-k`) no hay valor que
mostrar.

**BUG B (join)**: Cuando el canal tiene `+k` y no se proporciona key, el
vector `key` está vacío. La condición original lanzaba 461 "falseando" que
faltaban parámetros, cuando en realidad el cliente no tiene derecho a entrar
porque no conoce la contraseña → 475.

**Estado: implementado y compilado**

---

### BUG B — ~~MODE +k / JOIN incorrecto~~ → corregido como FIX 6
### BUG C — ~~MODE múltiples flags~~ → corregido como FIX 6
### BUG D — ~~461 en lugar de 471~~ → era efecto secundario de BUG C, corregido como FIX 6

### BUG E — Buffer del cliente no se vacía entre operaciones

**Archivo:** `Server.cpp`, `manageClientMessage()`

Los eventos que llegan mientras un cliente está inactivo se acumulan en el
buffer del socket. Cuando el cliente envía su siguiente comando, `recv()`
devuelve todos los datos acumulados de golpe, mezclando notificaciones de
eventos anteriores con la respuesta al nuevo comando. Relacionado con FIX 4.

---

## FIX 7 — INVITE sin prefijo `:` y enviado al canal en lugar de al usuario

**Archivo:** `_invite.cpp`
**Función:** `parseInvite()`

### Problema

Dos fallos en la misma línea de construcción del mensaje INVITE:

**Falta prefijo `:`** — igual que el BUG A de PRIVMSG. El mensaje no llevaba
el `:` inicial, por lo que los clientes IRC no lo reconocían como mensaje
válido con prefijo de origen.

**Enviado a todo el canal** — `sendResponseChannel()` manda el mensaje a todos
los usuarios del canal. El INVITE debe enviarse únicamente al usuario invitado,
no al resto del canal.

```
recibido (por todos en el canal):  alice!alice@localhost INVITE charlie #test
esperado (solo charlie recibe):    :alice!alice@localhost INVITE charlie #test
```

### Diff

```diff
-		std::string response = client.getHostname() + " INVITE " + client.getFullmsg()[1] + " " + temp.getName();
-        temp.setResponse(response);
-        temp.sendResponseChannel(response, client, 0);
+		std::string response = ":" + client.getHostname() + " INVITE " + client.getFullmsg()[1] + " " + temp.getName();
+        temp.setResponse(response);
+        send(getClientSocket(client.getFullmsg()[1]), (response + "\r\n").c_str(), response.size() + 2, 0);
```

### Explicación

El prefijo `:` indica que lo que sigue es el origen del mensaje en formato
`nick!user@host`. Sin él, el parser del cliente rechaza la línea.

El cambio de `sendResponseChannel` a `send` directo sobre el socket del
usuario invitado (`getClientSocket(client.getFullmsg()[1])`) garantiza que
solo charlie recibe la invitación. El `temp.setResponse` se mantiene para
que el canal tenga registro del último mensaje, sin efecto visible.

**Estado: implementado y compilado**

---

## FIX 8 — TOPIC y RPL_TOPIC sin `:` antes del texto

**Archivo:** `_topic.cpp`
**Funciones:** `rplTopic()`, `parseTopic()`

### Problema

Las respuestas que incluyen el texto del topic no ponían `:` antes del
contenido. En el protocolo IRC, el último parámetro de un mensaje que puede
contener espacios debe ir precedido de `:` para que el parser del cliente sepa
que todo lo que sigue es un único token.

```
recibido:  :alice!... TOPIC #test welcome to test channel
recibido:  :AsusTUF 332 alice #test welcome to test channel
esperado:  :alice!... TOPIC #test :welcome to test channel
esperado:  :AsusTUF 332 alice #test :welcome to test channel
```

### Diff

```diff
- response = ":" + client.getHostname() + " 332 " + client.getNick() + " " + chan.getName() + " " + chan.getTopic();
+ response = ":" + client.getHostname() + " 332 " + client.getNick() + " " + chan.getName() + " :" + chan.getTopic();

- response = ":" + client.getHostname() + " TOPIC " + temp.getName() + " " + temp.getTopic();
+ response = ":" + client.getHostname() + " TOPIC " + temp.getName() + " :" + temp.getTopic();
```

### Explicación

El topic se almacena en el canal sin el `:` (se elimina al guardar en
`setTopic`). Al construir la respuesta hay que reponerlo. Afectaba a tres
sitios: la función `rplTopic()` (RPL_TOPIC 332 al consultar) y dos ramas de
`parseTopic()` (TOPIC al establecer con longitud normal y con truncado).

**Estado: implementado y compilado**

---

## FIX 9 — Límite de 9 caracteres en username rechaza nombres válidos

**Archivo:** `_user.cpp`
**Función:** `parseUser()`

### Problema

El servidor rechazaba cualquier username con más de 9 caracteres lanzando
`ERR_USERLEN` (código 1234, inventado — no existe en el protocolo IRC).
El username "fragmented" (10 chars) de la prueba de paquetes fragmentados
del subject recibía ese error y no podía autenticarse.

```
recibido:  :AsusTUF 1234 fragmented :Username too long
esperado:  (sin error, autenticación completada)
```

El RFC 1459 y el RFC 2812 no especifican un límite máximo de longitud para
el campo username. El límite de 9 era una restricción inventada sin base en
el estándar.

### Diff

```diff
-		else if(fullmsg[1].length() > 9)
-			throw(ERR_USERLEN);
-		else if(fullmsg[3] != "*" || fullmsg[2] != "0" || fullmsg[4][0] == '\0')
+		else if(fullmsg[3] != "*" || fullmsg[2] != "0" || fullmsg[4][0] == '\0')
```

### Explicación

Se elimina la comprobación completa. No hay ningún motivo de protocolo para
imponer ese límite. El campo username solo necesita ser no vacío y no contener
caracteres inválidos, comprobación que el resto de la función ya cubre
implícitamente.

**Estado: implementado y compilado**
