#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <SFML/Network.hpp>

class Server {
private:
    unsigned short port;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;

    void connect_client(); // set up tcp connection
    void disconnect_client(sf::TcpSocket *client, size_t position); // end tcp conncetion
    void receive_packet(sf::TcpSocket *client, size_t position); // check if socket received a packet

    void manage_packet(const sf::Packet &packet); // decide what to do with the packet

public:
    Server(unsigned short);
    void Run();
};