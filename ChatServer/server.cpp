#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
#include <string>

using boost::asio::ip::tcp;

class Session; // Forward declaration of the Session class

class ChatServer {
public:
    ChatServer(short port)
        : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

    void run() {
        io_context_.run();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    auto session = std::make_shared<Session>(std::move(socket), clients_);
                    clients_.insert(session);
                    session->start();
                }
                do_accept();
            });
    }

    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::set<std::shared_ptr<Session>> clients_;
};

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, std::set<std::shared_ptr<Session>>& clients)
        : socket_(std::move(socket)), clients_(clients) {
        username_ = "Anonymous"; // Default username
    }

    void start() {
        clients_.insert(shared_from_this());
        do_read(); // Start reading messages
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    // If the username hasn't been set, it is the first message
                    if (username_ == "Anonymous") {
                        username_ = std::string(data_, length); // Set username from first message
                        std::string welcome_msg = username_ + " has joined the chat.\n";
                        broadcast(welcome_msg);
                    } else {
                        // Prepend username to the message
                        std::string message = username_ + ": " + std::string(data_, length);
                        broadcast(message);
                    }
                    do_read(); // Continue reading messages
                } else {
                    clients_.erase(self); // Remove client on error
                }
            });
    }

    void broadcast(const std::string& message) {
        for (auto client : clients_) {
            if (client != shared_from_this()) {
                boost::asio::async_write(client->socket_, boost::asio::buffer(message),
                    [](boost::system::error_code, std::size_t) {});
            }
        }
    }

    tcp::socket socket_;
    std::set<std::shared_ptr<Session>>& clients_;
    char data_[1024];
    std::string username_; // Store the username
};

int main() {
    try {
        ChatServer server(12345);
        server.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
