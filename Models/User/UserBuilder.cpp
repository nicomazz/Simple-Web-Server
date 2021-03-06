//
// Created by nicomazz97 on 20/11/16.
//

#include <DataSources/UserPreferenceProvider.hpp>
#include "UserBuilder.hpp"


User UserBuilder::buildFromJson(std::string json_string) {
    try {
        json json_content = json::parse(json_string);
        User u;
        u.id = get<long>(json_content,USER_ID);
        u.secretKey = get<std::string>(json_content,USER_SECRET_KEY);
        if (u.id == NEW_USER_DEFAULT_ID)
            u.secretKey = generateRandomString();
        u.firebaseID = get<std::string>(json_content,USER_ID_FIREBASE);
        u.lat = get<double>(json_content,USER_LAT);
        u.lng = get<double>(json_content,USER_LNG);
        u.minMagPreference = get<double>(json_content,USER_MIN_MAG);
        u.maxDistancePreference = get<double>(json_content,USER_MAX_DIST);
        u.minMillisNotificationDelay = get<long>(json_content,USER_DELAY_NOTIFICATION);
        u.lastNotificationMillis = 0;
        u.lastModify = TimeUtility::getCurrentMillis();
        u.lastActivity = TimeUtility::getCurrentMillis();
        u.receiveRealTimeNotification = get<bool>(json_content,USER_RECEIVE_TEST);
        try { // it can not have username
            u.username = get<std::string>(json_content, USER_USERNAME);
        } catch (...) {}
        if (u.hasId())
            addDBFields(u);
        return u;
   } catch (std::logic_error e) {
        syslog(LOG_INFO, e.what());
        throw std::invalid_argument("json string with bad format, cannot parse the user. "+string(e.what()));
    }
}
void UserBuilder::addDBFields(User &user) {
    try {
        UserPreferenceProvider::checkValidUserInDB(user);
        User inDb = UserPreferenceProvider::getUser(user.id);
        user.lastNotificationMillis = inDb.lastNotificationMillis;
        user.lastActivity = inDb.lastActivity;
    } catch (...){}
}

json UserBuilder::userToJson(User &u) {
    json json_content;
    json_content[USER_ID] = u.id;
    json_content[USER_LAT] = u.lat;
    json_content[USER_LNG] = u.lng;
    json_content[USER_LAST_ACTIVITY] = u.lastActivity;
    json_content[USER_USERNAME] = u.username;
    return json_content;
}

std::string UserBuilder::generateRandomString(int l) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(97, 126); // printable ascii chars
    std::string result;
    result.resize(l);
    for (auto &c : result)
        c = (char) dis(gen);
    return result;
}

template<typename T>
T UserBuilder::get(json j, std::string key) {
    try {
        T val = j[key].get<T>();
        return val;
    } catch (std::logic_error e){
        stringstream ss;
        ss<<"Missing value for key: "<<key;
        throw std::invalid_argument(ss.str());
    }
}

std::vector<long> UserBuilder::getUserIdList(string json_array) {
    try {
        json json_content = json::parse(json_array);
        User u;
        u.id = get<long>(json_content,USER_ID);
        u.secretKey = get<std::string>(json_content,USER_SECRET_KEY);
        UserPreferenceProvider::checkValidUserInDB(u); // if not valid throw exception

        json array_ids = json::parse(json_content["ids"].get<std::string>());
        vector<long> res;
        for (int id : array_ids) {
            res.push_back(id);
        }
        return res;
    } catch (std::logic_error e) {
        syslog(LOG_INFO, e.what());
        throw std::invalid_argument("json string with bad format, cannot parse the users list. "+string(e.what()));
    }
}



