#ifndef AVL_INDEX_H
#define AVL_INDEX_H

#include "utils.h"
#include "class_avl_index_node.h"
#include "class_avl_index_header.h"

class AVLIndex
{
    AVLIndexHeader header;
    std::string indexFileName;
    std::fstream file;

    void initFile();

    void initIndex();

    void insert(posType cPointer, AVLIndexNode &cNode, AVLIndexNode &iNode);

    void balance(posType nodePointer);

    void leftRotation(posType nodePointer);

    void rightRotation(posType nodePointer);

    bool isBalanced(posType nodePointer);

    int balancingFactor(posType nodePointer);

    void updateHeigth(posType nodePointer);
    
    long height(posType nodePointer);

    AVLIndexNode search(posType currentPointer, AVLIndexNode &cNode, Data &item);

    posType maxNode(posType nodePointer);

    bool removeIndex(posType cPointer, posType pPointer, AVLIndexNode &cNode, Data item);

    void fixValue(posType cPointer, AVLIndexNode &cNode, Data &item1, Data &item2);

    void displayPretty(const std::string &prefix, posType cPointer, bool isLeft);

    void searchIndexsByRange(posType cPointer, AVLIndexNode &cNode, std::vector<AVLIndexNode> &cVector, Data &begin, Data &end);

public:
    AVLIndex(std::string _indexFileName)
    {
        this->indexFileName = _indexFileName;
        initIndex();
    }

    bool insert(Data item);

    AVLIndexNode search(Data item);

    bool removeIndex(Data item);

    std::vector<AVLIndexNode> searchIndexsByRange(Data start, Data end);

    void displayPretty();
};

#endif // AVL_INDEX_H