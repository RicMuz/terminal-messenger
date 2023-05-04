add_executable(server
    main.cpp
    server.hpp
    server.cpp
    codes.hpp
)

target_link_libraries(server PRIVATE sfml-network sfml-system)