#pragma once

class App {
public:
    int run();

private:
    void startup();
    void shutdown();

    struct State;
    State *state_{nullptr};
};
