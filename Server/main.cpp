#include "server.hpp"

int main(int argc, char * argv[]) {
    // initialize server with port 7070
    Server server(7070);

    // run the main loop
    server.Run();

    return 0;
}