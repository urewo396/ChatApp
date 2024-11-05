#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <deque>

using boost::asio::ip::tcp;

class ChatClient {
public:
    ChatClient(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints)
        : socket_(io_context) {
        do_connect(endpoints);
    }

    void write(const std::string& message) {
        boost::asio::post(socket_.get_executor(),
            [this, message]() {
                bool write_in_progress = !write_msgs_.empty();
                write_msgs_.push_back(message + "\n");
                if (!write_in_progress) {
                    do_write();
                }
            });
    }

    void close() {
        boost::asio::post(socket_.get_executor(),
            [this]() { socket_.close(); });
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints) {
        boost::asio::async_connect(socket_, endpoints,
            [this](std::error_code ec, tcp::endpoint) {
                if (!ec) {
                    do_read();
                }
            });
    }

    void do_read() {
        boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_msg_), "\n",
            [this](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "Server: " << read_msg_.substr(0, length);
                    read_msg_.erase(0, length);
                    do_read();
                }
                else {
                    socket_.close();
                }
            });
    }

    void do_write() {
        auto message = write_msgs_.front();
        boost::asio::async_write(socket_,
            boost::asio::buffer(message.data(), message.length()),
            [this](std::error_code ec, std::size_t) {
                if (!ec) {
                    write_msgs_.pop_front();
                    if (!write_msgs_.empty()) {
                        do_write();
                    }
                }
                else {
                    socket_.close();
                }
            });
    }

    tcp::socket socket_;
    std::string read_msg_;
    std::deque<std::string> write_msgs_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: ChatClient <host> <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(argv[1], argv[2]);

        ChatClient client(io_context, endpoints);
        std::thread t([&io_context]() { io_context.run(); });

        std::string message;
        while (std::getline(std::cin, message)) {
            client.write(message);
        }

        client.close();
        t.join();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
