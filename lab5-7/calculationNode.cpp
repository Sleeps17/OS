#include <fstream>
#include <zmq.hpp>
#include<nlohmann/json.hpp>
#include<chrono>

#include "myMQ.hpp"
#include "request.hpp"
#include "response.hpp"

using namespace std;

std::chrono::system_clock::time_point start;
std::chrono::system_clock::time_point stop;
int countStopMilliseconds = -1;
bool stopped = false;
bool started = false;

#define PROD

using Error = const char*;


Error StartTimer() {
    if(started && !stopped) {
        return "Timer is already running";
    }
    if(started && stopped) {
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> elapsed = now - stop;
        countStopMilliseconds += int(elapsed.count());
    }
    if(!started) {
        start = std::chrono::system_clock::now();
        countStopMilliseconds = -1;
    }
    stopped = false;
    started = true;

    return nullptr;
}

Error StopTimer() {
    if(!started) {
        return "Timer has not been started yet";
    }

    if(stopped) {
        return "Timer is already stopped";
    }

    if(countStopMilliseconds == -1) {
        countStopMilliseconds = 0;
    }
    stop = std::chrono::system_clock::now();
    stopped = true;

    return nullptr;
}

int GetTimer() {
    int result;
    if(countStopMilliseconds == -1) {
        if(started) {
            auto now = std::chrono::system_clock::now();
            std::chrono::duration<double, std::milli> elapsed = now - start;
            result = int(elapsed.count());
        } else {
            result = 0;
        }
    } else {
        if (stopped) {
            std::chrono::duration<double, std::milli> elapsed = stop - start;
            result = int(elapsed.count());
            result -= countStopMilliseconds;
        } else {
            auto now = std::chrono::system_clock::now();
            std::chrono::duration<double, std::milli> elapsed = now - start;
            result = int(elapsed.count());
            result -= countStopMilliseconds;
        }
    }
    return result;
}

std::string HandleStart() {
    Response resp;
    nlohmann::json jsonResp;

    auto err = StartTimer();
    if(err != nullptr) {
        resp.status = ERROR;
        resp.error = std::string(err);
        jsonResp = {
                {"status", resp.status},
                {"error", resp.error}
        };
    } else {
        resp.status = OK;
        jsonResp = {
                {"status", resp.status},
        };
    }

    return jsonResp.dump();
}

std::string HandleStop() {
    Response resp;
    nlohmann::json jsonResp;

    auto err  = StopTimer();
    if(err != nullptr) {
        resp.status = ERROR;
        resp.error = std::string(err);

        jsonResp = {
                {"status", resp.status},
                {"error", resp.error}
        };
    } else {
        resp.status = OK;

        jsonResp = {
                {"status", resp.status},
        };
    }

    return jsonResp.dump();
}

std::string HandleTime() {
    Response resp;

    resp.result = GetTimer();
    resp.status = OK;

    nlohmann::json jsonResp = {
            {"status", resp.status},
            {"result", resp.result}
    };

    return jsonResp.dump();
}

std::string HandlePing() {
    Response resp;
    resp.status = OK;

    nlohmann::json jsonResp = {
            {"status", resp.status}
    };

    return jsonResp.dump();
}

std::string HandleUnknown() {
    Response resp;
    resp.status = ERROR;
    resp.error = "Unknown subcommand";

    nlohmann::json jsonResp = {
            {"status", resp.status},
            {"error", resp.error}
    };

    return jsonResp.dump();
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        return 1;
    }

    int id = std::stoi(argv[1]);

    // Initialize ZeroMQ context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::pair);
    socket.bind(MakeBindPort(id).c_str());

    // Receive message from parent process
    zmq::message_t request;
    while(socket.recv(&request)) {
        std::string jsonReqString = std::string(static_cast<char*>(request.data()), request.size());

        // Deserialize JSON string to data structure
        nlohmann::json jsonData = nlohmann::json::parse(jsonReqString);
        Request req;
        req.action = jsonData["action"];

        #ifdef DEBUG
        std::cout << "Received message from parent process: " << req.action << std::endl;
        #endif

        std::string jsonRespString;

        if(req.action == Time) {
            jsonRespString = HandleTime();
        } else if(req.action == Start) {
            jsonRespString = HandleStart();
        } else if (req.action == Stop) {
            jsonRespString = HandleStop();
        } else if(req.action == Ping) {
            jsonRespString = HandlePing();
        }else {
            jsonRespString = HandleUnknown();
        }

        // Send response back to parent process
        zmq::message_t reply(jsonRespString.begin(), jsonRespString.end());
        socket.send(reply, zmq::send_flags::none);
    }

    // Close the socket and context
    socket.close();
    context.close();
    return 0;
}