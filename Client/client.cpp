#include "client.hpp"

Client::Client() {
    std::cout << "Terminal messenger started" << std::endl;
}

bool
Client::Connect(const std::string &address, const unsigned short port) {
    std::cout << "Connecting to the server" << std::endl;

    // Try to connect to the server, if not possible exit the program
    if(socket.connect(address, port) != sf::Socket::Done) {
        std::cout << "Error: could not connect to the server." << std::endl;
        return false;
    }

    std::cout << "Connection successful" << std::endl;
    return true;
}

void
Client::Run() {
    while(true) {
        // Get correct command and connected data from user
        get_user_input();

        // Ask server and get answer from server
        handle_request();

        // Print answer to user
        print_answer();
    }
}