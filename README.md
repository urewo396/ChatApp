# Chat Application

A simple chat application built using C++ and Boost.Asio that allows multiple clients to connect to a server and exchange messages in real-time. This application supports username functionality, enabling users to identify themselves in the chat.

## Features

- **Real-Time Messaging**: Clients can send and receive messages instantly.
- **Username Support**: Users can set a username when they connect to the chat.
- **Broadcast Messages**: Messages are broadcasted to all connected clients.
- **Asynchronous I/O**: Utilizes Boost.Asio for efficient network communication.

## Requirements

- C++17 or later
- [Boost Libraries](https://www.boost.org/) (specifically Boost.Asio)
- A compatible C++ compiler (e.g., g++)

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/YOUR_USERNAME/chat_application.git
   cd chat_application
2. Install Boost: Make sure you have Boost installed on your system. (Boost version for this project is 1.86.0)

3. Start the server using ```./server```
4. Join with client using ```./client <ip> <port>```
5. Set your nickname which will be visable for others and chat with each other!
   
