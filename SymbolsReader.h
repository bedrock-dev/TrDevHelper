//
// Created by xhy on 2021/7/12.
//

#ifndef QTEST1_SYMBOLSREADER_H
#define QTEST1_SYMBOLSREADER_H

#include <string>
#include <utility>
#include <vector>
#include <set>
#include <ostream>
#include <map>

struct SymbolItem {
    std::string symbol;
    uint64_t header_offset;
    uint32_t section_number;
    uint64_t rva = 0;
    std::string prototype;

    friend std::ostream &operator<<(std::ostream &os, const SymbolItem &item) {
        os
                << " rva: " << item.rva
                << " prototype: " << item.prototype;
        return os;
    }

    bool operator<(const SymbolItem &rhs) const {
        return symbol < rhs.symbol;
    }
};

struct SectionHeader {
    uint32_t header_number;
    uint64_t virtual_address;
    uint32_t virtual_size;

    friend std::ostream &operator<<(std::ostream &os, const SectionHeader &header) {
        os << "header_number: " << header.header_number << " virtual_address: " << header.virtual_address
           << " virtual_size: " << header.virtual_size;
        return os;
    }
};


class SymbolsReader {
public:

    SymbolsReader();


    bool Load(const std::string &filename);

    bool Save(const std::string &filename);

    bool InitFromCvdump(const std::string &cv_filename);

    void Dump();

    void PrintSimpleInfo();


    std::map<std::string, SymbolItem> &symbols();


private:
    void calculateRva();

    void setBlackList();

    std::map<std::string, SymbolItem> symbols_;
    std::vector<SectionHeader> section_headers_;

    std::set<std::string> inline_black_list;
};


SymbolItem parserSymbolItemString(const std::string &s, bool &success);

#endif //QTEST1_SYMBOLSREADER_H
