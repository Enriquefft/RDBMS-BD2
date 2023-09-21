#ifndef RECORD_HPP
#define RECORD_HPP

#include "data.hpp"

template<typename KEY_TYPE>
class Record {
public:
    Data<KEY_TYPE> data;

    physical_pos raw_pos;
    physical_pos dup_pos;

    void setData(Data<KEY_TYPE> data) {
        this->data = data;
    }

    void setRawPos(physical_pos _raw_pos) {
        this->raw_pos = _raw_pos;
    }

    void setDupPos(physical_pos _dup_pos) {
        this->dup_pos = _dup_pos;
    }
};

#endif // RECORD_HPP