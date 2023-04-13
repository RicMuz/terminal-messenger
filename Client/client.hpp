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

    void handle_request();
    void send_packet(sf::Packet &packet);
    void receive_packet();

    void print_answer();
public:
    Client();
    bool Connect(const std::string &address, const unsigned short port);
    void Run();
};