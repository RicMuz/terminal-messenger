add_executable(client
    main.cpp
    client.hpp
    client.cpp
)

target_link_libraries(client PRIVATE sfml-network sfml-system)