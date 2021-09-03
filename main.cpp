#include <iostream>
#include "cxxopts.hpp"
#include "SymbolsReader.h"
#include "UserFile.h"

std::string getFileByArg(const cxxopts::ParseResult &r, const char *arg) {
    return r[arg].as<std::string>();
}

int main(int argc, char *argv[]) {
    cxxopts::Options options("tdh", "trapdoor develop helper");
    options.add_options()
            //input file
            ("s,sym", "input symbol files", cxxopts::value<std::string>())
            ("p,pdb", "input pdb file", cxxopts::value<std::string>())
            ("j,json", "input json file", cxxopts::value<std::string>())
            ("u,user", "input json file", cxxopts::value<std::string>())
            //function
            ("h,help", "helpe info")
            ("d,dump", "dump symbol file from pdb")
            ("e,export", "export json file from symbol file")
            ("c,check", "check json file according to the json and the sym file")
            ("g,generate", "generate C++ header file from json");

    auto result = options.parse(argc, argv);
    if (result.count("d")) {
        auto pdbFile = result["p"].as<std::string>();
        auto symFile = result["s"].as<std::string>();
        const std::string command = "cvdump -headers -p " + pdbFile + " > " + symFile;
        return system(command.c_str());
    } else if (result.count("e")) {
        auto symFile = result["s"].as<std::string>();
        auto jsonFile = result["j"].as<std::string>();
        SymbolsReader reader;
        reader.InitFromCvdump(symFile);
        reader.Save(jsonFile);
        return 0;
    } else if (result.count("c")) {
        auto userFile = result["u"].as<std::string>();
        auto jsonFile = result["j"].as<std::string>();
        SymbolsReader reader;
        reader.Load(jsonFile);
        UserFile uFile(userFile);
        uFile.checkExist(reader);
        return 0;
    } else if (result.count("g")) {
        auto userFile = result["u"].as<std::string>();
        auto jsonFile = result["j"].as<std::string>();
        SymbolsReader reader;
        reader.Load(jsonFile);
        UserFile uFile(userFile);
        uFile.geneteCppHeader(reader);
    } else if (result.count("h")) {
        std::cout << options.help();
    }
    return 0;
}
