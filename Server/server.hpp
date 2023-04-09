#pragma once

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <SFML/Network.hpp>

class Server {
private:
    unsigned short port;
    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
    std::map<std::string, std::string> loged_users;

    void connect_client(); // set up tcp connection
    void disconnect_client(sf::TcpSocket *client, size_t position); // end tcp conncetion
    void receive_packet(sf::TcpSocket *client, size_t position); // check if socket received a packet

    void manage_packet(sf::Packet &packet, sf::TcpSocket *client); // decide what to do with the packet

    void sign_up(sf::Packet &packet, sf::TcpSocket *client);
    void log_in(sf::Packet &packet, sf::TcpSocket *client);
    void log_out(sf::TcpSocket *client);
    void open_chat(sf::Packet &packet, sf::TcpSocket *client);
    void send_message(sf::Packet &packet, sf::TcpSocket *client);

    void send_answer_to_client(sf::Packet &packet, const std::string &type, sf::TcpSocket *client);

    bool is_username_used(std::string &user_name);
    void add_user_to_database(const std::string &user_name, const std::string &password);
    int check_login_data(std::string &user_name, std::string &password);
    std::string get_last_n_messages(const std::string &file_name, int n);
    void add_message_to_file(const std::string &user_name, const std::string &message, const std::string &file_name);

public:
    Server(unsigned short);
    void Run();
};