#ifndef DATA_H
#define DATA_H

struct Data
{
    int numero;
    
    bool operator==(const Data& otherData){ return numero == otherData.numero; }
    bool operator<(const Data& otherData){ return numero < otherData.numero; }
    bool operator>(const Data& otherData){ return numero > otherData.numero; }
    bool operator>=(const Data& otherData){ return numero >= otherData.numero; }
    bool operator<=(const Data& otherData){ return numero <= otherData.numero; }
};

#endif // DATA_H