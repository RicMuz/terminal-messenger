#pragma once

#include <string>
#include <iostream>
#include <map>
#include <SFML/Network.hpp>

class Client {
private:
    sf::TcpSocket socket;
    std::string logged_user_name;
    bool connected;
    bool logged_in;
    std::string data_to_send;
    int type_of_data;

    
    void get_user_input();

    void before_log_in_interface();
    void print_before_log_in_help();
    void get_user_name_and_password();
    bool check_user_name(const std::string &user_name); // should contain just alphabet and numbers
    bool check_password(const std::string &password); // should not contain any whitespace

    void after_log_in_interface();
    void print_after_log_in_help();
    void get_name_of_friend();

    void handle_request();
    void send_packet(sf::Packet &packet);
    void receive_packet();

    void print_answer();
public:
    Client();
    bool Connect(const std::string &address, const unsigned short port);
    void Run();
};