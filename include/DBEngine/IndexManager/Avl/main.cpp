#include <iostream>
#include "class_avl_index.h"

int main()
{
    AVLIndex index("avlIndex.dat");
    Data item1, item2, item3, item4, item5, item6, item7, item8, item9, item10;
    item1.dato = 1;
    item2.dato = 2;
    item3.dato = 3;
    item4.dato = 4;
    item5.dato = 5;
    item6.dato = 6;
    item7.dato = 7;
    item8.dato = 8;
    item9.dato = 9;


    index.insert(item1);
    index.insert(item2);
    index.insert(item3);
    index.insert(item4);
    index.insert(item5);
    index.insert(item6);
    index.insert(item7);
    index.insert(item8);
    index.insert(item9);

    index.displayPretty();

    std::vector<AVLIndexNode> vector;

    vector = index.searchIndexsByRange(item3, item8);

    for (int i = 0; i < vector.size(); i++)
    {
        std::cout << vector[i];
    }
    

    // index.removeIndex(item1);
    // index.removeIndex(item3);
    // index.removeIndex(item6);
    // index.removeIndex(item4);

    // index.displayPretty();

    return 0;
}