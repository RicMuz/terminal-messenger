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
                    if(selector.isReady(*clients[i])) {
                        receive_packet(clients[i].get(), i);
                        break; //TODO: possibly deleted user, but this might not be neccessary 
                    }
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
        std::cout << new_client->getRemoteAddress() << ":" << new_client->getRemotePort() << " connected" << std::endl;
        clients.push_back(std::move(new_client));
        selector.add(*clients.back());
    } else {
        std::cout << "Error: could not connect client" << std::endl;
    }
}

void
Server::disconnect_client(sf::TcpSocket *client, size_t position) {
    std::cout << client->getRemoteAddress() << ":" << client->getRemotePort() << " disconnected" << std::endl;
    client->disconnect();
    clients.erase(clients.begin() + position);
}

void
Server::receive_packet(sf::TcpSocket *client, size_t position) {
    sf::Packet packet;

    // Tcp connection could have ended
    if(client->receive(packet) == sf::Socket::Disconnected) {
        disconnect_client(client, position);
        return;
    }

    // If packet was empty something went wrong
    if(packet.getDataSize() <= 0) {
        std::cout << "Error: recieved empty packet" << std::endl;
        return;
    }

    // Possibly good packet
    manage_packet(packet, client);
}

void
Server::manage_packet(sf::Packet &packet, sf::TcpSocket *client) {
    int type;
    packet >> type;

    switch (type) //TODO: should watch if 2 login requests won't come from the same socket
    {
    case 0: // signup
        sign_up();
        break;
    case 1: // login
        log_in();
        break;
    case 2: // logout
        log_out();
        break;
    case 3: // open chat
        open_chat();
        break;
    case 4: // send message to chat
        send_message();
        break;
    
    default:
        std::cout << "Error: unknown packet type from " << client->getRemoteAddress() << ":" << client->getRemotePort() << std::endl;
        break;
    }
}
