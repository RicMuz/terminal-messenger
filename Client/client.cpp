#include "client.hpp"

Client::Client() {
    std::cout << "Terminal messenger started" << std::endl;
}

bool
Client::Connect(const std::string &address, const unsigned short port) {
    std::cout << "Connecting to the server" << std::endl;

    // Try to connect to the server, if not possible exit the program
    if(socket.connect(address, port) != sf::Socket::Done) {
        std::cout << "Error: could not connect to the server." << std::endl;
        return false;
    }

    std::cout << "Connection successful" << std::endl;
    return true;
}

void
Client::Run() {
    while(true) {
        // Get correct command and connected data from user
        get_user_input();

        // If user wants to exit and isn't logged in simply exit the program
        if(exit && !logged_in) {
            break;
        }

        // Ask server and get answer from server
        handle_request();

        // If user wants to exit and is logged in send packet to log out and exit
        if(exit) {
            break;
        }

        // Print answer to user
        receive_packet();
    }
}

//============================================================================================================
// GET USER INPUT - GET USER INPUT - GET USER INPUT - GET USER INPUT - GET USER INPUT - GET USER INPUT - GET U
//============================================================================================================

void
Client::get_user_input() {
    // Empty request before
    data_to_send.clear();
    to_send.clear();
    exit = false;

    // Choose current interface
    if(!logged_in) {
        before_log_in_interface();
    } else {
        data_to_send.push_back(logged_user_name);
        if(in_chat_room) {
            data_to_send.push_back(friend_name);
            after_log_in_interface_message_room();
        } else {
            after_log_in_interface_menu();
        }
    }
}

void
Client::before_log_in_interface() {
    // Cycle while all necessary data aren't collected
    while(true) {
        std::string input;
        std::cout << ">>>";
        std::cin >> input;

        if(input == "help") {
            print_before_log_in_help();
        } else if (input == "signup") {
            get_user_name_and_password();
            type_of_request = 0;
            break;
        } else if (input == "login") {
            get_user_name_and_password();
            type_of_request = 1;
            break;
        } else if (input == "exit") {
            exit = true;
            break;
        } else {
            std::cout << "Unknown command. Type help to print commands." << std::endl;
        }
    }
}

void
Client::print_before_log_in_help() {
    std::cout << "login\tto log in" << std::endl;
    std::cout << "signup\tto create a new account" <<std::endl;
    std::cout << "exit\tto exit the app" <<std::endl;
}

void
Client::get_user_name_and_password() {
    std::string user_name, password;

    std::cout << "user name: ";
    bool good_user_name = false;

    while(!good_user_name) {
        std::cin >> user_name;
        good_user_name = check_user_name(user_name);
        if(!good_user_name) {
            std::cout << "This username is not allowed, try again: ";
        }
    }

    std::cout << "password: ";
    bool good_password = false;

    while (!good_password) {
        std::cin >> password;
        good_password = check_password(password);
        if(!good_password) {
            std::cout << "Bad password, password cannot contain any type of white space, try again: ";
        }
    }

    logged_user_name = user_name;
    data_to_send.push_back(user_name);
    data_to_send.push_back(password);
}

bool
Client::check_user_name(const std::string &user_name) {
    // Check if the user name contains just alphanumeric symbols
    for(auto && c:user_name) {
        if(!isalnum(c)) {
            return false;
        }
    }
    return true;
}

bool
Client::check_password(const std::string &password) {
    // Check if password does not contain white space
    for(auto && c:password) {
        if(isspace(c)) {
            return false;
        }
    }
    return true;
}

void
Client::after_log_in_interface_menu() {
    // Cycle while all necessary data aren't collected
    while(true) {
        std::string input;
        std::cout << logged_user_name <<">>>";
        std::cin >> input;

        if(input == "help") {
            print_after_log_in_help();
        } else if (input == "ls") {
            type_of_request = 6;
            break; 
        } else if (input == "open") {
            get_name_of_friend();
            type_of_request = 4;
            break;
        } else if (input == "add") {
            get_name_of_friend();
            in_chat_room = true;
            type_of_request = 3;
            break;
        } else if (input == "logout") {
            type_of_request = 2;
            break;
        } else if (input == "exit") {
            type_of_request = 2; // before exiting the program we need to log out
            exit = true;
            break;
        } else {
            std::cout << "Unknown command. Type help to print commands." << std::endl;
        }
    }
}

void
Client::print_after_log_in_help() {
    std::cout << "ls\tto list all friends" << std::endl;
    std::cout << "open friend_name\t to access chat" << std::endl;
    std::cout << "add friend_name\t to add friend to your list of friends (if exists in database of people)" << std::endl;
    std::cout << "logout\tto log out" << std::endl;
    std::cout << "exit\tto log out and exit the app" << std::endl; 
}

void
Client::get_name_of_friend() {
    std::string friends_name;
    std::cin >> friends_name;

    // Wait till good data are inserted
    while(true) {
        if(check_user_name(friends_name) && friends_name != "") {
            break;
        }
        std::cout << "Wrong or missing friend's name try again: ";
        std::cin >> friends_name;
    }

    friend_name = friends_name;
    data_to_send.push_back(friends_name);
}

void
Client::after_log_in_interface_message_room() {
    while(true) {
        std::string input;
        std::cout << logged_user_name << "-" << friend_name <<">>>";
        std::cin >> input;

        if(input == "help") {
            print_message_room_help();
        } else if(input == "send") {
            type_of_request = 5;
            std::string message;
            getline(std::cin, message); // TODO: get rid of leading and trailing white spaces
            data_to_send.push_back(message);
            break;
        } else if(input == "leave") {
            type_of_request = 100;
            break;
        } else {
            std::cout << "Unknown command. Type help to print commands." << std::endl;
        }
    }
}


//============================================================================================================
// HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDL
//============================================================================================================

void
Client::handle_request() {
    switch (type_of_request)
    {
    case 0 ... 6: // requires answer from server
        create_packet();
        send_packet(to_send);
        break;

    case 100:
        in_chat_room = false;
        friend_name.clear();
        break;
    
    default:
        std::cout << "Erorr: unknown request " << type_of_request << std::endl;
        exit = true;
        break;
    }
}

void
Client::create_packet() {
    // Make an empty packet
    to_send.clear();

    // Make the packet
    to_send << type_of_request;
    for(auto && s:data_to_send) {
        to_send << s;
    }
}

void
Client::send_packet(sf::Packet &packet) {
    // Try to send the packet, if fails than exit
    if(socket.send(packet) != sf::Socket::Done) {
        std::cout << "Error: could not send packet, exiting program" << std::endl;
        exit = true;
    }
}

//============================================================================================================
// PRINT ANSWER - PRINT ANSWER - PRINT ANSWER - PRINT ANSWER - PRINT ANSWER - PRINT ANSWER - PRINT ANSWER - PR 
//============================================================================================================

void
Client::receive_packet() {
    received_packet.clear();

    while(true) {
        if(socket.receive(received_packet)==sf::Socket::Done) {
            break;
        }
    }

    print_answer();
}

void
Client::print_answer() {
    received_packet >> received_code;

    switch (received_code)
    {
    case 0:
        switch (type_of_request)
        {
        case 0:
            std::cout << "Sign up successful, try log in." << std::endl;
            break;
        case 1:
            std::cout << "Log in successful." << std::endl;
            logged_in = true;
            break;
        case 2:
            std::cout << "Log out successful." << std::endl;
            logged_in = false;
        case 3:
            std::cout << "You're friends now." << std::endl; 
            break;
        case 4: case 6:
            received_packet >> received_data;
            std::cout << received_data;
            break;
        case 5:
            std::cout << "OK" << std::endl;
            break;

        default:
            std::cout << "Success for unknown request" << std::endl;
            break;
        }
        break;
    case 1:
        std::cout << "Error: user already exists, try different name." << std::endl;
        break;
    case 2:
        std::cout << "Error: wrong password, try again" << std::endl;
        break;
    case 3:
        std::cout << "Error: user does not exist, try again." << std::endl;
        break;
    case 4:
        std::cout << "Error: unauthorized access, contact admin." << std::endl;
        break;
    case 5:
        std::cout << "Error: you can't add yourself as friend." << std::endl;
        break;
    case 6:
        std::cout << "Error: user does not exist." << std::endl;
        break;
    case 7:
        std::cout << "Error: you need to be friends to open chat." << std::endl;
        break;
    
    default:
        std::cout << "Error: unknown return code." << std::endl;
        break;
    }
}