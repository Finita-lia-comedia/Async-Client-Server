#include <iostream>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <mutex>

using boost::asio::ip::tcp;
std::mutex console_mutex;

void h_client(int client_id, const std::string& server_ip, int port, int num_messages) {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);

        socket.connect(tcp::endpoint(boost::asio::ip::make_address(server_ip), port));

        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "Client " << client_id << " connected to server.\n";
        }

        for (int i = 1; i <= num_messages; ++i) {
            std::string message;
            {
                std::lock_guard<std::mutex> lock(console_mutex);
                std::cout << "Enter message " << i << " for Client " << client_id << ": ";
                std::getline(std::cin, message);
            }

            boost::asio::write(socket, boost::asio::buffer(message + "\n"));

            boost::asio::streambuf response_buffer;
            boost::asio::read_until(socket, response_buffer, "\n");

            std::string response;
            std::istream response_stream(&response_buffer);
            std::getline(response_stream, response);

            {
                std::lock_guard<std::mutex> lock(console_mutex);
                std::cout << "Client " << client_id << " received: " << response << "\n";
            }
        }

        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "Client " << client_id << " finished sending messages.\n";
        }
    }
    catch (std::exception& e) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "Client " << client_id << " error: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: client.exe <server_ip> <port> <num_clients> <num_messages>\n";
        return 1;
    }

    std::string server_ip = argv[1];
    int port = std::atoi(argv[2]);
    int num_clients = std::atoi(argv[3]);
    int num_messages = std::atoi(argv[4]);

    std::vector<std::thread> threads;
    for (int i = 1; i <= num_clients; ++i) {
        threads.emplace_back(h_client, i, server_ip, port, num_messages);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}