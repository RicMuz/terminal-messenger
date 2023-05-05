enum type_of_request {
    sign_up,
    log_in,
    log_out,
    add_friend,
    open_chat,
    send_message,
    list_friends,
    exit_chat = 100
};

enum return_code {
    success,
    user_exists,
    wrong_password,
    user_does_not_exist,
    unauthorized_access,
    self_friend,
    non_existing_friend,
    not_friends
};