#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>
#include <unordered_set>

using namespace std;

namespace parse
{

    bool operator==(const Token &lhs, const Token &rhs)
    {
        using namespace token_type;

        if (lhs.index() != rhs.index())
        {
            return false;
        }
        if (lhs.Is<Char>())
        {
            return lhs.As<Char>().value == rhs.As<Char>().value;
        }
        if (lhs.Is<Number>())
        {
            return lhs.As<Number>().value == rhs.As<Number>().value;
        }
        if (lhs.Is<String>())
        {
            return lhs.As<String>().value == rhs.As<String>().value;
        }
        if (lhs.Is<Id>())
        {
            return lhs.As<Id>().value == rhs.As<Id>().value;
        }
        return true;
    }

    bool operator!=(const Token &lhs, const Token &rhs)
    {
        return !(lhs == rhs);
    }

    std::ostream &operator<<(std::ostream &os, const Token &rhs)
    {
        using namespace token_type;

#define VALUED_OUTPUT(type)         \
    if (auto p = rhs.TryAs<type>()) \
        return os << #type << '{' << p->value << '}';

        VALUED_OUTPUT(Number);
        VALUED_OUTPUT(Id);
        VALUED_OUTPUT(String);
        VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>())       \
        return os << #type;

        UNVALUED_OUTPUT(Class);
        UNVALUED_OUTPUT(Return);
        UNVALUED_OUTPUT(If);
        UNVALUED_OUTPUT(Else);
        UNVALUED_OUTPUT(Def);
        UNVALUED_OUTPUT(Newline);
        UNVALUED_OUTPUT(Print);
        UNVALUED_OUTPUT(Indent);
        UNVALUED_OUTPUT(Dedent);
        UNVALUED_OUTPUT(And);
        UNVALUED_OUTPUT(Or);
        UNVALUED_OUTPUT(Not);
        UNVALUED_OUTPUT(Eq);
        UNVALUED_OUTPUT(NotEq);
        UNVALUED_OUTPUT(LessOrEq);
        UNVALUED_OUTPUT(GreaterOrEq);
        UNVALUED_OUTPUT(None);
        UNVALUED_OUTPUT(True);
        UNVALUED_OUTPUT(False);
        UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

        return os << "Unknown token :("sv;
    }

    Lexer::Lexer(std::istream &input){
        ParseTokens(input);
    }

    const Token &Lexer::CurrentToken() const{


        if (current_token_ >= token_base_.size()){
            return token_base_.back();
        }
        return token_base_[current_token_];
    }

    Token Lexer::NextToken(){
        if (token_base_.empty()){
            return token_type::Eof{};
        }
        ++current_token_;
        return CurrentToken();
    }

    void Lexer::ParseTokens(std::istream &input)
    {
        std::string line;
        while (getline(input, line)){
            if(line.empty()){
                continue;
            }
            std::istringstream in(line);
            ParseDend(in);
            while (in){
                char c = in.peek();
                if (isdigit(c)){
                    ParseNumber(in);
                }
                else if (c == '\'' || c == '"'){
                    ParseString(in);
                }
                else if (MARKS.count(c)){
                    ParseOperation(in);
                }
                else if (c == '#'){
                    break;
                }
                else{
                    ParseKeyWordsOrIds(in);

                }
            }
            if (!token_base_.empty() && !token_base_.back().Is<token_type::Newline>()){
                token_base_.push_back(token_type::Newline{});
            }

        }
        if (dend_number_ > 0){
            for (size_t i = 0; i < dend_number_; ++i){
               token_base_.push_back(token_type::Dedent{});
            }
        }

        token_base_.push_back(token_type::Eof{});
    }

    void Lexer::ParseString(std::istream &input)
    {
        std::string s;
        const char quote = input.get();
        while (true){
            const char ch = input.get();
            if (ch == quote){
                break;
            }else if (ch == '\\')
            {
                const char symbol = input.get();
                switch (symbol)
                {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\'':
                    s.push_back('\'');
                    break;
                }
            }
            else{
                s.push_back(ch);
            }
        }
        token_base_.emplace_back(token_type::String{s});
    }

    void Lexer::ParseNumber(std::istream &input)
    {
        std::string parsed_num;
        while (std::isdigit(input.peek()))
        {
            parsed_num += static_cast<char>(input.get());
        }

        token_base_.emplace_back(token_type::Number{std::stoi(parsed_num)});
    }

    void Lexer::ParseKeyWordsOrIds(std::istream &input){
        std::string s;
        char c = input.get();
        while (c != ' ' && c != EOF && c != '\n'){
            if (c == '#' || MARKS.count(c)){
                input.putback(c);
                break;
            }
            s += c;
            c = input.get();
        }
        if (!s.empty()){

           const auto it = KEY_WORDS.find(s);
            if(it != KEY_WORDS.end()){
                token_base_.push_back(it->second);
            }
            else{
                token_base_.push_back(token_type::Id{s});
            }
        }
    }

    void Lexer::ParseDend(std::istream &input)
    {
        size_t spaces_number = 0;
        if (input.peek() == ' ')
            while (input.peek() == ' ')
            {
                input.get();
                ++spaces_number;
            }
        if (dend_number_ < spaces_number / 2){
            for (size_t i = 0; i < (spaces_number / 2) - dend_number_; ++i){

                token_base_.push_back(token_type::Indent{});
            }
        }
        else if (dend_number_ > spaces_number / 2){
            for (size_t i = 0; i < dend_number_ - (spaces_number / 2); ++i){

                token_base_.push_back(token_type::Dedent{});
            }
        }
        dend_number_ = spaces_number / 2;
    }

    void Lexer::ParseOperation(std::istream &input)
    {
        char c = input.get();
        if (c == '!' && input.peek() == '='){

            token_base_.push_back(token_type::NotEq{});
            input.get();
        }
        else if (c == '=' && input.peek() == '='){
            token_base_.push_back(token_type::Eq{});
            input.get();
        }
        else if (c == '<' && input.peek() == '='){
            token_base_.push_back(token_type::LessOrEq{});
            input.get();
        }
        else if (c == '>' && input.peek() == '='){
                        token_base_.push_back(token_type::GreaterOrEq{});
            input.get();
        }
        else{
            token_base_.push_back(token_type::Char{c});
        }
    }

} // namespace parse
