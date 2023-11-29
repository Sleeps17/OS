#include <iostream>
#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <utility>
#include <csignal>
#include <thread>
#include <cstdlib>


#include "topology.hpp"
#include "request.hpp"
#include "response.hpp"
#include "myMQ.hpp"

#define PROD

bool keepRunning = true;
std::unordered_map<int, std::pair<zmq::context_t, zmq::socket_t>> childs;
std::unordered_map<int, pid_t> pids;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Received SIGINT (Ctrl+C)" << std::endl;
        keepRunning = false;
        for (auto pid : pids) {
            kill(pid.second, SIGINT);
        }
    }
    std::cout << "KILL ALL PROCESSES" << std::endl;
    exit(EXIT_SUCCESS);
}

int readID() {
    std::string id_str;
    std::cin >> id_str;
    char* end;
    int id = (int)strtol(id_str.c_str(), &end, 10);
    if ( (*end != '\0' && *end != '\n') || id < 0) {
        return -1;
    }
    return id;
}

int main() {

    signal(SIGINT, signalHandler);

    std::cout << "\t\tUSAGE" << std::endl;
    std::cout << "Creating a new node: create <id>" << std::endl;
    std::cout << "Execution of a command on a computing node: exec <id> <subcommand>" << std::endl;
    std::cout << " Subcommands:" << std::endl;
    std::cout << "  - time: get current time" << std::endl;
    std::cout << "  - start: start timer" << std::endl;
    std::cout << "  - stop: stop timer" << std::endl;
    std::cout << "Output of all unavailable nodes: pingall" << std::endl;
    std::cout << "Remove node: remove <id>" << std::endl;
    std::cout << std::endl;

    Topology<int> topology;

    std::string action;
    while(keepRunning && std::cout << "> " && std::cin >> action) {
        if(action == "create") {

            int id = readID();
            if(id == -1) {
                std::cout << "Error: invalid id" << std::endl;
                continue;
            }

            Error err = topology.insert(id);
            if (err != nullptr) {
                std::cout << err << std::endl;
                continue;
            }

            zmq::context_t context(1);
            zmq::socket_t socket(context, zmq::socket_type::pair);
            socket.connect(MakeConnPort(id).c_str());
            childs[id] = std::make_pair(std::move(context), std::move(socket));

            pid_t pid = fork();
            if(pid == -1) {
                std::cout << "Error: can't create process" << std::endl;
                continue;
            }
            if(pid == 0) {
                execl("./bin/calculation", "calculation", std::to_string(id).c_str(), nullptr);
            } else {
                pids[id] = pid;
                std::cout << "OK: " << pid << std::endl;
            }
        } else if (action == "exec") {
            int id = readID();
            if(id == -1) {
                std::cout << "Error: invalid id" << std::endl;
                continue;
            }
            std::string command;
            std::cin >> command;

            bool find = topology.search(id);
            if (!find) {
                std::cout << "Error: node " << id << " not found" << std::endl;
                continue;
            }

            Request req(command);
            nlohmann::json jsonReq = {
                    {"action", req.action}
            };
            std::string jsonStringReq = jsonReq.dump();
            zmq::message_t msg(jsonStringReq.begin(), jsonStringReq.end());

            childs[id].second.send(msg, zmq::send_flags::dontwait);

            zmq::message_t reply;
            childs[id].second.recv(&reply);
            std::string jsonStringResp = std::string(static_cast<char*>(reply.data()), reply.size());

            #ifdef DEBUG
            std::cout << "Received message from parent child: " << jsonStringResp << std::endl;
            #endif

            nlohmann::json jsonReply = nlohmann::json::parse(jsonStringResp);
            Response resp;
            resp.status = jsonReply["status"];
            if(resp.StatusOK()) {
                if(req.action == Time) {
                    int result = jsonReply["result"];
                    std::cout << "OK time: " << id << ": " << result << std::endl;
                } else {
                    std::cout << "OK: " << id << std::endl;
                }
            } else {
                std::string error = jsonReply["error"];
                std::cout << "ERROR: " << error << std::endl;
            }
        } else if (action == "pingall") {
            int cnt = 0;
            for(auto & child : childs) {
                Request req;
                req.action = Ping;
                nlohmann::json jsonReq = {
                        {"action", req.action}
                };
                std::string jsonStringReq = jsonReq.dump();
                zmq::message_t msg(jsonStringReq.begin(), jsonStringReq.end());

                child.second.second.send(msg, zmq::send_flags::dontwait);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                zmq::message_t reply;
                bool replyed = true;
                try {
                    child.second.second.recv(&reply, ZMQ_DONTWAIT);
                } catch(const zmq::error_t& e) {
                    cnt++;
                    replyed = false;
                }

                if(replyed) {
                    std::string jsonStringResp = std::string(static_cast<char*>(reply.data()), reply.size());

                    #ifdef DEBUG
                    std::cout << "Received message from parent child: " << jsonStringResp << std::endl;
                    #endif

                    nlohmann::json jsonReply = nlohmann::json::parse(jsonStringResp);
                    Response resp;
                    resp.status = jsonReply["status"];
                    if(!resp.StatusOK()) {
                        cnt++;
                    }
                }
            }
            std::cout << "OK: " << (cnt == 0 ? -1 : cnt)<< std::endl;
        } else if(action == "remove") {
            int id = readID();

            auto err = topology.remove(id);
            if (err != nullptr) {
                std::cout << err << std::endl;
            } else {
                pid_t pid = pids[id];
                kill(pid, SIGINT);
                std::cout << "OK: " << id << std::endl;
            }
        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }

    signalHandler(2);
    return 0;
}