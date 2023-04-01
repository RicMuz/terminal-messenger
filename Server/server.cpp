#pragma once
#include "server.hpp"

void
Server::Run() {
    selector.add(listener);

    while(true) {
        // make selector wait for data on any socket
        if(selector.wait()) {
            // control if there isn't pending conection
            if(selector.isReady(listener)) {
                connect_client();
            } else {
                // test all clients
                for(size_t i = 0; i < clients.size(); ++i) {
                    receive_packet(clients[i].get(), i);
                }
            }
        } 
    }
}

Server::Server(unsigned short port) {
    std::cout << "Starting the server" << std::endl;

    this->port = port;

    if(listener.listen(port) != sf::Socket::Done) {
        std::cout << "Error: could not listen" << std::endl; // Should end the program
    }

    std::cout << "Server started" << std::endl;
}

void
Server::connect_client() {
    // Create pointer for new client
    std::unique_ptr<sf::TcpSocket> new_client = std::make_unique<sf::TcpSocket>();

    // Try connect client
    if(listener.accept(*new_client) == sf::Socket::Done) {
        // If successful then add him to clients and start listening
        clients.push_back(std::move(new_client));
        selector.add(*clients.back());
    } else {
        std::cout << "Error: could not connect client" << std::endl;
    }
}
