#ifndef DATA_HPP
#define DATA_HPP

#include "utils.hpp"

template<typename KEY_TYPE>
struct Data {
    KEY_TYPE key;

    Data(){}

    Data(KEY_TYPE _key) {
        this->key = _key;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Data<KEY_TYPE>& data) {
        stream<<" | key: "<<data.key;
        return stream;
    }

    bool operator==(const Data<KEY_TYPE>& other) const {
        return this->key == other.key;
    }

    bool operator<(const Data<KEY_TYPE>& other) const {
        return this->key < other.key;
    }
    
    bool operator<=(const Data<KEY_TYPE>& other) const {
        return this->key <= other.key;
    }

    bool operator>(const Data<KEY_TYPE>& other) const {
        return this->key > other.key;
    }

    bool operator>=(const Data<KEY_TYPE>& other) const {
        return this->key >= other.key;
    }
};

#endif // DATA_HPP