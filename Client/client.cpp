#include "client.hpp"

sf::Packet &operator>>(sf::Packet &packet, return_code &code) {
    int temp;
    packet >> temp;
    code = static_cast<return_code>(temp);
    return packet;
}

sf::Packet &operator<<(sf::Packet &packet, type_of_request &request) {
    int temp;
    temp = static_cast<int>(request);
    packet << temp;
    return packet;
}

Client::Client() {
    std::cout << "Terminal messenger started" << std::endl;
    logged_in = false;
    in_chat_room = false;
    exit = false;
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

        if(should_receive_packet) {
            // Print answer to user
            receive_packet();
        }
    }
}

//============================================================================================================
// GET USER INPUT - GET USER INPUT - GET USER INPUT - GET USER INPUT - GET USER INPUT - GET USER INPUT - GET U
//============================================================================================================


void
Client::prompt_info(const std::string &prompt, bool &should_break_loop, std::stringstream &input_stream) {
    std::string temp;
    should_break_loop = false;

    // Print the prompt and get the input
    std::cout << prompt << ">>>";
    getline(std::cin, temp);

    // If C-d was pressed end the program
    if(std::cin.eof()) {
        request_code = type_of_request::log_out;
        exit = true;
        should_break_loop = true;
    }

    // Create output
    input_stream.clear();
    input_stream = std::stringstream(temp);
}

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
        std::string input, user_name, password;
        bool should_break_loop;
        std::stringstream input_stream;
        prompt_info("",should_break_loop,input_stream);

        if(should_break_loop) {
            break;
        }

        input_stream >> input;
        input_stream >> user_name;
        input_stream >> password;

        if (input == "") {
            continue;
        } else if(input == "help") {
            print_before_log_in_help();
        } else if (input == "signup" || input == "login") {
            if(check_user_name(user_name) && check_password(password)) {
                data_to_send.push_back(user_name);
                data_to_send.push_back(password);
                if(input == "signup") {
                    request_code = type_of_request::sign_up;
                } else {
                    logged_user_name = user_name;
                    request_code = type_of_request::log_in;
                }
                break;
            } else {
                std::cout << "Wrong format of user name or password. Try again." << std::endl;
            }
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
    std::cout << "login $(user_name) $(password)\tto log in" << std::endl;
    std::cout << "signup $(user_name) $(password)\tto create a new account" <<std::endl;
    std::cout << "exit\tto exit the app" <<std::endl;
}

bool
Client::check_user_name(const std::string &user_name) {
    if(user_name.empty()) {
        return false;
    }

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
    if(password.empty()) {
        return false;
    }

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
        std::string input, temp;
        bool should_break_loop;
        std::stringstream input_stream;
        prompt_info(logged_user_name, should_break_loop, input_stream);

        if(should_break_loop) {
            break;
        }

        input_stream >> input;
        input_stream >> temp;

        if(input == "") {
            continue;
        } else if(input == "help") {
            print_after_log_in_help();
        } else if (input == "ls") {
            request_code = type_of_request::list_friends;
            break; 
        } else if (input == "open") {
            get_name_of_friend(temp);
            in_chat_room = true;
            request_code = type_of_request::open_chat;
            break;
        } else if (input == "add") {
            get_name_of_friend(temp);
            request_code = type_of_request::add_friend;
            break;
        } else if (input == "logout") {
            request_code = type_of_request::log_out;
            break;
        } else if (input == "exit") {
            request_code = type_of_request::log_out; // before exiting the program we need to log out
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
Client::get_name_of_friend(const std::string &input) {
    std::string friends_name;
    friends_name = input;

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
        std::string input, temp;
        bool should_break_loop;
        std::stringstream input_stream;
        prompt_info(logged_user_name + "-" + friend_name, should_break_loop, input_stream);

        if(should_break_loop) {
            break;
        }

        input_stream >> input;

        if(input == "") {
            continue;
        } else if(input == "help") {
            print_message_room_help();
        } else if(input == "send") {
            request_code = type_of_request::send_message;
            std::string message;
            input_stream >> std::ws && getline(input_stream,message,'\0');
            data_to_send.push_back(message);
            break;
        } else if(input == "leave") {
            request_code = type_of_request::exit_chat;
            break;
        } else {
            std::cout << "Unknown command. Type help to print commands." << std::endl;
        }
    }
}

void
Client::print_message_room_help() {
    std::cout << "send $(message)\tto send message" << std::endl;
    std::cout << "leave\tto exit to main menu" <<std::endl;
}


//============================================================================================================
// HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDLE REQUEST - HANDL
//============================================================================================================

void
Client::handle_request() {
    switch (request_code)
    {
    case type_of_request::sign_up ... type_of_request::list_friends:
        create_packet();
        should_receive_packet = true;
        send_packet(to_send);
        break;

    case type_of_request::exit_chat:
        in_chat_room = false;
        should_receive_packet = false;
        friend_name.clear();
        break;
    
    default:
        std::cout << "Erorr: unknown request " << request_code << std::endl;
        exit = true;
        break;
    }
}

void
Client::create_packet() {
    // Make an empty packet
    to_send.clear();

    // Make the packet
    to_send << request_code;
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
    case return_code::success:
        switch (request_code)
        {
        case type_of_request::sign_up:
            std::cout << "Sign up successful, try log in." << std::endl;
            break;
        case type_of_request::log_in:
            std::cout << "Log in successful." << std::endl;
            logged_in = true;
            break;
        case type_of_request::log_out:
            std::cout << "Log out successful." << std::endl;
            logged_in = false;
            break;
        case type_of_request::add_friend:
            std::cout << "You're friends now." << std::endl; 
            break;
        case type_of_request::open_chat: case type_of_request::list_friends:
            received_packet >> received_data;
            std::cout << received_data;
            break;
        case type_of_request::send_message:
            std::cout << "OK" << std::endl;
            break;

        default:
            std::cout << "Success for unknown request" << std::endl;
            break;
        }
        break;
    case return_code::user_exists:
        std::cout << "Error: user already exists, try different name." << std::endl;
        break;
    case return_code::wrong_password:
        std::cout << "Error: wrong password, try again" << std::endl;
        break;
    case return_code::user_does_not_exist:
        std::cout << "Error: user does not exist, try again." << std::endl;
        break;
    case return_code::unauthorized_access:
        std::cout << "Error: unauthorized access, contact admin." << std::endl;
        break;
    case return_code::self_friend:
        std::cout << "Error: you can't add yourself as friend." << std::endl;
        break;
    case return_code::non_existing_friend:
        std::cout << "Error: user does not exist." << std::endl;
        break;
    case return_code::not_friends:
        std::cout << "Error: you need to be friends to open chat." << std::endl;
        break;
    
    default:
        std::cout << "Error: unknown return code." << std::endl;
        break;
    }
}