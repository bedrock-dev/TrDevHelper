//
// Created by xhy on 2021/8/13.
//

#ifndef TRDEVHELPER_USERFILE_H
#define TRDEVHELPER_USERFILE_H

#include <vector>
#include <string>
#include <set>
#include "SymbolsReader.h"

class UserFile {
    std::set<std::string> symbols_;

    bool Load(const std::string &fileName);

public:
    explicit UserFile(const std::string &fileName);

    UserFile() = delete;

    const std::set<std::string> &symbols();

    bool checkExist(SymbolsReader &reader);

    void generateCppHeader(SymbolsReader &reader);
};


#endif //TRDEVHELPER_USERFILE_H
