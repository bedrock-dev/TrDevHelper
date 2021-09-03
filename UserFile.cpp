//
// Created by xhy on 2021/8/13.
//

#include "UserFile.h"
#include <fstream>
#include "md5.h"
#include <regex>
#include "tools.h"

namespace {
//from https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
    void replaceAll(std::string &str, const std::string &from, const std::string &to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }


////public: void __cdecl Village::tick(struct Tick,class BlockSource & __ptr64) __ptr64
    std::string getProtoTypePrefix(const std::string &raw_proto) {
        auto proto = raw_proto;
        replaceAll(proto, "std::", "std");
        auto p1 = proto.find("::");
        if (p1 == std::string::npos) {
            return "FUNC";
        }

        auto p2 = proto.rfind(' ', p1);
        auto p3 = proto.find('(', p2);
        auto class_name = proto.substr(p2, p1 - p2);
        auto func_name = proto.substr(p1 + 2, p3 - p1 - 2);

        auto name = class_name + "_" + func_name;
        replaceAll(name, "__", "_");
        return name;
    }


    template<typename I>
    std::string n2hexStr(I w, size_t hex_len = sizeof(I) << 1) {
        static const char *digits = "0123456789ABCDEF";
        std::string rc(hex_len, '0');
        for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
            rc[i] = digits[(w >> j) & 0x0f];
        return rc;
    }
}

UserFile::UserFile(const std::string &fileName) {
    this->Load(fileName);
}


bool UserFile::Load(const std::string &fileName) {
    info("Begin read user symbol file %s\n", fileName.c_str());
    std::ifstream input(fileName);
    if (input.is_open()) {
        std::string line;
        while (std::getline(input, line)) {
            if ((!line.empty()) && line[0] != '#') {
                info("got symbol %s\n", line.c_str());
                this->symbols_.insert(line);
            }
        }
    }
    return true;
}

const std::set<std::string> &UserFile::symbols() {
    return this->symbols_;
}

bool UserFile::checkExist(SymbolsReader &reader) {
    size_t num = 0;
    printf("---------------res----------------------\n");
    auto allSymbols = reader.symbols();
    for (auto &sym: this->symbols_) {
        if (allSymbols.find(sym) == allSymbols.end()) {
            fprintf(stderr, "Symbol [%s] has lost\n", sym.c_str());
            ++num;
        }
    }
    if (num == 0) {
        printf("All symbols is found\n");
    } else {
        fprintf(stderr, "total %zu symbols has lost\n", num);
    }
}

void UserFile::generateCppHeader(SymbolsReader &reader) {
    std::string text = "#pragma  once\nnamespace SymHook{\n";
    int n = 0;
    auto allSymbols = reader.symbols();
    for (auto &sym: this->symbols_) {
        ++n;
        text += "   //[" + std::to_string(n) + "]" + sym;
        auto md5Value = md5(sym);
        text += ";\n";
        auto iter = allSymbols.find(sym);
        if (iter == allSymbols.end()) {
            text += "   //TODO: Symbol not found\n";
            text += "   constexpr uint64_t ";
            text += "NOT_FOUND_" + md5Value;
            text += " = 0x0;\n\n";
        } else {
            text += "   //" + iter->second.prototype;
            text += "\n    constexpr uint64_t ";
            text += getProtoTypePrefix(iter->second.prototype) + "_" + md5Value.substr(0, 8);
            text += " = 0x";
            text += n2hexStr(static_cast<uint32_t>(iter->second.rva));
            text += ";\n\n";
        }
    }
    text += "}";
    printf("%s", text.c_str());
}
