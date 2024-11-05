#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <set>
#include <string>
using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;


class ChatServer {
public:
    ChatServer(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        start_accept();
    }

private:
    void start_accept() {
        auto new_connection = make_shared<tcp::socket>(acceptor_.get_executor().context());
        acceptor_.async_accept(*new_connection,
            [this, new_connection](const boost::system::error_code& error) {
                handle_accept(new_connection, error);
            });
    }

    void handle_accept(shared_ptr<tcp::socket> new_connection, const boost::system::error_code& error) {
        if (!error) {
            // Store the new connection
            clients_.insert(new_connection);
            start_receive(new_connection);
        }
        start_accept(); // Accept the next connection
    }

    void start_receive(shared_ptr<tcp::socket> client) {
        auto buffer = make_shared<string>(1024, 0); // Buffer for incoming messages
        client->async_receive(boost::asio::buffer(*buffer),
            [this, client, buffer](const boost::system::error_code& error, size_t bytes_transferred) {
                handle_receive(client, buffer, error, bytes_transferred);
            });
    }

    void handle_receive(shared_ptr<tcp::socket> client, shared_ptr<string> buffer,
        const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
            // Broadcast the message to all clients
            string message = *buffer;
            for (auto& c : clients_) {
                if (c != client) {
                    boost::asio::async_write(*c, boost::asio::buffer(message),
                        [](const boost::system::error_code&, size_t) {});
                }
            }
            start_receive(client); // Continue receiving
        }
        else {
            // Handle error or disconnection
            clients_.erase(client);
        }
    }

    tcp::acceptor acceptor_;
    set<shared_ptr<tcp::socket>> clients_; // Set of connected clients
};


int main(int argc, char* argv[]) {
    try {
        boost::asio::io_context io_context;
        short port = 12345; // Choose a port number
        ChatServer server(io_context, port);
        io_context.run(); // Start the server
    }
    catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
