#include <fstream>
#include <zmq.hpp>
#include<nlohmann/json.hpp>
#include<chrono>
#include <thread>
#include<iostream>

#include "myMQ.hpp"
#include "request.hpp"
#include "response.hpp"
#include "timer.hpp"

int id, depth;

int leftSonId;
int rightSonId;

zmq::context_t ParentContext(1);
zmq::socket_t ParentSocket(ParentContext, zmq::socket_type::pair);

zmq::context_t LeftSonContext(1);
zmq::socket_t LeftSonSocket(LeftSonContext, zmq::socket_type::pair);

zmq::context_t RightSonContext(1);
zmq::socket_t RightSonSocket(RightSonContext, zmq::socket_type::pair);

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

std::string HandleCreate(nlohmann::json& jsonData, Request& req) {
    req.path = std::vector<int>(jsonData.at("path"));
    for(auto& elem : req.path) {
        std::cout << "PATH: " << elem << std::endl;
    }
    req.id = jsonData.at("id");
    req.maxDepth = jsonData.at("maxDepth");

    Response resp;
    nlohmann::json jsonResp;


    if (req.path.size() == 1) {
        std::cout << "HELLO I AM PARENT OF NEW NODE, MY ID " << id << std::endl;
        pid_t pid = fork();

        if (pid == -1) {
            resp.status = ERROR;

            jsonResp = {
                    {"status", resp.status},
                    {"error", "forked error"}
            };

            return jsonResp.dump();
        } else if (pid == 0) {
            execl("./bin/calculation", "./calculation", std::to_string(req.id).c_str(), std::to_string(depth+1).c_str(), nullptr);
        } else {

            if (req.id < id) {
                LeftSonSocket.connect(MakeConnPort(req.id).c_str());
                leftSonId = req.id;
            } else {
                RightSonSocket.connect(MakeConnPort(req.id).c_str());
                rightSonId = req.id;
            }

            resp.status = OK;
            resp.pid = pid;

            jsonResp = {
                    {"status", resp.status},
                    {"pid", resp.pid}
            };

            return jsonResp.dump();
        }
    }
    else {
        Request reqToSon;
        reqToSon.action = Create;
        req.path.erase(req.path.begin());
        reqToSon.path = req.path;
        reqToSon.id = req.id;
        reqToSon.maxDepth = req.maxDepth;

        nlohmann::json jsonReq = {
                {"action", reqToSon.action},
                {"path", reqToSon.path},
                {"id", reqToSon.id},
                {"maxDepth", reqToSon.maxDepth}
        };
        std::string jsonReqString = jsonReq.dump();

        zmq::message_t message(jsonReqString.begin(), jsonReqString.end());

        zmq::message_t reply;
        if (leftSonId == reqToSon.path[0]) {
            std::cout << "HELLO I AM LEFT SON WITH ID: " << leftSonId << std::endl;
            LeftSonSocket.send(message, zmq::send_flags::none);

            std::this_thread::sleep_for(std::chrono::milliseconds(req.maxDepth - depth + 1));

            bool replyed = true;

            try {
                LeftSonSocket.recv(&reply, ZMQ_DONTWAIT);
            } catch(const zmq::error_t& e) {
                resp.status = ERROR;
                resp.error = std::string(e.what());

                replyed = false;
            }

            if (replyed) {
                std::string jsonRespString = std::string(static_cast<char*>(reply.data()), reply.size());
                jsonResp = nlohmann::json::parse(jsonRespString);
            }

            return jsonResp.dump();

        } else if (rightSonId == reqToSon.path[0]) {
            std::cout << "HELLO I AM RIGHT SON WITH ID: " << rightSonId << std::endl;
            RightSonSocket.send(message, zmq::send_flags::none);

            std::this_thread::sleep_for(std::chrono::milliseconds(req.maxDepth - depth + 1));

            bool replyed = true;

            try {
                RightSonSocket.recv(&reply, ZMQ_DONTWAIT);
            } catch(const zmq::error_t& e) {
                resp.status = ERROR;
                resp.error = std::string(e.what());

                replyed = false;
            }

            if (replyed) {
                std::string jsonRespString = std::string(static_cast<char*>(reply.data()), reply.size());
                jsonResp = nlohmann::json::parse(jsonRespString);
            }

            std::cout << "I reply message, my id " << id << std::endl;

            return jsonResp.dump();
        }
    }

    return "";
}

int main(int argc, char* argv[]) {

    if(argc < 2) {
        return 1;
    }

    id = std::stoi(argv[1]);
    depth = std::stoi(argv[2]);


    ParentSocket.bind(MakeBindPort(id).c_str());

    zmq::message_t request;
    while(ParentSocket.recv(&request)) {
        std::string jsonReqString = std::string(static_cast<char*>(request.data()), request.size());

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
        } else if (req.action == Create) {
            jsonRespString = HandleCreate(jsonData, req);
        } else {
            jsonRespString = HandleUnknown();
        }

        zmq::message_t reply(jsonRespString.begin(), jsonRespString.end());
        ParentSocket.send(reply, zmq::send_flags::none);
    }

    ParentSocket.close();
    ParentContext.close();
    return 0;
}
