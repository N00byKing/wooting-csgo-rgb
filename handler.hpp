#pragma once

class KeyboardHandler {
    public:
        static void wooting_handle_event(std::string);
        static void wooting_exit();

    private:
        static int bombCounter;

        static void wooting_set_arrowkeys(int, int, int);
};