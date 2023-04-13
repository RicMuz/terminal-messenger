#include "client.hpp"

int main(int argc, char * argv[]) {
    Client client;

    // Try connect to the server
    if(!client.Connect("127.0.0.1", 7070)) {
        return 1;
    }

    // Run the client
    client.Run();

    return 0;
}