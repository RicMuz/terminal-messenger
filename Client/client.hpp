#pragma once

#include <string>
#include <iostream>
#include <SFML/Network.hpp>

class Client {
private:
    sf::TcpSocket socket;
    std::string user_name;
    bool connected;
    bool logged_in;

    void get_user_input();
    void send_packet(sf::Packet &packet);
    void receive_packet();
public:
    Client();
    void Run();
};