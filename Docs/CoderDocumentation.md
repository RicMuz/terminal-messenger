# *TMessenger (Terminal Messenger)*

## Packets
*Packets are used to send information to and from server*

### Requst packets
```type user_name data```

types:
- 0 = sign up
- 1 = log in
- 2 = log out
- 3 = add friend
- 4 = open chat
- 5 = send message
- 6 = list friends
- all others are handlel as errors

user_name is name of user who send the request. For types 0 and 1 it is used to check databases of users. From 3 up it is used to check if requst came from correct address. 

data differs from type to type:
- 0 -> password
- 1 -> password
- 2 -> empty
- 3 -> other_user_name = name of user who will be added to friends
- 4 -> other_user_name = name of user with whom request sender has chat and wants to open it
- 5 -> other_user_name message = message is text that will be added to chat with other_user_names
- 6 -> empty

### Answer packets
```return_code data```

return codes:
- 0 = success
- 1 = user already exists (sign up request)
- 2 = wrong password (log in request)
- 3 = user does not exist (log in request)
- 4 = user_name from packet is different than one with which user log in on given address (general)
- 5 = user wants to add himself as friend (add friend request)
- 6 = user wants to add account which does not exist (add friend request)
- 7 = users are not friends

data differs from type of requests:
- 4 -> last 10 messages
- 6 -> friend list
- all other are empty

## Client

### Class variables

- ```int type_of_requst``` tells what the program should do next and if sending packet is neccesary, than it contains what type of packet is needed to be send to the server
- ```sf::TcpSocket socket``` is TCP socket connected to the server
- ```sf::Packet to_send``` is packet containing the request, which will be send to the server
- ```std::string logged_user_name``` contains the name of the user who is logged in (before sign up it is empty)
- ```bool logged_in``` contains information if any user is logged in in the client (deciding which interface should be used)
- ```std::string data_to_send``` cointains additional information that server needs for current request 
- ```bool exit``` tells the program if user wants to exit the program
