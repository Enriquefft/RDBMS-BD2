#ifndef SEQUENTIAL_INDEX_HPP
#define SEQUENTIAL_INDEX_HPP

#include "index.hpp"
#include "sequential_index_header.hpp"
#include "sequential_index_record.hpp"
#include "binary_search_response.hpp"

template<typename KEY_TYPE = default_data_type>
class SequentialIndex: public Index::Index<KEY_TYPE> {
    std::string indexFilename;
    std::string auxFilename;
    /*
        Helper functions 
    */
    void createFile();
    bool fileExists();

    bool validNumberRecords();

    void searchAuxFile(Data<KEY_TYPE> data, BinarySearchResponse<KEY_TYPE>& bir, std::vector<physical_pos>& records, SequentialIndexRecord<KEY_TYPE>& sir);

    template <typename FileType = std::fstream>
    void insertAux(FileType& indexFile, SequentialIndexRecord<KEY_TYPE>& sir_init, SequentialIndexRecord<KEY_TYPE>& sir, BinarySearchResponse<KEY_TYPE>& bsr);

    void insertAuxFile(SequentialIndexRecord<KEY_TYPE>& sir);

    template <typename FileType = std::fstream>
    void insertAfterRecord(FileType& file, SequentialIndexRecord<KEY_TYPE>& sir_prev, SequentialIndexRecord<KEY_TYPE>& sir, SequentialIndexHeader& sih, bool header);

    Response add(Data<KEY_TYPE> data, physical_pos raw_pos, bool rebuild);
    Response erase(Data<KEY_TYPE> data, Response& response);
    
    /*
        Binary search in files
    */
    template<typename FileType = std::fstream>
    BinarySearchResponse<KEY_TYPE> binarySearch(FileType& file, Data<KEY_TYPE> data);


public:
    /*
        Constructor
    */
    SequentialIndex(
        std::string _table_name,
        std::string _attribute_name,
        bool PK = false
    ) {
        this->table_name = _table_name;
        this->attribute_name = _attribute_name;
        this->index_name = "Sequential";
        this->indexFilename = _table_name + "_" + _attribute_name + "_" + this->index_name + "_indexFile.bin";
        this->auxFilename = _table_name + "_" + _attribute_name + "_"+ this->index_name + "_auxFile.bin";
        this->duplicatesFilename = _table_name + "_" + _attribute_name + "_" + this->index_name + "_duplicateFile.bin";
        this->PK = PK;
        if (!fileExists()) { createFile(); }
    }

    std::string get_index_name() override;

    /*
        Query functions
    */

    Response bulkLoad(std::vector<std::pair<Data<KEY_TYPE>, physical_pos>>& records);

    Response add(Data<KEY_TYPE> data, physical_pos raw_pos) override;
    Response search(Data<KEY_TYPE> data) override;
    Response rangeSearch(Data<KEY_TYPE> begin, Data<KEY_TYPE> end) override;
    Response erase(Data<KEY_TYPE> data) override;

    Response addNotRebuild(Data<KEY_TYPE> data, physical_pos raw_pos);

    Response loadRecords(std::vector<std::pair<Data<KEY_TYPE>, physical_pos>>& records);
    void rebuild();

    /*
        Print files sequentially
    */
    void printIndexFile();
    void printAuxFile();
    void printDuplicatesFile();
};

#include "sequential_index.tpp"

#endif //SEQUENTIAL_INDEX_HPP