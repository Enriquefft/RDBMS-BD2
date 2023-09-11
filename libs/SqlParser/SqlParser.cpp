#include <cassert>
#include <cctype>
#include <fstream>
#include <functional>

#include "SqlParser.hpp"

const std::string METADATA_PATH = "./meta.data";

SqlParser::SqlParser(DBEngine& dbengine): engine(dbengine) {
  std::ifstream metadata(METADATA_PATH, std::ios::app | std::ios::binary);
  if (!metadata.is_open()) {
    std::cerr << "Can't open meta.data\n";
    exit(EXIT_FAILURE);
  }

  // Load all tablenames in memory
  metadata.seekg(0);
  while (metadata.peek() != EOF) {
    char name[64];
    metadata.read(name, 64);
    this->tablenames.insert(name);
  }

  metadata.close();
}

SqlParser::~SqlParser() {
  delete sc;
  sc = nullptr;
  delete parser;
  parser = nullptr;
}

void SqlParser::parse(const char *filename) {
  assert(filename != nullptr);
  std::ifstream in_file(filename);
  if (!in_file.good()) {
    exit(EXIT_FAILURE);
  }
  parse_helper(in_file);
}

std::vector<std::string>& SqlParser::parse(std::istream &stream) {
  if (!stream.good() && stream.eof()) {
    return this->response;
  }
  parse_helper(stream);
  return this->response;
}

void SqlParser::parse_helper(std::istream &stream) {
  delete (sc);
  try {
    sc = new scanner(&stream);
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate scanner: (" << ba.what()
              << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }
  delete (parser);
  try {
    parser = new yy::parser((*sc), (*this));
  } catch (std::bad_alloc &ba) {
    std::cerr << "Failed to allocate parser: (" << ba.what()
              << "), exiting!!\n";
    exit(EXIT_FAILURE);
  }

  const int accept(0);
  if (parser->parse() != accept) {
    std::cerr << "Parse failed!!\n";
  }
}

void SqlParser::checkTableName(const std::string& tablename) {
  this->engine.checkTableName(tablename);
}

void SqlParser::createTable(std::string &tablename, const std::vector<column_t *> &columns) {
  std::vector<Type> types(columns.size());
  std::vector<std::string> col_names(columns.size());



  //this->engine.create_table(tablename, );
  /* if (this->tablenames.find(tablename) != nullptr) {
    std::cerr << "Table already exist\n";
    exit(EXIT_FAILURE);
  }
  this->tablenames.insert(tablename);

  std::ofstream metadata(METADATA_PATH, std::ios::binary | std::ios::app);
  metadata.seekp(0, std::ios::end);
  metadata.write(tablename.c_str(), 64);
  metadata.close();

  std::ofstream tablefile(tablename + ".bin", std::ios::app);
  for (auto &column : columns) {
    tablefile.write((char *)&*column, sizeof(*column));
    delete column;
  }
  tablefile.close(); */
}

void SqlParser::select(std::string &tablename, std::vector<std::string> *column_names, std::list<std::list<condition_t>>& constraints) {
  auto& table = this->engine.get_index(tablename);

  // check if col exists
  for (auto& col: column_names) {
    if (table.find(col) == nullptr) {
      std::cerr << "Column " << col << " not exists\n";
      throw 2;
    }
  }


  for(auto it = constraints.begin(); it != constraints.end(); it++) {
    condition_t* constraint_key;
    std::function<bool(std::string)> lamb = [](std::string rec) {return true;};

    for (auto it2 = it->begin(); it2 != it->end(); it2++) {
      // Checkear si el constraint actual tiene un indice asociado
      // si lo tiene, asignar al constraint_key
      // si no, construir un predicado con los operadores;
      auto& index = table.find(it2->column);
      if (index.size() != 0){
      
      }
      else {
      switch (it2->c)
      {
      case EQUAL:
        lamb = [it2, lamb](std::string rec) {return lamb(rec) && rec == it2->value;};
        break;
      case GE:
        lamb = [it2, lamb](std::string rec) {return lamb(rec) && rec >= it2->value;};
        break;
      case G:
        lamb = [it2, lamb](std::string rec) {return lamb(rec) && rec > it2->value;};
        break;
      case LE:
        lamb = [it2, lamb](std::string rec) {return lamb(rec) && rec <= it2->value;};
        break;
      case L:
        lamb = [it2, lamb](std::string rec) {return lamb(rec) && rec < it2->value;};
        break;
      default:
        break;
      }
      }
    }
    //if (constraint_key->c == EQUAL)
    //  this->engine.search(tablename, constraint_key->value, lamb);
    //else
      //this->engine.range_search(tablename,); 
  }
}
