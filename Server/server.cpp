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

Server::Server(unsigned short port) { //TODO: should check if database of users ("account_database.txt") exists, and if not then create empty
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
        sign_up(packet, client);
        break;
    case 1: // login
        log_in(packet, client);
        break;
    case 2: // logout
        log_out(client);
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

void
Server::send_answer_to_client(sf::Packet &packet, const std::string &type, sf::TcpSocket *client) {
    if(client->send(packet) != sf::Socket::Done) {
        std::cout << "Error: could not send response for " << type <<" request of " << client->getRemoteAddress() << ":" << client->getRemotePort() << std::endl;
    }
}

void
Server::sign_up(sf::Packet &packet, sf::TcpSocket *client) {
    // Extract wanted username and password
    //TODO: what if password or both are missing
    std::string user_name, password;
    packet >> user_name;
    packet >> password;

    // Prepare packet for response
    sf::Packet answer;

    // Check if username isn't already used in database
    bool already_exists = is_username_used(user_name);

    if(already_exists) {
        // Send unsuccesseful response
        answer << 1;
    } else {
        // Send successful response and add to database
        add_user_to_database(user_name, password);
        answer << 0;
    }

    // Send the response
    send_answer_to_client(answer, "sign up", client);
}

bool
Server::is_username_used(std::string &user_name) {
    bool to_return = false; // Need to close the file before returning the value

    // Cycle through all user names from database
    std::ifstream account_database("account_database.txt");
    for(std::string line; getline(account_database, line);) {
        std::stringstream line_steam(line);
        std::string known_user;
        line_steam >> known_user;

        // If if username is already in there then break
        if(user_name == known_user) {
            to_return = true;
            break;
        }
    }
    account_database.close();

    return to_return;
}

void
Server::add_user_to_database(const std::string &user_name, const std::string &password) {
    // Open database in append mode 
    std::ofstream out;
    out.open("account_database.txt", std::ios::app);

    // Add new user
    out << user_name << " " << password << std::endl;
    out.close(); 
}

void
Server::log_in(sf::Packet &packet, sf::TcpSocket *client) {
    // Extract username and password
    //TODO: what if password or both are missing
    std::string user_name, password;
    packet >> user_name;
    packet >> password;

    // Check database
    int answer_from_database = check_login_data(user_name, password);

    // Create packet with answer
    sf::Packet answer;
    answer << answer_from_database;

    if(answer_from_database == 0) { 
        std::stringstream user_address;
        user_address << client->getRemoteAddress() << ":" << client->getRemotePort();
        loged_users[user_address.str()] = user_name;
        std::cout << "User " << user_name << " has loged in on " << user_address.str() << std::endl;
    }

    // Send the response
    send_answer_to_client(answer, "log in", client);
}

int
Server::check_login_data(std::string &user_name, std::string &password) {
    int to_return = 2; // Need to close the file before returning the values
    /*
    0 - login successful
    1 - wrong password
    2 - user does not exist
    */

    // Cycle through all user names from database
    std::ifstream account_database("account_database.txt");
    for(std::string line; getline(account_database, line);) {
        std::stringstream line_stream(line);
        std::string user_name_database, password_database;
        line_stream >> user_name_database;
        line_stream >> password_database;

        // If if username is already in there then break
        if(user_name == user_name_database) {
            if(password == password_database) {
                to_return = 0;
                break;
            }
            to_return = 1;
            break;
        }
    }
    account_database.close();

    return to_return;
}

void
Server::log_out(sf::TcpSocket *client) {
    // Get user address and port
    std::stringstream user_address;
    user_address << client->getRemoteAddress() << ":" << client->getRemotePort();

    // Delete it from map
    loged_users.erase(user_address.str());
}
