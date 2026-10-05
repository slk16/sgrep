#pragma once 
#ifndef SGREP_HPP_INCLUDED
#define SGREP_HPP_INCLUDED
#include <iostream>
#include <regex>
#include <optional>
#include <functional>
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <variant>
#include <string_view>
#include <iomanip>
#include <fstream>

namespace sgrep{
    class sgrep_config_error : public std::runtime_error{
    public:
        sgrep_config_error(char const* error) : std::runtime_error(error){}
    };
    class sgrep_runtime_error : public std::runtime_error{
    public:
        sgrep_runtime_error(char const* error) : std::runtime_error(error) {

        }
    };
    inline void usage() {
        std::cout 
            << "sgrep [Options...] [Input] [Target]" << std::endl
            << "-E using the ECMAScript to search" << std::endl
            << "-n to show the block number" << std::endl;
    }
    class Sgrep {
    public:
        struct Config {
        public:
            struct Color {
            public:
                struct Option {
                    constexpr static const char* Grey = "\033[1;30m";
                    constexpr static const char* Red = "\033[1;31m";
                    constexpr static const char* Green = "\033[1;32m";
                    constexpr static const char* Yellow = "\033[1;33m";
                    constexpr static const char* Blue = "\033[1;34m";
                    constexpr static const char* Pink = "\033[1;35m";
                    constexpr static const char* Cyan = "\033[1;36m";
                    constexpr static const char* White = "\033[1;37m";
                    
                    constexpr static const char* thin_Grey = "\033[0;30m";
                    constexpr static const char* thin_Red = "\033[0;31m";
                    constexpr static const char* thin_Green = "\033[0;32m";
                    constexpr static const char* thin_Yellow = "\033[0;33m";
                    constexpr static const char* thin_Blue = "\033[0;34m";
                    constexpr static const char* thin_Pink = "\033[0;35m";
                    constexpr static const char* thin_Cyan = "\033[0;36m";
                    constexpr static const char* thin_White = "\033[0;37m";

                    constexpr static const char* Reset = "\033[0m";
                };
                void set_begin_color(std::string color = Option::Red) noexcept {
                    this->begin_color = color;
                }
                void set_end_color(std::string color = Option::Reset) noexcept {
                    this->end_color = color;
                }
                void reset_begin_color() noexcept {
                    this->begin_color = Option::Reset; 
                }
                void reset_end_color() noexcept{
                    this->end_color = Option::Reset;
                }
                std::string get_begin_color() const noexcept {
                    return this->begin_color; 
                }
                std::string get_end_color() const noexcept {
                    return this->end_color;
                }
            private:
                std::string begin_color = Option::Red; 
                std::string end_color = Option::Reset;
            };
            class Ios {
            public:
                enum class ReadingOption {
                    ByStdio,
                    ByFile
                };
                enum class ReadingPattern{
                    ByLine
                };
                // set io
                void set_in(std::istream& in) noexcept {
                    ins_ = in;
                }
                void set_out(std::ostream& out) noexcept {
                    outs_ = out;
                }
                void reset_in() noexcept {
                    ins_ = std::cin; 
                }
                void reset_out() noexcept {
                    outs_ = std::cout; 
                }
                //get io
                std::reference_wrapper<std::istream>
                get_ins() const noexcept {
                    return this->ins_;
                }
                std::reference_wrapper<std::ostream>
                get_outs() const noexcept {
                    return this->outs_;
                }
                //set /get reading option
                void set_opt(ReadingOption opt) {
                    this->opt_ = opt;
                }
                ReadingOption get_opt() {
                    return this->opt_;
                }
                //set / get reading pattern
                void set_patt(ReadingPattern patt) {
                    patt_ = patt;
                }
                ReadingPattern get_patt() {
                    return this->patt_;
                }
                // set / get file_name
                std::string const& get_file_name() {
                    return this->file_name_;
                }
                void set_file_name(std::string const& file_name) {
                    this->file_name_ = file_name;
                }
            private:
                ReadingPattern patt_ = ReadingPattern::ByLine;
                ReadingOption opt_ = ReadingOption::ByStdio;
                std::string file_name_;
                std::reference_wrapper<std::istream> ins_ = std::cin;
                std::reference_wrapper<std::ostream> outs_ = std::cout;
            };
            class SearchPattern {
            public:
                enum class Option{
                    ByString,
                    ByRegex
                };
                void set_pattern(Option opt = Option::ByString) {
                    opt_ = opt;
                }
                Option get_pattern() {
                    return this->opt_;
                }
            private:
                Option opt_ = Option::ByString;
            };
            class PrintMode{
            public:
                struct Option {
                    static constexpr std::uint64_t mode_default =     0x0000'0000'0000'0000;
                    static constexpr std::uint64_t block_number =      0x0000'0000'0000'0001;
                    static constexpr std::uint64_t file_name =        0x0000'0000'0000'0002;
                };
                bool have_flag(std::uint64_t mode) noexcept {
                    return static_cast<bool>((this->mode_ & mode) == mode);
                }
                void set_flag(std::uint64_t mode) noexcept {
                    this->mode_ |= mode;
                }
                void reset_flag() noexcept {
                    this->mode_ = Option::mode_default; 
                }
                std::uint64_t mode_ = Option::mode_default;
                std::optional<std::vector<std::size_t>> block_number_;
                std::optional<std::string> file_name_;
            };
            Color color_;
            Ios ios_;
            SearchPattern pattern_;
            PrintMode pmode_;
        }; // configuration for sgrep
    private:
        class Matches { 
        public:
            std::string block_;
            std::vector<std::pair<std::size_t, std::size_t>> mark_;
            explicit Matches(std::string block,
                std::vector<std::pair<std::size_t, std::size_t>> matchs) : 
                block_(std::move(block)), 
                mark_(std::move(matchs)) {}
            Matches(Matches const& other) = delete;
            Matches& operator=(Matches const& other) = delete;
            Matches(Matches &&other) {
                this->block_ = std::move(other.block_); 
                this->mark_= std::move(other.mark_);
            }
            Matches& operator=(Matches && other) = delete;
        };
    public:

        void print_block(Matches const& match) {
            using Mode = sgrep::Sgrep::Config::PrintMode::Option;
            std::ostream& out = this->conf_.ios_.get_outs().get();
            decltype (match.mark_)::size_type before = 0;
            for (   decltype(match.mark_)::size_type i = 0;
                    i < match.mark_.size();
                    ++i){
                out
                    << match.block_.substr(before, match.mark_[i].first)
                    << this->conf_.color_.get_begin_color()
                    << match.block_.substr(match.mark_[i].first, match.mark_[i].second - match.mark_[i].first)
                    << this->conf_.color_.get_end_color();
                before = match.mark_[i].second;
            }
            out << match.block_.substr(before);
        }
        void print_modifier() {
            using PrintOption = sgrep::Sgrep::Config::PrintMode::Option;
            if (this->conf_.pmode_.have_flag(PrintOption::file_name)) {
                if (this->conf_.pmode_.file_name_.has_value())
                    std::cout << "File_name : " << this->conf_.pmode_.file_name_.value() << std::endl;
            }            
        }
        void print_prefix(std::size_t i) {
            using PrintColor = Sgrep::Config::Color::Option;
            using PrintOption = sgrep::Sgrep::Config::PrintMode::Option;
            if (this->conf_.pmode_.have_flag(PrintOption::block_number)) {
                std::size_t size = std::string_view{std::to_string(this->results_.size())}.size();
                if (this->conf_.pmode_.block_number_.has_value())
                    std::cout
                        << std::left 
                        << std::setw(size) 
                        << PrintColor::thin_Green
                        << this->conf_.pmode_.block_number_.value()[i] 
                        << PrintColor::Reset
                        << " : ";
            }
        }
        void print() {
            this->print_modifier();
            for (std::size_t i = 0; i < this->results_.size(); ++i) {
                this->print_prefix(i);
                this->print_block(this->results_[i]);
                std::cout << std::endl;
            }
        }
        //sgrep
        bool read(std::string& block) {
            using ReadingPattern = sgrep::Sgrep::Config::Ios::ReadingPattern;
            ReadingPattern pattern = this->conf_.ios_.get_patt();
            if (pattern == ReadingPattern::ByLine) {
                return static_cast<bool>(std::getline(this->conf_.ios_.get_ins().get(), block));           
            }
            return false;
        }
        [[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>>
        handle_block(std::string& block) {
            using GrepPatt = sgrep::Sgrep::Config::SearchPattern::Option;
            std::vector<std::pair<std::size_t, std::size_t>> ret;
            std::string::size_type ind = 0;
            if (this->conf_.pattern_.get_pattern() == GrepPatt::ByString) {
                while (std::string::npos != (ind = block.find(this->get_target(), ind + 1))) {
                    ret.emplace_back(ind, ind + this->get_target().size());
                }
            } else { // ByRegex
                for (   std::sregex_iterator it {block.begin(), block.end(), this->get_regex()}, end;
                        it != end;
                        ++it) {
                    ret.emplace_back(it->position(0), it->length());
                }
            }
            return ret;
        }
        class ifstream_guard {
        public:
            explicit ifstream_guard(std::ifstream& ifs) :
                ifs_(ifs) 
            {}
            ~ifstream_guard() {
                ifs_.close();
            }
            ifstream_guard(const ifstream_guard& other) = delete;
            ifstream_guard& operator=(const ifstream_guard& other) = delete;
        private:
            std::ifstream& ifs_;
        };
        void handle() {
            using PrintOption = sgrep::Sgrep::Config::PrintMode::Option;
            using ReadingOption = sgrep::Sgrep::Config::Ios::ReadingOption;
            std::string block;
            std::vector<std::pair<std::size_t, std::size_t>> matches;
            std::size_t block_nu = 1;
            //设置读取方式
            std::ifstream ifs;
            ifstream_guard ifs_grd{ifs};
            if (ReadingOption::ByStdio == this->conf_.ios_.get_opt()) {
                this->conf_.ios_.set_in(std::cin);
            } else if (ReadingOption::ByFile == this->conf_.ios_.get_opt()) {
                ifs.open(this->conf_.ios_.get_file_name(), std::ios::in);
                this->conf_.ios_.set_in(ifs);
            }
            while (this->read(block)){
                matches = this->handle_block(block);
                if (!matches.empty()) {
                    if (this->conf_.pmode_.have_flag(PrintOption::block_number)) {
                        if (!this->conf_.pmode_.block_number_.has_value())
                            this->conf_.pmode_.block_number_.emplace();
                        this->conf_.pmode_.block_number_.value().push_back(block_nu);
                    }
                    this->results_.emplace_back(std::move(block), std::move(matches));
                }
                ++block_nu;
            }
            using PrintOption = sgrep::Sgrep::Config::PrintMode::Option;
        }
        void run() {
            this->handle();
            this->print();
        }
    public:
        const std::string& get_target() const {
            if (std::holds_alternative<std::string>(this->target_))
                return std::get<std::string>(target_);         
            else 
                throw sgrep_runtime_error("try to get regex from sgrep which has a config of string");
        }
        void set_target(const std::string& t) {
            this->target_ = t;            
        }
        const std::regex& get_regex() const {
            if (std::holds_alternative<std::regex>(this->target_))
                return std::get<std::regex>(this->target_);
            else
                throw sgrep_runtime_error("try to get string from sgrep which has a config of regex");
        }
        void set_regex(const std::regex& re) {
            this->target_ = re;
        }
    public:
        Config conf_;
    private:
        std::vector<Matches> results_;
        std::variant<std::string, std::regex> target_;
    };// Sgrep
};
#endif // SGREP_HPP_INCLUDED