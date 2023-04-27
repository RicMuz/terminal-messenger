# *TMessenger (Terminal Messenger)*

**Author:** *Richard Mužík*

**Type:** *CLI Utility, Network utility, Application*

**Target Platform:** *Linux*

**Perceived difficulty:** *14 - 21 days of work*

*This project should be lightweight version of widely used applications like messenger or what's up. Unlike those, this one won't have GUI and will have just CLI. I want to learn how to send packets and work with databases more.*

## User interface
*This program aims to all people who do not care about GUI and love simple terminal applications. Once the application is executed user will use commands to control it.*

### CLI
*For unknown command:*

```
>>>sd
Unknown command. Type help to print commands.
```

*Help after open:*

```
>>>help
login   to login
signup  to create new user
exit    to exit the app
```

*Help after login:*

```
$(your_user_name)>>>help
ls      to list all friends
open    friend_name to access chat
add     friend_name to add friend to your list of friends (if exists in database of people)
logout  to log out 
exit    to log out and exit the app 
```

*Help in chat:*

```
$(your_user_name)>>>help
send "Your message"     to send message
leave                   to exit to main menu
```

*The program will print last 20 messages while in chat any new ones will be added to them.*

```
$(your_user_name)
Hi!

$(user_name)
Hey!

$(your_user_name)
How is it going?

$(user_name)
Good!

$(user_name)
How are you?

$(your_user_name)
Good, good.

$(user_name)
Have you talked to Peter lately?

$(your_user_name)
No, I haven't.

$(your_user_name)
He is in England.

$(your_user_name)
Have you forgot about it?

$(your_user_name)>>>
```

## Architecture

*This project will require 2 parts to be programmed. The server part, which will save all the data (messages, users...), will have to be able to communicate with database and answer requests recieved from port. The user part will send request to the server and will print the answers in described way.*

## Development & Testing
**Build system:** *CMake*

**Testing Framework:** *None from the complexity of usage of the program*

**Required libraries/frameworks:** *SFML*

*Handling recieved packets, printing outputs for given input*

## Acceptance criteria

**Minimum Viable Product:** *Working soulution with one server, and show given number of messages*

**Extensions:** *More servers communicating together (regular sync), more messages while not donwloading all of them if not necessary, possibility to reply to messages*

## Instalation

Pull the repository:

```bash
git pull https://github.com/RicMuz/terminal-messenger.git
```

For the installation user needs to install CMake. After Cmake is installed follow these steps:

```bash
mkdir build && cd build
```

After that use the CMake (user should be inside of build dir):

```bash
cmake ..
```

This step will take a while, because cmake has to pull and install sfml libraries and generate MakeFiles for the project.

```bash
make .
```

The executables can be found in directories build/Server and build/Client as server and client.