//
// Created by xhy on 2021/9/1.
//
#include "CLI11.hpp"
#include "SymbolsReader.h"
#include "UserFile.h"
#include "tools.h"

void set_debug(uint64_t v) {
    enableDebug();
}

void print_version(uint64_t v) {
    printf("Tr Symbol Helper v0.4 by hhhxiao");
}

int main(int argc, const char *argv[]) {
    CLI::App app{"Tr Symbol Helper"};
    app.require_subcommand();
    app.add_flag("-d,--debug", set_debug, "enable debug mode");
    app.add_flag("-v,--version", print_version, "got versiobn info");

    auto dump = app.add_subcommand("dump", "从PDB导出信息到导出的文本文件");

    std::string pdb_file;
    std::string output_file;
    dump->add_option("pdb_file", pdb_file, "pdb file")->required()->
            check(CLI::ExistingFile);
    dump->add_option("-o,--output", output_file, "dump text file")->required();
    dump->callback([&]() {
        const std::string command = "cvdump -headers -p " + pdb_file + " > " + output_file;
        system(command.c_str());
    });


    auto sys = app.add_subcommand("exp", "从导出的文本文件生成系统符号表");
    std::string dump_file;
    sys->add_option("dump_file", dump_file, "dump text file")->required()->check(CLI::ExistingFile);
    sys->add_option("-o,--output", output_file, "system symbol file");
    sys->callback([&]() {
        SymbolsReader reader;
        reader.InitFromCvdump(dump_file);
        reader.Save(output_file);
    });


    std::string system_symbol;
    std::string user_symbol;
    auto check = app.add_subcommand("check", "根据系统符号表检查用户符号是否有缺失");
    check->add_option("-s,--sys", system_symbol, "system symbol path")->required()->check(CLI::ExistingFile);
    check->add_option("-u,--user", user_symbol, "user symbol path")->required()->check(CLI::ExistingFile);
    check->callback([&]() {
        SymbolsReader reader;
        reader.Load(system_symbol);
        UserFile uFile(user_symbol);
        uFile.checkExist(reader);
    });


    auto gen = app.add_subcommand("gen", "根据系统符号表和用户符号表生成cpp头文件");
    gen->add_option("-s,--sys", system_symbol, "system symbol path")->required()->check(CLI::ExistingFile);
    gen->add_option("-u,--user", user_symbol, "user symbol path")->required()->check(CLI::ExistingFile);
    gen->callback([&]() {
        SymbolsReader reader;
        reader.Load(system_symbol);
        UserFile uFile(user_symbol);
        uFile.generateCppHeader(reader);
    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}
