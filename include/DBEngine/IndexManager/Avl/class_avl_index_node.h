#ifndef AVL_INDEX_NODE_H
#define AVL_INDEX_NODE_H

#include "utils.h"

struct AVLIndexNode
{
    Data data;
    posType posRecord;

    posType leftChildren = -1;
    posType rightChildren = -1;

    posType nextDelete = -1;

    int height = 0;

    friend std::ostream& operator<<(std::ostream& os, const AVLIndexNode& node)
    {
        os << "Data: " << node.data.numero << "\n";
        os << "Left Children: " << node.leftChildren << "\n";
        os << "Right Children: " << node.rightChildren << "\n";
        os << "Next Delete: " << node.nextDelete << "\n";
        os << "Height: " << node.height << "\n\n";
        return os;
    }

};

#endif // AVL_INDEX_NODE_H