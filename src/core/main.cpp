#include <iostream>
#include "lupck/cli.hpp"
#include "lupck/loader.hpp"
#include "lupck/package.hpp"
#include "lupck/compiler.hpp"
#include "lupck/utils.hpp"
#include "CLI11/CLI11.hpp"

#define VERSION "0.1.0"
#define GITHUB "https://github.com/WellizxSilva/lupck"


int main(int argc, char* argv[]) {
    std::string exePath = Lupck::Utils::GetExecutablePath();
    if (Lupck::Loader::HasEmbeddedPackage(exePath.c_str())) {
        if (!Lupck::Loader::RunEmbeddedPackage(exePath.c_str())) {
            return 1;
        }
        return 0;
    }
    if (argc == 1) {
        std::cout << "Lua Packer Tool (lupck) v" << VERSION << "\n";
        std::cout << "Type '--help' to see available commands\n";
        return 0;
    }
    CLI::App app{"Lua Packer Tool (lupck)"};
    app.set_help_flag("-h,--help", "Show help message");
    app.set_version_flag("--version", VERSION);
    std::string runInput;
    auto run = app.add_subcommand("run", "Run a lupck package");
    run->add_option("input", runInput, ".lpk")->required();

    std::string packOutput;
    std::vector<std::string> packInputs;
    bool packLog = false;
    auto pack = app.add_subcommand("pack", "Pack files into .lpk");
    pack->add_option("output", packOutput, "Output package")->required();
    pack->add_option("files", packInputs, "Input files")->required();
    pack->add_option("--log", packLog, "Enable logging")->default_val(true);

    std::string unpackInput;
    std::string unpackDir{"unpacked"};
    auto unpack = app.add_subcommand("unpack", "Unpack .lpk into directory");
    unpack->add_option("input", unpackInput, "Package to unpack")->required();
    unpack->add_option("-o,--output", unpackDir, "Output directory");

    std::string compileInput, compileOutput, compileBaseEngie{"dist/lupck.exe"};
    auto compile = app.add_subcommand("compile", "Generate standalone exe from .lpk");
    compile->add_option("input", compileInput, "Package file")->required();
    compile->add_option("output", compileOutput, "Output exe")->required();
    compile->add_option("--base", compileBaseEngie, "Path to base executable (default: dist/lupck.exe)");
    bool compileLog = false;
    compile->add_option("--log", compileLog, "Enable logging")->default_val(true);

    CLI11_PARSE(app, argc, argv);

    if (*run) {
        Lupck::CLI::CLIOptions opts{true, "run", runInput, {}};
        if (!Lupck::Loader::Run(opts)) return 1;
    }
    else if (*pack) {
        if (!Lupck::Package::Packer::CreatePackage(packOutput, packInputs, packLog)) return 1;
    }
    else if (*unpack) {
        if (!Lupck::Package::Unpacker::ExtractAll(unpackInput, unpackDir)) return 1;
    }
    else if (*compile) {
        if (!Lupck::Compiler::CreateExecutable(compileInput, compileOutput, compileBaseEngie, compileLog)) return 1;
    }

    return 0;
}
