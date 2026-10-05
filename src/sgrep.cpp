#include <iostream>
#include <stdexcept>
#include "sgrep.hpp"
void run_grep(int argc, char* argv[]) {
    sgrep::Sgrep grep;
    using grepPatt = sgrep::Sgrep::Config::SearchPattern::Option;
    using printMode = sgrep::Sgrep::Config::PrintMode::Option;
    grep.conf_.pattern_.set_pattern(grepPatt::ByRegex);
    constexpr std::uint32_t normal =            0x0000'0000'0000'0001;
    constexpr std::uint32_t expect_regex =      0x0000'0000'0000'0002;
    constexpr std::uint32_t expect_file_name =  0x0000'0000'0000'0004;
    std::uint32_t state = normal;
    std::uint32_t pre_state = normal;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::string_view a = argv[i];
            if (a == "-E") {
                pre_state = state;                
                state = expect_regex;
            }
            else if (a == "-n")
                grep.conf_.pmode_.set_flag(printMode::block_number);
            else if (a == "-f") {
                pre_state =  state;
                state = expect_file_name;
            }
            else if (state == normal) {
                grep.conf_.pattern_.set_pattern(grepPatt::ByString);
                grep.set_target(argv[i]);                
            }
            else if (state == expect_regex) {
                grep.conf_.pattern_.set_pattern(grepPatt::ByRegex);
                grep.set_regex(std::regex(argv[i]));                
                state = pre_state;
            }
            else if (state == expect_file_name) {
                grep.conf_.ios_.set_file_name(argv[i]);
                state = pre_state;
            }
            else {
                sgrep::usage();
                return ;
            }
        }
    } else {
        sgrep::usage();
        return ;
    }
    if (1 == pre_state) {
        grep.run();
    } else {
        sgrep::usage();
    }
}
int main(int argc, char* argv[]) {
    try {
        run_grep(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "sgrep : "  << e.what() << std::endl;
        throw;
    }

    return 0;
}