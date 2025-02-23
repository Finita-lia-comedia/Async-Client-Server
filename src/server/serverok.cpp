#include <cstdlib>   
#include <iostream>   
#include <memory>    
#include <utility>   
#include <boost/asio.hpp>  
#include <atomic>    
#include <mutex>      
#include <vector>     
#include <string>     

using boost::asio::ip::tcp;

std::atomic<int> connection_count(0);  
std::mutex console_mutex;              



class session
    : public std::enable_shared_from_this<session> 
{
public:
    session(tcp::socket socket, int id)
        : socket_(std::move(socket)), client_id_(id)
    {
        report_connection();
    }

    ~session()
    {
        report_disconnection();
    }

    void start()
    {
        do_read(); 
    }

private:
    int client_id_; 
    std::vector<std::string> messages_;

    void report_connection()
    {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << "Connection established: " << client_id_ << "\n";
    }

    void report_disconnection()
    {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << "Connection lost: " << client_id_ << "\n";
    }

    void do_read()
    {
        auto self(shared_from_this());  
        socket_.async_read_some(boost::asio::buffer(data_, max_length),  
            [this, self](boost::system::error_code ec, std::size_t length)  
            {
                if (!ec) 
                {
                    std::string message(data_, length);
                    messages_.push_back(message); 

                    if (message.find('\n') != std::string::npos)
                    {
                        procces_messages();
                    }
                    else
                    {
                        do_read();
                    }
                }
            });
    }

    void procces_messages()
    {
        std::string response = "Message received\n";
        boost::asio::write(socket_, boost::asio::buffer(response));

        messages_.clear();
        do_read();
    }

    tcp::socket socket_; 
    enum { max_length = 1024 }; 
    char data_[max_length];
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        do_accept(); 
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(  
            [this](boost::system::error_code ec, tcp::socket socket) 
            {
                if (!ec) 
                {
                    int client_id = ++connection_count;  
                    std::make_shared<session>(std::move(socket), client_id)->start(); 
                }

                do_accept(); 
            });
    }

    tcp::acceptor acceptor_;  
};

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1; 
        }

        boost::asio::io_context io_context; 

        server s(io_context, std::atoi(argv[1]));

        io_context.run(); 
    }
    catch (std::exception& e) 
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0; 
}