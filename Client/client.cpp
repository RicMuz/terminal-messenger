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

        // Ask server and get answer from server
        handle_request();

        // Print answer to user
        print_answer();
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

    // Choose current interface
    if(!logged_in) {
        before_log_in_interface();
    } else {
        after_log_in_interface();
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
            type_of_request = -1;
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

    data_to_send = user_name + " " + password;
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
Client::after_log_in_interface() {
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
            type_of_request = 3;
            break;
        } else if (input == "logout") {
            type_of_request = 2;
            break;
        } else if (input == "exit") {
            type_of_request = -1;
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

    data_to_send = friends_name;
}


//============================================================================================================
// HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDL
//============================================================================================================

void
Client::handle_request() {
    switch (type_of_request)
    {
    case -1: // exit
        // need to log out
        break;
    case 0|1|2|3|4|5|6: // requires answer from server
        create_packet();
        send_packet(to_send);
        break;
    
    default:
        break;
    }
}

void
Client::create_packet() {
    // Make an empty packet
    to_send.clear();

    // Make the packet
    to_send << type_of_request << " " << data_to_send;
}