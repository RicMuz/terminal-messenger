#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <SFML/Network.hpp>

class Client {
private:
    sf::TcpSocket socket;

    sf::Packet to_send;
    std::string logged_user_name;
    bool logged_in;
    
    bool in_chat_room;
    std::string friend_name;

    std::vector<std::string> data_to_send;
    int type_of_request;

    sf::Packet received_packet;
    int received_code;
    std::string received_data;

    bool exit;

    
    void get_user_input();

    void before_log_in_interface();
    void print_before_log_in_help();
    void get_user_name_and_password();
    bool check_user_name(const std::string &user_name); // should contain just alphabet and numbers
    bool check_password(const std::string &password); // should not contain any whitespace

    void after_log_in_interface_menu();
    void print_after_log_in_help();
    void get_name_of_friend();

    void after_log_in_interface_message_room();
    void print_message_room_help();

    void handle_request();
    void create_packet();
    void send_packet(sf::Packet &packet);

    void receive_packet();
    void print_answer();
public:
    Client();
    bool Connect(const std::string &address, const unsigned short port);
    void Run();
};