### Both
- [X] wrong packet dealing MAJOR ERROR

### Server
- [X] answer packet spaces betweem data and return code !!!!
- [X] file handling (all closes, good file names, create dirs)
- [X] add more log prints
- [X] create friend_list file when sign up
- [X] create chat file when adding friends
- [X] repetetive code (return code 4)
- [X] send message and open chat without friendlist (check if the guy is in the friend list, if yes than he exists because existance was controlled while adding)
- [X] control if answer packets are send back
- [X] names of files can be const strings 
- [] after client disconnets server seems to freeze MAJOR ERROR

### Client
- [X] check if all neccessary data are send (especially usr and pwd), and after login usr who is logged in as the first after request code 
- [] when user enters login/signup/add/open... he can't get back -> add back (return codes will be needed)