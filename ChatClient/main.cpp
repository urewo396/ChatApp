#include <iostream>
#include <boost/asio.hpp>
#include <thread>

using boost::asio::ip::tcp;

class ChatClient {
public:
    ChatClient(const std::string& host, const std::string& port)
        : socket_(io_service_) {
        boost::asio::connect(socket_, tcp::resolver(io_service_).resolve(host, port));
    }

    void run() {
        std::thread read_thread([this]() { read_messages(); });
        std::string username;

        // Set the username
        std::cout << "Enter your username: ";
        std::getline(std::cin, username);
        boost::asio::write(socket_, boost::asio::buffer(username + "\n"));

        // Start sending messages
        std::string message;
        while (std::getline(std::cin, message)) {
            boost::asio::write(socket_, boost::asio::buffer(message + "\n"));
        }

        read_thread.join();
    }

private:
    void read_messages() {
        char data[1024];
        while (true) {
            boost::system::error_code error;
            size_t length = socket_.read_some(boost::asio::buffer(data), error);
            if (error) {
                std::cerr << "Error while reading: " << error.message() << "\n";
                break;
            }
            std::cout.write(data, length);
            std::cout << std::endl;
        }
    }

    boost::asio::io_service io_service_;
    tcp::socket socket_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }
        ChatClient client(argv[1], argv[2]);
        client.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
