#include <assert.h>
#include <stdlib.h>

#include "util.hpp"
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

using namespace std;

enum class Token_Type {
    // Single-character tokens.
    L_Paren,
    R_Paren,
    L_Brace,
    R_Brace,
    Comma,
    Minus,
    Plus,
    Slash,
    Star,
    Quote,

    // Comparators / Control flow.
    EQ,
    NE,
    GT,
    LT,
    AND,
    OR,
    FALSE,
    TRUE,
    ADDASSIGN,

    // Turtle control builtins.
    PENUP,
    PENDOWN,
    FORWARD,
    BACK,
    LEFT,
    RIGHT,
    SETPENCOLOR,
    TURN,
    SETHEADING,
    SETX,
    SETY,

    // Queries.
    XCOR,
    YCOR,
    HEADING,
    COLOR,

    // Function and variable identifier stuffs.
    Identifier,
    // we can determine when lexing whether the token
    // denotes defining vs using a variable
    VariableIdentifier,
    VariableUse,
    Fn,
    EndFn,
    Number,

    // Control flow stuffs.
    MAKE,
    IF,
    WHILE,

    // Misc.
    Comment,
    InvalidToken,
};

enum class Statement_Type {
    Function_Decleration,
    Variable_Decleration,
    Pen_Movement,
    Pen_Color,
    Add_Assign,
    If,
    While,
    Comment,
    Erroneous
};

typedef variant<float, bool> Logo_Value;

struct Pen_State {
    tuple<float> pos;
    bool         down;
};

Pen_State               pen_state;
map<string, Logo_Value> variables;

vector<string> split(string s, char seperator) {
    vector<string> words;
    string         word;
    for (char ch : s) {
        if (ch == seperator) {
            if (word.length() > 0) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += ch;
        }
    }
    if (word.length() > 0) {
        words.push_back(word);
    }
    return words;
}

Token_Type get_type_from_token(string s) {
    string tmp = "";
    // Too lazy to change the if statements :)) so we have this hack.
    for (auto ch : s) {
        tmp += tolower(ch);
    }
    s = tmp;
    if (s == "(") return Token_Type::L_Paren;
    if (s == ")") return Token_Type::R_Paren;
    if (s == "{") return Token_Type::L_Brace;
    if (s == "}") return Token_Type::R_Brace;
    if (s == ",") return Token_Type::Comma;
    if (s == "-") return Token_Type::Minus;
    if (s == "+") return Token_Type::Plus;
    if (s == "/") return Token_Type::Slash;
    if (s == "*") return Token_Type::Star;
    if (s == "\"") return Token_Type::Quote;

    if (s == "==") return Token_Type::EQ;
    if (s == "!=") return Token_Type::NE;
    if (s == ">") return Token_Type::GT;
    if (s == "<") return Token_Type::LT;
    if (s == "and") return Token_Type::AND;
    if (s == "or") return Token_Type::OR;
    if (s == "false") return Token_Type::FALSE;
    if (s == "true") return Token_Type::TRUE;
    if (s == "addassign") return Token_Type::ADDASSIGN;

    if (s == "penup") return Token_Type::PENUP;
    if (s == "pendown") return Token_Type::PENDOWN;
    if (s == "forward") return Token_Type::FORWARD;
    if (s == "back") return Token_Type::BACK;
    if (s == "left") return Token_Type::LEFT;
    if (s == "right") return Token_Type::RIGHT;
    if (s == "setpencolor") return Token_Type::SETPENCOLOR;
    if (s == "turn") return Token_Type::TURN;
    if (s == "setheading") return Token_Type::SETHEADING;
    if (s == "setx") return Token_Type::SETX;
    if (s == "sety") return Token_Type::SETY;

    if (s == "xcor") return Token_Type::XCOR;
    if (s == "ycor") return Token_Type::YCOR;
    if (s == "heading") return Token_Type::HEADING;
    if (s == "color") return Token_Type::COLOR;

    if (s == "to") return Token_Type::Fn;
    if (s == "end") return Token_Type::EndFn;
    if (s == "make") return Token_Type::MAKE;
    if (s == "if") return Token_Type::IF;
    if (s == "while") return Token_Type::WHILE;

    if (s == "//") return Token_Type::Comment;

    return Token_Type::InvalidToken;
}

vector<Token_Type> parse_out_tokens(vector<string> string_tokens) {
    vector<Token_Type> tokens;
    for (auto string_token : string_tokens) {
        tokens.push_back(get_type_from_token(string_token));
    }
    return tokens;
}

Statement_Type parse_statement_type(Token_Type token) {
    using enum Token_Type;
    switch (token) {
        case PENUP:
        case PENDOWN:
        case FORWARD:
        case BACK:
        case LEFT:
        case RIGHT:
        case TURN:
        case SETHEADING:
        case SETX:
        case SETY:
            return Statement_Type::Pen_Movement;
        case SETPENCOLOR:
            return Statement_Type::Pen_Color;
        case ADDASSIGN:
            return Statement_Type::Add_Assign;
        case Fn:
            return Statement_Type::Function_Decleration;
        case MAKE:
            return Statement_Type::Variable_Decleration;
        case IF:
            return Statement_Type::If;
        case WHILE:
            return Statement_Type::While;
        case Comment:
            return Statement_Type::Comment;
        case InvalidToken:
            return Statement_Type::Erroneous;
        default:
            return Statement_Type::Erroneous;
    }
}

void parse_pen_movement(vector<Token_Type> tokens) {}
void parse_pen_color_change(vector<Token_Type> tokens) {}
void parse_variable_decleration(vector<Token_Type> tokens) {}
void parse_function_decleration(vector<Token_Type> tokens) {}
void parse_add_assign(vector<Token_Type> tokens) {}
// void parse_if(vector<string> tokens) {}
// void parse_while(vector<string> tokens) {}

void parse_line(string line) {
    auto string_tokens  = split(line, ' ');
    auto tokens         = parse_out_tokens(string_tokens);
    auto statement_type = parse_statement_type(tokens[0]);
    switch (statement_type) {
        case Statement_Type::Pen_Movement:
            return parse_pen_movement(tokens);
        case Statement_Type::Variable_Decleration:
            return parse_variable_decleration(tokens);
        case Statement_Type::Add_Assign:
            return parse_add_assign(tokens);
        case Statement_Type::Comment:
            return;
        case Statement_Type::Erroneous:
            puts("This file seems messed up!!! Goodbye (:");
            exit(-1);
        default:
            puts("Trying to parse a type of statement we don't know how to handle yet... Goodbye (:");
    }
}

int main(void) {
    auto contents = read_file("./logo_examples/1_08_harder_combo.lg");
    auto lines    = split(contents, '\n');
    for (auto line : lines) {
        parse_line(line);
    }
}