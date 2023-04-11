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
        return;
    }

    // Check or create account_database.txt
    std::ofstream out;
    out.open("account_database.txt", std::ios::app);

    if(!out.good()) {
        std::cout << "Error: could not open account database" << std::endl;
        return;
    }

    out.close();

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
    case 3: // add users to friend list
        add_friend(packet, client);
        break;
    case 4: // open chat
        open_chat(packet, client);
        break;
    case 5: // send message to chat
        send_message(packet, client);
        break;
    case 6: // lists friends from friend list
        list_friends(packet, client);
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

bool
Server::check_user_and_address(const std::string &user_name, const std::string &type,sf::TcpSocket *client) {
    std::stringstream user_address;
    user_address << client->getRemoteAddress() << ":" << client->getRemotePort();

    sf::Packet answer;

    // Test if the right user is asking
    if(loged_users[user_address.str()] != user_name) {
        std::cout << "Error: " << user_address.str() << " wants to " << type << " as " << user_name << " although " << loged_users[user_address.str()] << " is logged in on that address" << std::endl;
        answer << 4;
        send_answer_to_client(answer, type, client);
        return false;
    }

    return true;
}

//============================================================================================================
// SIGN UP - SIGN UP - SIGN UP - SIGN UP - SIGN UP - SIGN UP - SIGN UP - SIGN UP - SIGN UP - SIGN UP - SIGN UP
//============================================================================================================

void
Server::sign_up(sf::Packet &packet, sf::TcpSocket *client) {
    // Extract wanted username and password
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

        // Create friend list file
        std::ofstream friend_list_stream;
        friend_list_stream.open(user_name + ".txt", std::ios::app);
        friend_list_stream.close();

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

//============================================================================================================
// LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN -
//============================================================================================================

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
    int to_return = 3; // Need to close the file before returning the values
    /*
    0 - login successful
    2 - wrong password
    3 - user does not exist
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
            to_return = 2;
            break;
        }
    }
    account_database.close();

    return to_return;
}

//============================================================================================================
// LOG OUT - LOG OUT - LOG OUT - LOG OUT - LOG OUT - LOG OUT - LOG OUT - LOG OUT - LOG OUT - LOG OUT - LOG OUT
//============================================================================================================

void
Server::log_out(sf::TcpSocket *client) {
    // Get user address and port
    std::stringstream user_address;
    user_address << client->getRemoteAddress() << ":" << client->getRemotePort();

    // Delete it from map
    loged_users.erase(user_address.str());
}

//============================================================================================================
// ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD 
//============================================================================================================

void
Server::add_friend(sf::Packet &packet, sf::TcpSocket *client) {
    std::string user_name, other_user, friendlist_file_name;
    packet >> user_name;
    packet >> other_user;

    // Test if the right user is asking
    if(!check_user_and_address(user_name, "log in", client)) {
        return;
    }

    sf::Packet answer;

    // User wants to add himself as friend
    if(user_name == other_user) {
        std::cout << "Error: user " << user_name << " wants to add himself as friend." << std::endl;
        answer << 5;
        send_answer_to_client(answer, "add friend", client);
        return;
    }

    // User wants to add non existing account 
    if(!is_username_used(other_user)) {
        std::cout << "Error: user " << user_name << " wants to add non-existing account as friend" << std::endl;
        answer << 6;
        send_answer_to_client(answer, "add friend", client);
        return;
    }

    // Add friend to friend list file
    add_to_friend_list(user_name, other_user);

    // Create chat file
    std::string chat_file_name = create_chat_file_name(user_name, other_user);
    std::ofstream out;
    out.open(chat_file_name, std::ios::app);
    out.close(); 

    // Create an answer with possitive result
    answer << 0;

    // Send the response
    send_answer_to_client(answer, "add friend", client);
    
}

void
Server::add_to_friend_list(const std::string &user_name, const std::string &other_user_name) {
    // Open chat file in append mode 
    std::ofstream out;
    out.open(user_name + ".txt", std::ios::app);

    // Add the message
    out << other_user_name << std::endl;
    out.close(); 
}

//============================================================================================================
// OPEN CHAT - OPEN CHAT - OPEN CHAT - OPEN CHAT - OPEN CHAT - OPEN CHAT - OPEN CHAT - OPEN CHAT - OPEN CHAT -  
//============================================================================================================

void
Server::open_chat(sf::Packet &packet, sf::TcpSocket *client) {
    std::string user_name, other_user, file_name, chat;
    packet >> user_name;
    packet >> other_user;

    // Test if the right user is asking
    if(!check_user_and_address(user_name, "open chat", client)) {
        return;
    }

    sf::Packet answer;

    // TODO: other user can not exist (typo...), file might not exist (typo, aren't friens)
    // Get the chat file name
    file_name = create_chat_file_name(user_name, other_user);

    // Get requested lines
    chat = get_last_n_messages(file_name, 10);

    // Create an answer with possitive result
    answer << 0 << " " << chat;

    // Send the response
    send_answer_to_client(answer, "open chat", client);
}

std::string
Server::create_chat_file_name(const std::string &user_name, const std::string &other_user_name) {
    if(user_name > other_user_name) {
        return user_name + "-" + other_user_name + ".txt";
    } else {
        return other_user_name + "-" + user_name + ".txt";
    }
}

std::string
Server::get_last_n_messages(const std::string &file_name, int n) {
    std::ifstream chat_stream;
    chat_stream.open(file_name);

    // Test if the file can be openned 
    if(!chat_stream.good()) {
        std::cout << "Error: can't open chat file " << file_name << std::endl;
        return "Error: can't open chat file, contact administrator";
    }

    std::queue<std::string> lines;
    
    // Get last n lines if possible
    for(std::string line; getline(chat_stream, line);) {
        lines.push(line);
        if(lines.size() > n) {
            lines.pop();
        }
    }

    chat_stream.close();

    std::stringstream output_stream;

    // Construct them back with new line separator
    for(int i = 0; i < lines.size(); ++i) {
        output_stream << lines.front() << std::endl;
        lines.pop();
    }

    return output_stream.str();
}

//============================================================================================================
// SEND MESSAGE - SEND MESSAGE - SEND MESSAGE - SEND MESSAGE - SEND MESSAGE - SEND MESSAGE - SEND MESSAGE - SE   
//============================================================================================================

void
Server::send_message(sf::Packet &packet, sf::TcpSocket *client) {
    std::string user_name, other_user, message, file_name;
    packet >> user_name;
    packet >> other_user;
    packet >> message;

    // Test if the right user is asking
    if(!check_user_and_address(user_name, "log in", client)) {
        return;
    }

    sf::Packet answer;

    // TODO: other user can not exist (typo...), file might not exist (typo, aren't friens)
    // Get the chat file name
    file_name = create_chat_file_name(user_name, other_user);

    // Add the message to the end of chat file
    add_message_to_file(user_name, message, file_name);

    // Create an answer with possitive result
    answer << 0;

    // Send the response 
    send_answer_to_client(answer, "send message", client);
}

void
Server::add_message_to_file(const std::string &user_name, const std::string &message, const std::string &file_name) {
    // Open chat file in append mode 
    std::ofstream out;
    out.open(file_name, std::ios::app);

    // Add the message
    out << user_name << ": " << message << std::endl;
    out.close(); 
}

//============================================================================================================
// LIST FRIENDS - LIST FRIENDS - LIST FRIENDS - LIST FRIENDS - LIST FRIENDS - LIST FRIENDS - LIST FRIENDS - LI   
//============================================================================================================

void
Server::list_friends(sf::Packet &packet, sf::TcpSocket *client) {
    std::string user_name;
    packet >> user_name;

    // Test if the right user is asking
    if(!check_user_and_address(user_name, "log in", client)) {
        return;
    }

    sf::Packet answer;

    // Create return packet
    answer << 0 << " " << friend_list(user_name);

    // Send the response
    send_answer_to_client(answer, "list friends", client);
}

std::string
Server::friend_list(const std::string &user_name) {
    // Open file 
    std::ifstream friend_list_stream;
    friend_list_stream.open(user_name + ".txt");

    // Create string buffer and read the file
    std::stringstream buffer;
    buffer << friend_list_stream.rdbuf();

    // Close the file
    friend_list_stream.close();

    return buffer.str();
}