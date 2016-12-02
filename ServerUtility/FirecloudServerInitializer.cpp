//
// Created by nicomazz97 on 26/11/16.
//

#include <Firebase/NotificationDataBuilder.hpp>
#include <Detector/ReportParserHTTP.hpp>
#include "FirecloudServerInitializer.hpp"


void FCMServer::default_resource_send(const HttpServer &server,
                                      const shared_ptr<HttpServer::Response> &response,
                                      const shared_ptr<ifstream> &ifs) {
    //read and send 128 KB at a time
    static vector<char> buffer(131072); // Safe when server is running on one thread
    streamsize read_length;
    if ((read_length = ifs->read(&buffer[0], buffer.size()).gcount()) > 0) {
        response->write(&buffer[0], read_length);
        if (read_length == static_cast<streamsize>(buffer.size())) {
            server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
                if (!ec)
                    default_resource_send(server, response, ifs);
                else
                    cerr << "Connection interrupted" << endl;
            });
        }
    }
}

void FCMServer::initServer(SimpleWeb::Server<SimpleWeb::HTTP> &server) {

    /**
     * handle request to add/update user info.
     * the respose is a json with the id of the new/updated user
     */
    server.resource["^/addUser"]["POST"] = [](shared_ptr<HttpServer::Response> response,
                                              shared_ptr<HttpServer::Request> request) {
        try {

            string content = request->content.string();
            User user = UserBuilder::buildFromJson(content);
            std::string esit;
            if (user.id < 0) // non esisteva!
                esit = "################################################################# New user created";
            else
                esit = "################################################################# User updated";
            syslog(LOG_INFO, esit.c_str());

            long newId = UserPreferenceProvider().handleNewUserRequest(user);
            stringstream ss;
            ss << "{ \"id\":" << newId << "}";
            std::string resp = ss.str();

            *response << "HTTP/1.1 200 OK\r\n"
                      << "Content-Type: application/json\r\n"
                      << "Content-Length: " << resp.length() << "\r\n\r\n"
                      << resp;
        }
        catch (exception &e) {
            string resp(e.what());
            syslog(LOG_INFO, e.what());
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << resp.length() << "\r\n\r\n"
                      << resp;
        }
    };

    //GET-example for the path /info
    //Responds with request-information
    server.resource["^/info$"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                           shared_ptr<HttpServer::Request> request) {
        stringstream content_stream;
        content_stream << "<h1>Request from " << request->remote_endpoint_address << " ("
                       << request->remote_endpoint_port << ")</h1>";
        content_stream << request->method << " " << request->path << " HTTP/" << request->http_version << "<br>";
        for (auto &header: request->header) {
            content_stream << header.first << ": " << header.second << "<br>";
        }

        //find length of content_stream (length received using content_stream.tellp())
        content_stream.seekp(0, ios::end);

        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n"
                  << content_stream.rdbuf();
    };
    //GET-example for the path /info
    //Responds with request-information
    server.resource["^/users"]["GET"] = [](shared_ptr<HttpServer::Response> response,
                                           shared_ptr<HttpServer::Request> /*request*/) {
        stringstream content_stream;
        UserPreferenceProvider userProvider;
        std::vector<User> allUsers = userProvider.requestUsersFromDB();
        json jsonObj;
        for (User &u : allUsers)
            jsonObj.push_back(UserBuilder::userToJson(u));

        content_stream << jsonObj.dump(3);

        //find length of content_stream (length received using content_stream.tellp())
        content_stream.seekp(0, ios::end);

        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n"
                  << content_stream.rdbuf();
    };
    //GET-example for the path /match/[number], responds with the matched string in path (number)
    //For instance a request GET /match/123 will receive: 123
    server.resource["^/user/([0-9]+)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response,
                                                           shared_ptr<HttpServer::Request> request) {
        thread work_thread([response, request] {
            try {
                int user_id = std::stoi(request->path_match[1]);

                stringstream content_stream;
                UserPreferenceProvider userProvider;
                User user = userProvider.getUser(user_id);
                json jsonObj = UserBuilder::userToJson(user);

                content_stream << jsonObj.dump(3);

                //find length of content_stream (length received using content_stream.tellp())
                content_stream.seekp(0, ios::end);

                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n"
                          << content_stream.rdbuf();
            }
            catch (exception &e) {
                string resp(e.what());
                syslog(LOG_INFO, e.what());
                *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << resp.length() << "\r\n\r\n"
                          << resp;
            }
        });
        work_thread.detach();
    };
    //GET-example for the path /match/[number], responds with the matched string in path (number)
    //For instance a request GET /match/123 will receive: 123
    server.resource["^/match/([0-9]+)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response,
                                                            shared_ptr<HttpServer::Request> request) {
        string number = request->path_match[1];
        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << number.length() << "\r\n\r\n" << number;
    };

    //Get example simulating heavy work in a separate thread
    server.resource["^/work$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response,
                                                  shared_ptr<HttpServer::Request> /*request*/) {
        thread work_thread([response] {
            this_thread::sleep_for(chrono::seconds(5));
            string message = "Work done";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << message.length() << "\r\n\r\n" << message;
        });
        work_thread.detach();
    };

    server.resource["^/report"]["POST"] = [](shared_ptr<HttpServer::Response> response,
                                             shared_ptr<HttpServer::Request> request) {
        try {
            static SimpleEQDetector detector;
            string content = request->content.string();
            Report r = ReportParserHTTP::parseRequest(content);

            thread work_thread([response, r] {
                string message = "Report send!";
                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << message.length() << "\r\n\r\n" << message;

                detector.addReports(r);
            });
            work_thread.detach();
        } catch (exception &e) {
            string resp(e.what());
            syslog(LOG_INFO, e.what());
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << resp.length() << "\r\n\r\n"
                      << resp;
        }
    };

    //Default GET-example. If no other matches, this anonymous function will be called.
    //Will respond with content in the web/-directory, and its subdirectories.
    //Default file: index.html
    //Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    server.default_resource["GET"] = [&server](shared_ptr<HttpServer::Response> response,
                                               shared_ptr<HttpServer::Request> request) {
        *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << 0 << "\r\n\r\n";
        return;
        try {
            auto web_root_path = boost::filesystem::canonical("web");
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            //Check if path is within web_root_path
            if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
                !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw invalid_argument("path must be within root path");
            if (boost::filesystem::is_directory(path))
                path /= "index.html";
            if (!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
                throw invalid_argument("file does not exist");

            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary);

            if (*ifs) {
                ifs->seekg(0, ios::end);
                auto length = ifs->tellg();

                ifs->seekg(0, ios::beg);

                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << length << "\r\n\r\n";
                default_resource_send(server, response, ifs);
            } else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e) {
            string content = "Could not open path " + request->path + ": " + e.what();
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n"
                      << content;
        }
    };

}

