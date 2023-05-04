#include "server.hpp"

sf::Packet &operator>>(sf::Packet &packet, type_of_request &data) {
    int temp;
    packet >> temp;
    data = static_cast<type_of_request>(temp);
    return packet;
}

sf::Packet &operator<<(sf::Packet &packet, return_code &code) {
    int temp;
    temp = static_cast<int>(code);
    packet << temp;
    return packet;
}

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
        std::cout << "Error: could not listen" << std::endl;
        return;
    }

    // Check or create account_database.txt
    std::ofstream out;
    out.open(user_database_name, std::ios::app);

    if(!out.good()) {
        std::cout << "Error: could not open account database" << std::endl;
        return;
    }

    out.close();

    std::cout << "Server started" << std::endl;
    std::cout << "Listening on port: " << port << std::endl;
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
    std::stringstream user_address_sream;
    user_address_sream << client->getRemoteAddress() << ":" << client->getRemotePort();
    std::cout << user_address_sream.str() << " disconnected" << std::endl;
    if(loged_users.count(user_address_sream.str())) {
        std::cout << "User haven't logged out. Requesting log out from server." << std::endl;
        log_out(client, true);
    }
    selector.remove(*client);
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

    std::cout << "Recieved good packet" << std::endl;
    // Possibly good packet
    manage_packet(packet, client);
}

void
Server::manage_packet(sf::Packet &packet, sf::TcpSocket *client) {
    type_of_request type;
    packet >> type;

    switch (type) //TODO: should watch if 2 login requests won't come from the same socket
    {
    case type_of_request::sign_up: // signup
        sign_up(packet, client);
        break;
    case type_of_request::log_in: // login
        log_in(packet, client);
        break;
    case type_of_request::log_out: // logout
        log_out(client, false);
        break;
    case type_of_request::add_friend: // add users to friend list
        add_friend(packet, client);
        break;
    case type_of_request::open_chat: // open chat
        open_chat(packet, client);
        break;
    case type_of_request::send_message: // send message to chat
        send_message(packet, client);
        break;
    case type_of_request::list_friends: // lists friends from friend list
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

    std::cout << "Answer send successfuly" << std::endl;
}

bool
Server::check_user_and_address(const std::string &user_name, const std::string &type,sf::TcpSocket *client) {
    std::stringstream user_address;
    user_address << client->getRemoteAddress() << ":" << client->getRemotePort();

    sf::Packet answer;

    // Test if the right user is asking
    if(loged_users[user_address.str()] != user_name) {
        std::cout << "Error: " << user_address.str() << " wants to " << type << " as " << user_name << " although " << loged_users[user_address.str()] << " is logged in on that address" << std::endl;
        answer << return_code::unauthorized_access;
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
    std::cout << "Handling sign up request." << std::endl;

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
        answer << return_code::user_exists;
    } else {
        // Send successful response and add to database
        add_user_to_database(user_name, password);

        // Create friend list file
        std::ofstream friend_list_stream;
        friend_list_stream.open(user_name + ".txt", std::ios::app);
        friend_list_stream.close();

        answer << return_code::success;
    }

    // Send the response
    send_answer_to_client(answer, "sign up", client);
}

bool
Server::is_username_used(std::string &user_name) {
    bool to_return = false; // Need to close the file before returning the value

    // Cycle through all user names from database
    std::ifstream account_database(user_database_name);
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
    out.open(user_database_name, std::ios::app);

    // Add new user
    out << user_name << " " << password << std::endl;
    out.close(); 
}

//============================================================================================================
// LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN - LOG IN -
//============================================================================================================

void
Server::log_in(sf::Packet &packet, sf::TcpSocket *client) {
    std::cout << "Handling log in request." << std::endl;

    // Extract username and password
    std::string user_name, password;
    packet >> user_name;
    packet >> password;

    // Check database
    return_code answer_from_database = check_login_data(user_name, password);

    // Create packet with answer
    sf::Packet answer;
    answer << answer_from_database;

    if(answer_from_database == return_code::success) { 
        std::stringstream user_address;
        user_address << client->getRemoteAddress() << ":" << client->getRemotePort();
        loged_users[user_address.str()] = user_name;
        std::cout << "User " << user_name << " has loged in on " << user_address.str() << std::endl;
    }

    // Send the response
    send_answer_to_client(answer, "log in", client);
}

return_code
Server::check_login_data(std::string &user_name, std::string &password) {
    return_code to_return = return_code::user_does_not_exist; // Need to close the file before returning the values
    /*
    0 - login successful
    2 - wrong password
    3 - user does not exist
    */

    // Cycle through all user names from database
    std::ifstream account_database(user_database_name);
    for(std::string line; getline(account_database, line);) {
        std::stringstream line_stream(line);
        std::string user_name_database, password_database;
        line_stream >> user_name_database;
        line_stream >> password_database;

        // If if username is already in there then break
        if(user_name == user_name_database) {
            if(password == password_database) {
                to_return = return_code::success;
                break;
            }
            to_return = return_code::wrong_password;
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
Server::log_out(sf::TcpSocket *client, bool from_server) {
    std::cout << "Handling log out request." << std::endl;

    // Get user address and port
    std::stringstream user_address;
    user_address << client->getRemoteAddress() << ":" << client->getRemotePort();

    // Delete it from map
    loged_users.erase(user_address.str());

    if(!from_server) {
        // Create packet with successful answer
        sf::Packet answer;
        answer << return_code::success;

        send_answer_to_client(answer, "logout", client);
    }
}

//============================================================================================================
// ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD FRIEND - ADD 
//============================================================================================================

void
Server::add_friend(sf::Packet &packet, sf::TcpSocket *client) {
    std::cout << "Handling add friend request." << std::endl;

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
        answer << return_code::self_friend;
        send_answer_to_client(answer, "add friend", client);
        return;
    }

    // User wants to add non existing account 
    if(!is_username_used(other_user)) {
        std::cout << "Error: user " << user_name << " wants to add non-existing account as friend" << std::endl;
        answer << return_code::non_existing_friend;
        send_answer_to_client(answer, "add friend", client);
        return;
    }

    // Add friend to friend list file
    add_to_friend_list(user_name, other_user);
    add_to_friend_list(other_user, user_name);

    // Create chat file
    std::string chat_file_name = create_chat_file_name(user_name, other_user);
    std::ofstream out;
    out.open(chat_file_name, std::ios::app);
    out.close(); 

    // Create an answer with possitive result
    answer << return_code::success;

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
    std::cout << "Handling open chat request." << std::endl;

    std::string user_name, other_user, file_name, chat;
    packet >> user_name;
    packet >> other_user;

    // Test if the right user is asking
    if(!check_user_and_address(user_name, "open chat", client)) {
        return;
    }

    sf::Packet answer;

    // Check if they are friends
    if(!are_friends(user_name, other_user)) {
        answer << return_code::not_friends;
        std::cout << "Error: user wants to open non-existing chat";
        send_answer_to_client(answer, "open chat", client);
        return;
    }

    // Get the chat file name
    file_name = create_chat_file_name(user_name, other_user);

    // Get requested lines
    chat = get_last_n_messages(file_name, 10);

    // Create an answer with possitive result
    answer << return_code::success;
    answer << chat;

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

bool
Server::are_friends(const std::string &user_name, const std::string &other_user) {
    bool to_return = false;

    // Open file 
    std::ifstream friend_list_stream;
    friend_list_stream.open(user_name + ".txt");

    // Check all lines if other user is in the friend list
    for(std::string line; getline(friend_list_stream, line);) {
        if(other_user == line) {
            to_return = true;
            break;
        }
    }

    // Close the file
    friend_list_stream.close();

    return to_return;
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
    while(!lines.empty()) {
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
    std::cout << "Handling send message request." << std::endl;

    std::string user_name, other_user, message, file_name;
    packet >> user_name;
    packet >> other_user;
    packet >> message;

    // Test if the right user is asking
    if(!check_user_and_address(user_name, "log in", client)) {
        return;
    }

    sf::Packet answer;

    // Check if they are friends
    if(!are_friends(user_name, other_user)) {
        answer << return_code::not_friends;
        std::cout << "Error: user wants to open non-existing chat";
        send_answer_to_client(answer, "open chat", client);
        return;
    }

    // Get the chat file name
    file_name = create_chat_file_name(user_name, other_user);

    // Add the message to the end of chat file
    add_message_to_file(user_name, message, file_name);

    // Create an answer with possitive result
    answer << return_code::success;

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
    std::cout << "Handling list friends request." << std::endl;

    std::string user_name;
    packet >> user_name;

    // Test if the right user is asking
    if(!check_user_and_address(user_name, "log in", client)) {
        return;
    }

    sf::Packet answer;

    // Create return packet
    answer << return_code::success;
    answer << friend_list(user_name);

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