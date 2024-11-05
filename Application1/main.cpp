#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <string>

using boost::asio::ip::tcp;

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
                    std::make_shared<Session>(std::move(socket), clients_)->start();
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
        clients_.insert(shared_from_this());
    }

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    do_send(length);
                } else {
                    clients_.erase(self);
                }
            });
    }

    void do_send(std::size_t length) {
        for (auto client : clients_) {
            if (client != shared_from_this()) {
                boost::asio::async_write(client->socket_, boost::asio::buffer(data_, length),
                    [](boost::system::error_code, std::size_t) {});
            }
        }
        do_read();
    }

    tcp::socket socket_;
    std::set<std::shared_ptr<Session>>& clients_;
    char data_[1024];
};

int main() {
    try {
        ChatServer server(12345);
        server.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
