//
// Created by xhy on 2021/7/12.
//

#include "SymbolsReader.h"
#include <fstream>
#include <windows.h>
#include <dbghelp.h>
#include <iostream>
#include <algorithm>
#include "tools.h"

namespace {

    enum ReadStatus {
        Start,
        PubSymbol,
        Headers,
        SectionSize,
    };

    uint64_t strToHex(const char *str, bool &success) {
        char *end;
        unsigned long long result;
        result = strtoull(str, &end, 16);
        if (result == 0 && end == str) {
            success = false;
        } else if (result == ULLONG_MAX && errno) {
            success = false;
        } else if (*end) {
            success = false;
        } else {
            success = true;
        }
        return result;
    }

    std::string symToPrototype(const std::string &symbol) {
        char buffer[1024];
        memset(buffer, 0, 1024);
        UnDecorateSymbolName(symbol.c_str(), buffer, 1024, 0);
        return std::string(buffer);
    }

    uint32_t parseSectionNumberFromStr(const std::string &s) {
        auto p = s.rfind('#');
        if (p == std::string::npos) {
            return -1;
        }
        auto str = s.substr(p + 1, s.size() - p);
        bool success;
        auto r = strToHex(str.c_str(), success);
        return success ? static_cast<uint32_t>(r) : -1;

    }

    uint64_t parseSectionVirtualAddressFromStr(const std::string &s) {
        auto p = s.find('v');
        if (p == std::string::npos) {
            return -1;
        }
        auto str = s.substr(0, p - 1);
        bool success;
        auto r = strToHex(str.c_str(), success);
        return success ? r : -1;
    }
}


SymbolItem parserSymbolItemString(const std::string &s, bool &success) {
    //S_PUB32: [0002:000E9BC0], Flags: 00000000, ??_C@_0BH@IHIHBKLA@minecraft?3spawn_groups@
    auto p1 = s.find('[');
    auto p2 = s.find(']', p1);
    if (p1 == std::string::npos || p2 == std::string::npos) {
        success = false;
        return SymbolItem();
    }
    auto address = s.substr(p1 + 1, p2 - p1 - 1);
    auto p3 = s.rfind(' ');
    auto p4 = address.find(':');
    if (p4 == std::string::npos || p3 == std::string::npos) {
        success = false;
        return SymbolItem();
    }

    auto symbol = s.substr(p3 + 1, s.size() - p3);
    auto header_number = address.substr(0, p4);
    auto address_in_section = address.substr(p4 + 1, address.size() - p4);
    SymbolItem item;
    bool s1, s2;
    item.header_offset = strToHex(address_in_section.c_str(), s1);
    item.section_number = strToHex(header_number.c_str(), s2);
    success = s1 && s2;
    item.symbol = symbol;
    item.prototype = symToPrototype(symbol);
    return item;
}


bool SymbolsReader::InitFromCvdump(const std::string &cv_filename) {
    std::ifstream input(cv_filename);
    if (input.is_open()) {
        ReadStatus readStatus = Start;
        std::string line;
        uint32_t sectionNumber = -1;
        while (std::getline(input, line)) {
            switch (readStatus) {
                case Start:
                    if (line == "*** PUBLICS") {
                        info("meet symbols part\n");
                        readStatus = PubSymbol;
                    }
                    break;
                case PubSymbol:
                    if (line == "*** SECTION HEADERS") {
                        info("meet headers part\n");
                        readStatus = Headers;
                    } else {
                        if (!line.empty()) {
                            bool success;
                            auto item = parserSymbolItemString(line, success);
                            for (auto &black_token:this->inline_black_list) {
                                auto r = std::any_of(inline_black_list.begin(), inline_black_list.end(),
                                                     [&](const std::string &token) {
                                                         return item.prototype.find(token) != std::string::npos;
                                                     }
                                );
                                if (!r) {
                                    this->symbols_[item.symbol] = item;
                                }
                            }
                        }
                    }
                case Headers:
                    if (line.rfind("SECTION HEADER", 0) == 0) {
                        sectionNumber = parseSectionNumberFromStr(line);
                        info("find section header\n");
                        readStatus = SectionSize;
                    }
                    break;
                case SectionSize:
                    if (line.find("virtual address") != std::string::npos) {
                        if (sectionNumber == -1) {
                            printf("error\n");
                        } else {
                            auto addr = parseSectionVirtualAddressFromStr(line);
                            while (section_headers_.size() <= sectionNumber &&
                                   section_headers_.size() < 1024)
                                section_headers_.emplace_back();
                            section_headers_[sectionNumber].header_number = sectionNumber;
                            section_headers_[sectionNumber].virtual_address = addr;
                            sectionNumber = -1;
                        }
                        readStatus = Headers;
                    }
                    break;
            }
        }
        input.close();
        this->calculateRva();
        this->PrintSimpleInfo();
        return true;
    } else {
        return false;
    }
}


void SymbolsReader::Dump() {

    printf("total %llu symbols:\n", this->symbols_.size());
    for (const auto &sym:this->symbols_) {
        printf("%s\n[0x%llx] %s\n", sym.second.prototype.c_str(), sym.second.rva, sym.second.symbol.c_str());
        //std::cout << sym << std::endl;
    }

    printf("total %llu section headers:\n", this->section_headers_.size());
    for (const auto &sh:this->section_headers_) {
        std::cout << sh << std::endl;
    }

}

void SymbolsReader::PrintSimpleInfo() {
    info("total %llu symbols was found\n", this->symbols_.size());
    info("total %llu section headers was found\n", this->section_headers_.size());
}


std::map<std::string, SymbolItem> &SymbolsReader::symbols() {
    return this->symbols_;
}

void SymbolsReader::calculateRva() {

    for (auto &h:this->section_headers_) {
        std::cout << h.header_number << "  " << h.virtual_address << std::endl;
    }
    for (auto &symKv:this->symbols_) {
        auto &sym = symKv.second;
        if (sym.section_number < this->section_headers_.size()) {
            this->symbols_[symKv.first].rva =
                    sym.header_offset + this->section_headers_[sym.section_number].virtual_address;
        } else {
            fprintf(stderr, "error when calculate symbol: %s\n", sym.symbol.c_str());
        }
    }
}


void SymbolsReader::setBlackList() {
    this->inline_black_list = {
            "entt::",
            "JsonUtil:",
            "lambda",
            "iterator",
            "api-ms-win",
            "deleting destructor",
            "vftable"
    };
}

SymbolsReader::SymbolsReader() {
    this->setBlackList();
}

bool SymbolsReader::Load(const std::string &filename) {
    std::ifstream input(filename);
    this->symbols_.clear();
    std::vector<std::string> buffer;
    if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) {
            if (!line.empty()) {
                buffer.push_back(line);
            }
            if (buffer.size() == 3) {
                bool success = false;
                auto rva = strToHex(buffer[0].c_str(), success);
                if (!success) {
                    fprintf(stderr, "error when parse file");
                }
                this->symbols_[buffer[2]] = {buffer[2], 0, 0, rva, buffer[1]};
                buffer.clear();
            }
        }
        this->PrintSimpleInfo();
    }
    return true;
}

bool SymbolsReader::Save(const std::string &filename) {
    FILE *fp = nullptr;
    if (filename.empty()) {
        fp = stdout;
    } else {
        fp = fopen(filename.c_str(), "a+");
    }

    if (fp == nullptr) {
        return false;
    }
    for (auto &iKv:this->symbols_) {
        auto i = iKv.second;
        fprintf(fp, "%#llX\n%s\n%s\n\n", i.rva, i.prototype.c_str(), i.symbol.c_str());
    }
    fflush(fp);
    fclose(fp);
    return true;
}



