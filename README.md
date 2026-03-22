*This project has been created as part of the 42 curriculum by jvalle-d, aehrl, sbenitez*

# ft_irc — Internet Relay Chat Server

## Description

ft_irc is an IRC server implementation written in C++98. The goal is to build a functional IRC server that real IRC clients can connect to and interact with, following the IRC protocol standards.

The server handles multiple simultaneous clients using non-blocking I/O with `poll()`, and supports the core features of the IRC protocol: authentication, channels, private messaging, and channel operator commands.

## Instructions

### Requirements

- C++98 compatible compiler (`c++`)
- GNU Make

### Compilation

```bash
make
```

This produces the `ircserv` executable. To clean build files:

```bash
make clean    # remove object files
make fclean   # remove object files and executable
make re       # full rebuild
```

### Execution

```bash
./ircserv <port> <password>
```

- `port`: the port number the server will listen on (1–65535)
- `password`: the connection password required by IRC clients

**Example:**

```bash
./ircserv 6667 mypassword
```

### Connecting with an IRC client

Use any standard IRC client (e.g. HexChat, WeeChat, irssi). Configure it to connect to:

- **Server:** `127.0.0.1` (or your machine's IP)
- **Port:** the port you specified
- **Password:** the password you specified

### Supported commands

| Command | Description |
|---------|-------------|
| `PASS`  | Authenticate with the server password |
| `NICK`  | Set or change nickname |
| `USER`  | Set username and realname |
| `JOIN`  | Join a channel |
| `PART`  | Leave a channel |
| `PRIVMSG` | Send a message to a user or channel |
| `KICK`  | Eject a user from a channel (operator only) |
| `INVITE` | Invite a user to a channel (operator only) |
| `TOPIC` | View or change the channel topic (operator only if +t) |
| `MODE`  | Set channel modes (operator only) |
| `QUIT`  | Disconnect from the server |

### Channel modes

| Mode | Description |
|------|-------------|
| `+i` | Invite-only channel |
| `+t` | Only operators can change the topic |
| `+k` | Channel key (password) |
| `+o` | Give/take channel operator privilege |
| `+l` | Set a user limit on the channel |

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 — IRC Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812)
- [Modern IRC documentation](https://modern.ircdocs.horse/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [poll(2) man page](https://man7.org/linux/man-pages/man2/poll.2.html)

### AI usage

AI tools were used during this project for the following tasks:
- Understanding IRC protocol message formats and numeric reply codes
- Reviewing error handling logic and edge cases in command parsing
- Debugging socket and poll-based I/O patterns

All generated suggestions were reviewed, tested, and fully understood before being integrated into the project.
