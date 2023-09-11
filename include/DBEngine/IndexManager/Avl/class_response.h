#ifndef RESPONSE_H
#define RESPONSE_H
#include "utils.h"

struct Response
{
    std::vector<posType> records;
    time_t query_time;

    Response(){}

    void startTimer() { this->query_time = time(NULL); }
    void stopTimer() {this->query_time = time(NULL) - this->query_time;}
};

#endif // RESPONSE_H