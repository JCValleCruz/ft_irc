# Cómo funciona el parsing IRC en este servidor

## El protocolo IRC: mensajes y parámetros

En IRC, cada mensaje tiene esta forma:

```
COMANDO param1 param2 :trailing param con espacios\r\n
```

La regla clave es el **`:`**: cuando aparece al principio de un parámetro, indica que ese parámetro es "trailing" — puede contener espacios y se extiende hasta el final del mensaje.

Ejemplos reales:
```
PASS pass               → password sin trailing
PASS :pass              → password con trailing (algunos clientes hacen esto)
USER gacel 8 * :gacel   → realname con trailing (puede tener espacios: :Juan García)
PRIVMSG #canal :Hola mundo con espacios
```

---

## `ft_split`: el parser de este proyecto

La función está en `Server_Utils.cpp` y tiene esta firma:

```cpp
std::vector<std::string> ft_split(std::string str, char skip, char stop);
//                                                        ^          ^
//                                                      ' '        ':'
```

Se llama siempre así desde `parseMessage`:

```cpp
client.setFullmsg(ft_split(message, ' ', ':'));
```

### Cómo funciona paso a paso

```cpp
std::vector<std::string> ft_split(std::string str, char skip, char stop)
{
    // Divide por espacios ('skip')
    // Cuando encuentra ':', mete todo lo que queda (incluyendo el ':') como último elemento
}
```

Con `PASS :pass`:

```
str = "PASS :pass"
        ^^^^^  → "PASS" → split[0]
              ^^^^^  → encuentra ':' en ":pass"
                    → extract = "" (vacío, no se añade)
                    → remaining = str.substr( pos de ':' ) = ":pass"
                    → split[1] = ":pass"   ← el ':' SE INCLUYE
```

Con `USER gacel 8 * :gacel`:

```
str = "USER gacel 8 * :gacel"
split[0] = "USER"
split[1] = "gacel"
split[2] = "8"
split[3] = "*"
       → encuentra ':' en ":gacel"
split[4] = ":gacel"   ← el ':' SE INCLUYE
```

**Conclusión: `ft_split` conserva el `:` en el parámetro trailing.**

---

## Por qué esto causaba bugs

### Bug 1: PASS fallaba siempre con clientes como irssi/weechat

El cliente enviaba `PASS :pass` (con `:`). El servidor comparaba:

```cpp
fullmsg[1] == this->password
// ":pass" == "pass"  → false → 464 Password incorrect
```

El fix: quitar el `:` antes de comparar.

```cpp
std::string passArg = fullmsg[1];
if (!passArg.empty() && passArg[0] == ':')
    passArg = passArg.substr(1);
if (passArg == this->password)   // "pass" == "pass" → correcto
```

### Bug 2: USER rechazaba modos válidos

El código original validaba:

```cpp
else if(fullmsg[3] != "*" || fullmsg[2] != "0" || fullmsg[4][0] == '\0')
    throw ERR_NEEDMOREPARAMS;
```

El campo `fullmsg[2]` es el **modo de usuario** (RFC 2812: `USER <user> <mode> <unused> <realname>`). El RFC dice que el cliente puede enviar cualquier número aquí. `8` es un modo válido (significa invisible + wallops), pero el código solo aceptaba `"0"`.

El fix: no validar campos que el RFC no restringe.

```cpp
else if(fullmsg[4].empty())
    throw ERR_NEEDMOREPARAMS;
```

### Bug 3 (cascada): NICK y USER devolvían 451 sin razón

Porque PASS fallaba → `client.getVerify()` quedaba `false` → `parseNick` y `parseUser` tiraban `ERR_NOTREGISTERED` (451) antes de hacer nada.

Un solo bug en PASS se propagaba a todos los comandos siguientes del handshake.

---

## El handshake IRC: orden correcto

Para que un cliente quede registrado, debe completar esta secuencia:

```
Cliente → Servidor

CAP LS 302          (negociación de capacidades, opcional)
PASS <password>     (contraseña del servidor)
NICK <nick>         (elegir apodo)
USER <user> <mode> <unused> :<realname>   (registrar usuario)
```

Solo cuando `NICK` y `USER` se completan con éxito el servidor envía el RPL_WELCOME (001) y el cliente queda "registrado". Cualquier comando de chat (JOIN, PRIVMSG, etc.) antes de ese momento devuelve 451.

En este servidor:
- `client.getVerify()` → true después de PASS correcto
- `client.getNick()` → no vacío después de NICK
- `client.getUserVerified()` → true después de USER (cuando setUsername se llama)
- Solo cuando `getNick() != ""` y `getUsername() != ""` se activan los comandos de usuario

---

## Dónde mirar cuando aparece un error inesperado

| Error | Código | Qué revisar |
|-------|--------|-------------|
| 451 You have not registered | ERR_NOTREGISTERED | ¿PASS se procesó correctamente? ¿`getVerify()` es true? |
| 461 Not enough parameters | ERR_NEEDMOREPARAMS | ¿La validación de `fullmsg.size()` o de sus campos es demasiado estricta? |
| 464 Password incorrect | ERR_PASSWDMISMATCH | ¿Se está comparando con `:` incluido? |
| 462 Already registered | ERR_ALREADYREGISTERED | El cliente mandó PASS/USER dos veces |

Tip para debuggear: el `printVector` ya en el código muestra el vector completo. Si ves `:pass` en lugar de `pass`, es el problema del trailing.
