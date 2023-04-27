add_executable(server
    main.cpp
    server.hpp
    server.cpp
)

target_link_libraries(server PRIVATE sfml-network sfml-system)