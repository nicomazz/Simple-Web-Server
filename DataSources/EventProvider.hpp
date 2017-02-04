#ifndef EVENT_DATA_SOURCE_HPP
#define    EVENT_DATA_SOURCE_HPP


#include <vector>
#include <memory>
#include "Models/Event/Event.hpp"
#include <iostream>
#include <sqlite3.h>
#include "DataSources/EventsWebProvider/INGVDataSource.hpp"
#include "Models/Event/Event-odb.hpp"

#include "Database.hpp"

class EventProvider {
public:
    EventProvider() {}

    /**
     * update the database from the web data sources (INGV, USGS)
     * @return
     */
    static std::vector<Event> requestEventWebUpdate();

    static std::vector<Event> requestNewEventNotInDB();

    static std::vector<Event> requestEventFromDB();

    static std::vector<Event> requestEventsInInterval(long from_millis, long to_millis);

    /**
     * @return all the events that has at least one report associated
     */
    static std::vector<Event> requestDetectedEvents();
    static std::vector<Event> requestDetectedEvents(int from_millis, int to_millis);

    static long persistEvent(Event &e, bool checkAlreadyPresent = false);

    static Event getEvent(long id);

    static void deleteEvent(Event e);

    static bool isEventPresent(long id);

    static void updateEvent(Event & e);
    /**
     * erase old (2 days ago) event that are not real time ones and not have related reports;
     * */
    static void eraseOldEvents();

};

#endif	/* EVENT_DATA_SOURCE_HPP */
