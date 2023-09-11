#ifndef AVL_INDEX_CPP
#define AVL_INDEX_CPP

#include "class_avl_index.h"

void AVLIndex::initFile()
{
    file.open(this->indexFileName, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            file.open(this->indexFileName, std::ios::out | std::ios::binary);
            if (!file.good()) { throw std::runtime_error("No se pudo crear el AVLIndexFile!"); }
            file.close();
        }
        else { file.close(); }
        return;
}

void AVLIndex::initIndex()
{
    initFile();
    file.open(this->indexFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) { throw std::runtime_error("No se pudo abrir el archivo AVLIndex!"); }
    file.seekg(0, std::ios::end);
    long bytes = file.tellg();
    if (bytes < sizeof(AVLIndexHeader))
    {
        this->header.rootPointer = -1;
        this->header.lastDelete = -1;
        file.seekp(0, std::ios::beg);
        file.write((char*) &header, sizeof(AVLIndexHeader));
    }
    else
    {
        file.seekg(0, std::ios::beg);
        file.read((char*) &this->header, sizeof(AVLIndexHeader));
    }
    
    file.close();
    return;
}

void AVLIndex::insert(posType cPointer, AVLIndexNode &cNode, AVLIndexNode &iNode)
{
    if (cPointer == -1)
        {
            file.seekp(sizeof(AVLIndexHeader), std::ios::beg);
            file.write((char*) &iNode, sizeof(AVLIndexNode));
            header.rootPointer = sizeof(AVLIndexHeader);

            file.seekp(0, std::ios::beg);
            file.write((char*) &header, sizeof(AVLIndexHeader));
            return;
        }

        file.seekg(cPointer, std::ios::beg);
        file.read((char*) &cNode, sizeof(AVLIndexNode));

        if (iNode.data > cNode.data)
        {
            if (cNode.rightChildren != -1) { insert(cNode.rightChildren, cNode, iNode); }
            else
            {
                file.seekg(0, std::ios::end);
                posType insertPointer = file.tellg();
                if (header.lastDelete != -1)
                {
                    insertPointer = header.lastDelete;
                    AVLIndexNode tempNode;
                    file.seekg(insertPointer, std::ios::beg);
                    file.read((char*) &tempNode, sizeof(AVLIndexNode));

                    header.lastDelete = tempNode.nextDelete;
                    file.seekp(0, std::ios::beg);
                    file.write((char*) &header, sizeof(AVLIndexHeader));
                }

                file.seekp(insertPointer, std::ios::beg);
                file.write((char*) &iNode, sizeof(AVLIndexNode));

                cNode.rightChildren = insertPointer;
                file.seekp(cPointer, std::ios::beg);
                file.write((char*) &cNode, sizeof(AVLIndexNode));
            }
        }
        else if (iNode.data < cNode.data)
        {
            if (cNode.leftChildren != -1) { insert(cNode.leftChildren, cNode, iNode); }
            else
            {
                file.seekg(0, std::ios::end);
                posType insertPointer = file.tellg();
                if (header.lastDelete != -1)
                {
                    insertPointer = header.lastDelete;
                    AVLIndexNode tempNode;
                    file.seekg(insertPointer, std::ios::beg);
                    file.read((char*) &tempNode, sizeof(AVLIndexNode));

                    header.lastDelete = tempNode.nextDelete;
                    file.seekp(0, std::ios::beg);
                    file.write((char*) &header, sizeof(AVLIndexHeader));
                }

                file.seekp(insertPointer, std::ios::beg);
                file.write((char*) &iNode, sizeof(AVLIndexNode));

                cNode.leftChildren = insertPointer;
                file.seekp(cPointer, std::ios::beg);
                file.write((char*) &cNode, sizeof(AVLIndexNode));
            }
        }
        else { return ; }

        updateHeigth(cPointer);
        if (!isBalanced(cPointer)) { balance(cPointer); }
        return;
}

void AVLIndex::balance(posType nodePointer)
{
    if (nodePointer == -1) { return; }
    AVLIndexNode node;
    file.seekg(nodePointer, std::ios::beg);
    file.read((char*) &node, sizeof(AVLIndexNode));

    int balance = balancingFactor(nodePointer);

    if (balance > 1) // Esta cargado a la izquierda?
    {
        if (balancingFactor(node.leftChildren) <= -1) { 
            leftRotation(node.leftChildren); }
        rightRotation(nodePointer);
    }
    else if (balance < -1) // Esta cargado a la derecha?
    {
        if (balancingFactor(node.rightChildren) >= 1) { 
            rightRotation(node.rightChildren); }
        leftRotation(nodePointer);
    }
    return;
}

void AVLIndex::leftRotation(posType nodePointer)
{
    AVLIndexNode a, b;
    file.seekg(nodePointer, std::ios::beg);
    file.read((char*) &a, sizeof(AVLIndexNode));

    posType childPointer = a.rightChildren;

    file.seekg(a.rightChildren, std::ios::beg);
    file.read((char*) &b, sizeof(AVLIndexNode));

    a.rightChildren = b.leftChildren;
    b.leftChildren = childPointer;

    file.seekp(nodePointer, std::ios::beg);
    file.write((char*) &b, sizeof(AVLIndexNode));

    file.seekp(childPointer, std::ios::beg);
    file.write((char*) &a, sizeof(AVLIndexNode));

    updateHeigth(childPointer);
    updateHeigth(nodePointer);

    return;
}

void AVLIndex::rightRotation(posType nodePointer)
{
    AVLIndexNode a, b;
    file.seekg(nodePointer, std::ios::beg);
    file.read((char*) &a, sizeof(AVLIndexNode));

    posType childPointer = a.leftChildren;

    file.seekg(a.leftChildren, std::ios::beg);
    file.read((char*) &b, sizeof(AVLIndexNode));

    a.leftChildren = b.rightChildren;
    b.rightChildren = childPointer;

    file.seekp(nodePointer, std::ios::beg);
    file.write((char*) &b, sizeof(AVLIndexNode));

    file.seekp(childPointer, std::ios::beg);
    file.write((char*) &a, sizeof(AVLIndexNode));

    updateHeigth(childPointer);
    updateHeigth(nodePointer);
    return;
}

bool AVLIndex::isBalanced(posType nodePointer)
{
    if (nodePointer == -1) { return true; }
    if (std::abs(balancingFactor(nodePointer)) > 1) { return false; }
    return true;
}

int AVLIndex::balancingFactor(posType nodePointer)
{
    if (nodePointer == -1) { return 0; }
    AVLIndexNode node;
    file.seekg(nodePointer, std::ios::beg);
    file.read((char*) &node, sizeof(AVLIndexNode));
    return height(node.leftChildren) - height(node.rightChildren);
}

void AVLIndex::updateHeigth(posType nodePointer)
{
    if (nodePointer == -1) { return; }
    AVLIndexNode node;
    file.seekg(nodePointer, std::ios::beg);
    file.read((char*) &node, sizeof(AVLIndexNode));
    posType hLeft = height(node.leftChildren);
    posType hRigth = height(node.rightChildren);
    node.height = 1 + (hRigth > hLeft ? hRigth : hLeft);
    file.seekp(nodePointer, std::ios::beg);
    file.write((char*) &node, sizeof(AVLIndexNode));
    return;
}

long AVLIndex::height(posType nodePointer)
{
    if (nodePointer == -1) { return -1; }
    AVLIndexNode node;
    file.seekg(nodePointer, std::ios::beg);
    file.read((char*) &node, sizeof(AVLIndexNode));
    return node.height;
}

AVLIndexNode AVLIndex::search(posType currentPointer, AVLIndexNode &cNode, Data &item)
{
    if (currentPointer == -1) { throw std::runtime_error("No se ha encontrado el elemento!"); }

    file.seekg(currentPointer, std::ios::beg);
    file.read((char*) &cNode, sizeof(AVLIndexNode));
    if (item > cNode.data) { return search(cNode.rightChildren, cNode, item); }
    else if (item < cNode.data) { return search(cNode.leftChildren, cNode, item); }
    else { return cNode; }
}

posType AVLIndex::maxNode(posType nodePointer)
{
    if (nodePointer == -1) { throw std::runtime_error("El arbol está vacío!"); }

    AVLIndexNode node;
    file.seekg(nodePointer, std::ios::beg);
    file.read((char*) &node, sizeof(AVLIndexNode));

    if (node.rightChildren == -1) { return nodePointer; }
    else { return maxNode(node.rightChildren); }
}

bool AVLIndex::removeIndex(posType cPointer, posType pPointer, AVLIndexNode &cNode, Data item)
{
    if (cPointer == -1) { return false; }

    file.seekg(cPointer, std::ios::beg);
    file.read((char*) &cNode, sizeof(AVLIndexNode)); 

    if (item > cNode.data)
    {
        pPointer = cPointer;
        if (!removeIndex(cNode.rightChildren, pPointer, cNode, item)) { return false; }
    }
    else if (item < cNode.data)
    {
        pPointer = cPointer;
        if (!removeIndex(cNode.leftChildren, pPointer, cNode, item)) { return false; }
    }
    else
    {
        if (cNode.leftChildren == -1 && cNode.rightChildren == -1)
        {
            if (pPointer != -1)
            {
                AVLIndexNode pNode;
                file.seekg(pPointer, std::ios::beg);
                file.read((char*) &pNode, sizeof(AVLIndexNode));

                if (pNode.leftChildren == cPointer) { pNode.leftChildren = -1; }
                else if (pNode.rightChildren == cPointer) { pNode.rightChildren = -1; }

                file.seekp(pPointer, std::ios::beg);
                file.write((char*) &pNode, sizeof(AVLIndexNode));
            }

            cNode.nextDelete = header.lastDelete;
            file.seekp(cPointer, std::ios::beg);
            file.write((char*) &cNode, sizeof(AVLIndexNode));
            
            header.lastDelete = cPointer;
            file.seekp(0, std::ios::beg);
            file.write((char*) &header, sizeof(AVLIndexHeader));
        }
        else if (cNode.leftChildren == -1)
        {
            if (pPointer != -1)
            {
                AVLIndexNode pNode;
                file.seekg(pPointer, std::ios::beg);
                file.read((char*) &pNode, sizeof(AVLIndexNode));

                if (pNode.leftChildren == cPointer) { pNode.leftChildren = cNode.rightChildren; }
                else if (pNode.rightChildren == cPointer) { pNode.rightChildren = cNode.rightChildren; }

                file.seekp(pPointer, std::ios::beg);
                file.write((char*) &pNode, sizeof(AVLIndexNode));
            }

            cNode.nextDelete = header.lastDelete;
            file.seekp(cPointer, std::ios::beg);
            file.write((char*) &cNode, sizeof(AVLIndexNode));

            header.lastDelete = cPointer;
            file.seekp(0, std::ios::beg);
            file.write((char*) &header, sizeof(AVLIndexHeader));
        }
        else if (cNode.rightChildren == -1)
        {
            if (pPointer != -1)
            {
                AVLIndexNode pNode;
                file.seekg(pPointer, std::ios::beg);
                file.read((char*) &pNode, sizeof(AVLIndexNode));

                if (pNode.leftChildren == cPointer) { pNode.leftChildren = cNode.leftChildren; }
                else if (pNode.rightChildren == cPointer) { pNode.rightChildren = cNode.leftChildren; }

                file.seekp(pPointer, std::ios::beg);
                file.write((char*) &pNode, sizeof(AVLIndexNode));
            }

            cNode.nextDelete = header.lastDelete;
            file.seekp(cPointer, std::ios::beg);
            file.write((char*) &cNode, sizeof(AVLIndexNode));

            header.lastDelete = cPointer;
            file.seekp(0, std::ios::beg);
            file.write((char*) &header, sizeof(AVLIndexHeader));
        }
        else
        {
            posType newPos = maxNode(cNode.leftChildren);
            AVLIndexNode tempNode;

            file.seekg(newPos, std::ios::beg);
            file.read((char*) &tempNode, sizeof(AVLIndexNode));

            Data tempItem = tempNode.data;
            removeIndex(header.rootPointer, -1, tempNode, tempItem);
            fixValue(cPointer, tempNode, cNode.data, tempItem);
        }
    }

    updateHeigth(cPointer);
    if (!isBalanced(cPointer)) {  balance(cPointer); }
    return true;
}

void AVLIndex::fixValue(posType cPointer, AVLIndexNode &cNode, Data &item1, Data &item2)
{
    if (cPointer == -1) { return; }
    
    file.seekg(cPointer, std::ios::beg);
    file.read((char*) &cNode, sizeof(AVLIndexNode));

    if (cNode.data > item1) { return fixValue(cNode.leftChildren, cNode, item1, item2); }
    else if (cNode.data < item1) { return fixValue(cNode.rightChildren, cNode, item1, item2); }
    else
    {
        cNode.data = item2;
        file.seekp(cPointer, std::ios::beg);
        file.write((char*) &cNode, sizeof(AVLIndexNode));
    }
    return;
}

void AVLIndex::displayPretty(const std::string &prefix, posType cPointer, bool isLeft)
{
    if (cPointer == -1) { return; }

    AVLIndexNode cNode;

    file.seekg(cPointer, std::ios::beg);
    file.read((char*) &cNode, sizeof(AVLIndexNode));

    std::cout << prefix;
    std::cout << (isLeft ? "|--" : "|__");
    std::cout << cNode.data.numero << "(" << cNode.height << ")" << std::endl;

    displayPretty(prefix + (isLeft ? "|   " : "    "), cNode.leftChildren, true);
    displayPretty(prefix + (isLeft ? "|   " : "    "), cNode.rightChildren, false);
    return;
}

void AVLIndex::searchIndexsByRange(posType cPointer, AVLIndexNode &cNode, std::vector<AVLIndexNode> &cVector, Data &begin, Data &end)
{
    if (cPointer == -1) { return; }
    
    file.seekg(cPointer, std::ios::beg);
    file.read((char*) &cNode, sizeof(AVLIndexNode));

    if (cNode.data > begin)
    {
        searchIndexsByRange(cNode.leftChildren, cNode, cVector, begin, end);
        file.seekg(cPointer, std::ios::beg);
        file.read((char*) &cNode, sizeof(AVLIndexNode));
    }
    if (cNode.data >= begin && cNode.data <= end) { cVector.push_back(cNode); }
    if (cNode.data < end)
    {
        searchIndexsByRange(cNode.rightChildren, cNode, cVector, begin, end);
        file.seekg(cPointer, std::ios::beg);
        file.read((char*) &cNode, sizeof(AVLIndexNode));
    }

    return;
}

bool AVLIndex::insert(Data item)
{
    file.open(this->indexFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) { throw std::runtime_error("No se pudo abrir el archivo AVLIndex!"); }

    AVLIndexNode insertNode;
    insertNode.data = item;

    AVLIndexNode currentNode;

    insert(header.rootPointer, currentNode, insertNode);

    file.close();
    return true;
}

AVLIndexNode AVLIndex::search(Data item)
{
    file.open(this->indexFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) { throw std::runtime_error("No se pudo abrir el archivo AVLIndex!"); }
    
    AVLIndexNode searchNode;

    search(header.rootPointer, searchNode, item);

    file.close();
    return searchNode;
}

bool AVLIndex::removeIndex(Data item)
{
    file.open(this->indexFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) { throw std::runtime_error("No se pudo abrir el archivo AVLIndex!"); }

    AVLIndexNode currentNode;

    bool isRemoved = removeIndex(header.rootPointer, -1, currentNode, item);

    file.close();
    return isRemoved;
}

std::vector<AVLIndexNode> AVLIndex::searchIndexsByRange(Data start, Data end)
{
    file.open(this->indexFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) { throw std::runtime_error("No se pudo abrir el archivo AVLIndex!"); }

    AVLIndexNode node;
    std::vector<AVLIndexNode> vector;

    searchIndexsByRange(header.rootPointer, node, vector, start, end);

    file.close();
    return vector;
}

void AVLIndex::displayPretty()
{
    file.open(this->indexFileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) { throw std::runtime_error("No se pudo abrir el archivo AVLIndex!"); }

    AVLIndexNode node;
    file.seekg(header.rootPointer, std::ios::beg);
    file.read((char*) &node, sizeof(AVLIndexNode));
    displayPretty("", header.rootPointer, true);
    std::cout << std::endl;

    file.close();
    return;
}

#endif // AVL_INDEX_CPP