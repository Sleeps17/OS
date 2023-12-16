#include <iostream>
#include <zmq.hpp>
#include <nlohmann/json.hpp>
#include <csignal>
#include <thread>
#include <cstdlib>


#include "topology.hpp"
#include "request.hpp"
#include "response.hpp"
#include "myMQ.hpp"

zmq::context_t context(1);
zmq::socket_t socket(context, zmq::socket_type::pair);
bool connected = false;

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
    while(std::cout << "> " && std::cin >> action) {
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

            if (!connected) {
                pid_t pid = fork();

                if (pid == -1) {
                    std::cout << "Error: fork failed" << std::endl;
                    continue;
                } else if (pid == 0) {
                    execl("./bin/calculation", "calculation", std::to_string(id).c_str(), std::to_string(1).c_str(), nullptr);
                } else {
                    socket.connect(MakeConnPort(id));
                    connected = true;
                    std::cout << "OK: " << pid << std::endl;
                }
            } else {
                auto path = topology.findPathToNode(id);
                reverse(path.begin(), path.end());
                path.pop_back();

                Request req(Create, path, id);
                nlohmann::json jsonReq = {
                        {"action", req.action},
                        {"path", req.path},
                        {"id", req.id},
                        {"maxDepth", topology.maxDepth()}
                };

                std::string jsonReqString = jsonReq.dump();
                zmq::message_t msg(jsonReqString.begin(), jsonReqString.end());
                socket.send(msg, zmq::send_flags::none);

                std::this_thread::sleep_for(topology.maxDepth() * std::chrono::milliseconds(50));

                zmq::message_t reply;
                bool replyed;
                try {
                    replyed = socket.recv(&reply, ZMQ_DONTWAIT);
                } catch(zmq::error_t& e) {
                    replyed = false;
                }


                if (replyed) {
                    std::string replyStr = std::string(static_cast<char*>(reply.data()), reply.size());

                    nlohmann::json jsonReply = nlohmann::json::parse(replyStr);

                    Response resp;

                    resp.status = jsonReply.at("status");

                    if (resp.status == ERROR) {
                        std::string error = jsonReply["error"];
                        std::cout << error << std::endl;
                    } else if (resp.status == OK) {
                        int pid = jsonReply["pid"];
                        std::cout << "OK: " << pid << std::endl;
                    }
                } else {
                    std::cout << "Error: node with id = " << id << " not found" << std::endl;
                    continue;
                }
            }

        }
        else if (action == "exec") {
            int id = readID();
            if(id == -1) {
                std::cout << "Error: invalid id" << std::endl;
                continue;
            }

            std::string command;
            std::cin >> command;

            if(command != "time" && command != "start" && command != "stop") {
                std::cout << "Error: invalid subcommand" << std::endl;
                continue;
            }

            bool finded = topology.search(id);
            if (!finded) {
                std::cout << "Error: node with id = " << id << " not found" << std::endl;
                continue;
            }

            auto path = topology.findPathToNode(id);
            reverse(path.begin(), path.end());
            path.pop_back();

            Request req(command, path, id);
            nlohmann::json jsonReq = {
                    {"action", req.action},
                    {"path", req.path},
                    {"id", req.id},
                    {"maxDepth", topology.maxDepth()}
            };

            std::string jsonReqString = jsonReq.dump();
            zmq::message_t msg(jsonReqString.begin(), jsonReqString.end());
            socket.send(msg, zmq::send_flags::none);

            std::this_thread::sleep_for(topology.maxDepth() * std::chrono::milliseconds(topology.maxDepth()*50));

            zmq::message_t reply;
            bool replyed;
            try {
                replyed = socket.recv(&reply, ZMQ_DONTWAIT);
            } catch(zmq::error_t& e) {
                replyed = false;
            }

            Response resp;
            nlohmann::json jsonReply;

            if (replyed) {
                std::string replyStr = std::string(static_cast<char*>(reply.data()), reply.size());
                jsonReply = nlohmann::json::parse(replyStr);
            } else {
                std::cout << "Error : node with id = " << id << " not available" << std::endl;
                continue;
            }


            resp.status = jsonReply.at("status");
            if (resp.status == ERROR) {
                std::string error = jsonReply["error"];
                std::cout << error << std::endl;
            } else if (resp.status == OK) {
                std::cout << "OK: " << id;
                if (req.action == Time) {
                    std::cout << " " << jsonReply["result"] << "ms";
                }
                std::cout << std::endl;
            }
        } else if (action == "pingall") {

        } else if(action == "remove") {

        } else {
            std::cout << "Unknown command" << std::endl;
        }
    }
    return 0;
}