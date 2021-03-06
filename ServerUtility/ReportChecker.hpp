//
// Created by nicomazz97 on 04/02/17.
//

#ifndef SIMPLE_WEB_SERVER_REPORTCHECKER_HPP
#define SIMPLE_WEB_SERVER_REPORTCHECKER_HPP

#include <Utility/TimeUtility.hpp>
#include <Models/Event/Event.hpp>
#include <DataSources/EventProvider.hpp>
#include <DataSources/ReportProvider.hpp>

class ReportChecker {
public:
    /**
     * search the number of reports related events in interval -minutes*2, -minutes
     * @param minutes
     */
    static void checkEventRelatedReport(int minutes);

    /**
     * reupdate the number of reports for every events in db
     */
    static void checkAllEventRelatedReport();

    static void updateEventReportsNumber(Event &e);
};


#endif //SIMPLE_WEB_SERVER_REPORTCHECKER_HPP
