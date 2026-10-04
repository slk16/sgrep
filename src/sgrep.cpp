#include <functional>
#include <iostream>
#include <functional>
#include <stdexcept>
#include <cstring>
#include "sgrep.hpp"
void run_grep(int argc, char* argv[]) {
    sgrep::Sgrep grep;
    using grepPatt = sgrep::Sgrep::Config::SearchPattern::Option;
    using printMode = sgrep::Sgrep::Config::PrintMode::Option;
    grep.conf_.pattern_.set_pattern(grepPatt::ByRegex);
    if (argc > 1) {
        auto error = [](const char* err = "\
            Usage: sgrep [OPTION]... PATTERNS [FILE]\n\
            -E using the ECMAScript to search") {
            throw std::runtime_error(err);
        };
        constexpr int expect_regex = 1;
        constexpr int expect_string = 2;
        int state = expect_string;
        bool ready = false;
        for (int i = 1; i < argc; ++i) {
            std::string_view a = argv[i];
            if (a == "-E")
                state = expect_regex;
            else if (a == "-n")
                grep.conf_.pmode_.set_flag(printMode::line_number);
            else if (state == expect_string && !ready) {
                grep.conf_.pattern_.set_pattern(grepPatt::ByString);
                grep.set_target(argv[i]);                
                ready = true;
            }
            else if (state == expect_regex&& !ready) {
                grep.conf_.pattern_.set_pattern(grepPatt::ByRegex);
                grep.set_regex(std::regex(argv[i]));                
                ready = true;
            }
            else
                error();
        }
        grep.run();
    } else {
        std::cout 
            << "sgrep [Options] ... [Target]" << std::endl
            << "-E using the ECMAScript to search" << std::endl;
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