# 🌍📡 TCP | UDP Application 📡🌍

**Copyright 2023 - 2024 Stefan Ioana 322CA**

---

## 🚀 About the Application

This is a **client-server** application that enables communication between a server and multiple clients using either **TCP** or **UDP** protocols. The **server** manages connections and messages received from clients, while **clients** can send messages to the server as follows:

- **TCP clients** can **subscribe** or **unsubscribe** to one or more topics. The server will forward messages received from clients to those subscribed to the corresponding topic.
- **UDP clients** can send **messages** on a topic, and the server will forward the received messages to all subscribed clients.

---

## 🛠️ Compilation

```sh
make  
```
- Compiles both the **server** and **client**.

```sh
make clean  
```
- Deletes the compiled executables for the **server** and **client**.

```sh
make server  
```
- Compiles **only** the server.

```sh
make client  
```
- Compiles **only** the client.

---

## ▶️ Running the Application

### 🖥️ **Server**
```sh
./server <PORT>
```

### 🖥️ **Client**
```sh
./client <CLIENT_ID> <SERVER_IP> <SERVER_PORT>
```

---

## ⚙️ Implementation Details

🔹 The **server** creates a **socket** and waits for client connections, while clients connect to the server to send messages.

🔹 The **server** also listens for **UDP messages** from clients.

🔹 The **server** can receive messages from `stdin` to allow manual shutdown of all connections.

🔹 All these sockets, along with client sockets, are monitored using the **`poll()` function** and the `pollfd` structure.

🔹 To establish a connection, **TCP clients** must send a **unique registration ID** to the server. If the ID is already in use, the client cannot connect.

🔹 **TCP clients** can subscribe to one or more topics (**one per message**). These topics can include **wildcards**:
  - `*` → Matches **any number** of topic levels.
  - `+` → Matches **a single** topic level.

---

## 🎮 How to Use

### 🖥️ **TCP Clients**
- ✅ **Subscribe to a topic:**
  ```sh
  subscribe <topic>
  ```
- ❌ **Unsubscribe from a topic:**
  ```sh
  unsubscribe <topic>
  ```
- 🚪 **Disconnect from the server:**
  ```sh
  exit
  ```

### 🖥️ **Server Commands**
- 🔴 **Shutdown the server and disconnect all clients:**
  ```sh
  exit
  ```

---

## 🏁 Final Thoughts

🎯 This project was a **challenging yet rewarding** experience! I learned:

✅ How to use **sockets** for establishing connections between a **server** and multiple **clients**.
✅ How to use **poll()** to efficiently **monitor multiple sockets**.
✅ How to **send and receive messages** between the server and clients.

💡 A particularly tricky part was implementing **topic matching with wildcards**, as ensuring correct pattern validation was quite complex.

🚀 **Happy coding & enjoy the network communication!** 🌍📡🔥
