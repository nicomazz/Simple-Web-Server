//
// Created by nicomazz97 on 26/11/16.
//

#ifndef SIMPLE_WEB_SERVER_EARTQUAKESERVER_H
#define SIMPLE_WEB_SERVER_EARTQUAKESERVER_H

#include <syslog.h>

#include <DataSources/EventProvider.hpp>
#include <Firebase/FirebaseNotificationManager.hpp>
#include <ServerUtility/client_http.hpp>
#include <ServerUtility/server_http.hpp>
#include <ServerUtility/FirecloudServerInitializer.hpp>

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

class EarthquakeServer {
public:
    void startServer();
private:
    void serverMainLoop();
};


#endif //SIMPLE_WEB_SERVER_EARTQUAKESERVER_H