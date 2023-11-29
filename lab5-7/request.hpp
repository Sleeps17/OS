#ifndef ACTION_HPP
#define ACTION_HPP

#include <string>

enum action_t {
    Time,
    Start,
    Stop,
    Ping,
    Unknown
};

struct Request {

public:
    action_t action;
    Request(): action(Unknown){}
    explicit Request(const std::string& str) {
        if (str == "time") {
            action = Time;
        } else if (str == "start") {
            action = Start;
        } else if (str == "stop") {
            action = Stop;
        } else if(str == "ping") {
            action = Ping;
        } else {
            action = Unknown;
        }
    }
    ~Request() = default;

    explicit operator std::string() const {
        switch (action) {
            case Time:
                return "time";
            case Start:
                return "start";
            case Stop:
                return "stop";
            default:
                return "unknown";
        }
    }
};


#endif //ACTION_HPP
