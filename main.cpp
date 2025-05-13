#include <assert.h>
#include <cmath>
#include <iostream>
#include <optional>
#include <stdlib.h>

#define OLIVEC_IMPLEMENTATION
#include "third_party/olive.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third_party/stb_image_write.h"

#include "util.hpp"
#include <cstdio>
#include <map>
#include <string>
#include <variant>
#include <vector>

using namespace std;
typedef variant<float, bool, string> Logo_Value;

constexpr uint16_t IMG_WIDTH  = 1000;
constexpr uint16_t IMG_HEIGHT = 1000;
Olivec_Canvas      canvas;

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

struct Pen_State {
    bool  down;
    float pos[2];
    float direction;
};

enum class Direction { Forward, Back, Left, Right };

vector<std::string>  split(string s, char seperator);
Token_Type           get_type_from_token(std::string s);
optional<Logo_Value> get_variable_value(string name, Logo_Value initial_value);
vector<Token_Type>   parse_out_tokens(vector<string> string_tokens);
Statement_Type       parse_statement_type(Token_Type token);
void                 parse_pen_movement(vector<Token_Type> tokens, std::vector<std::string> string_tokens);
void                 parse_pen_color_change(vector<Token_Type> tokens);
void                 parse_variable_decleration(vector<Token_Type> tokens);
void                 parse_function_decleration(vector<Token_Type> tokens);
void                 parse_add_assign(vector<Token_Type> tokens);

void draw_line(float start[2], float end[2]);

Pen_State               pen_state;
map<string, Logo_Value> variables;

void parse_line(string line) {
    auto string_tokens  = split(line, ' ');
    auto tokens         = parse_out_tokens(string_tokens);
    auto statement_type = parse_statement_type(tokens[0]);
    using enum Statement_Type;
    switch (statement_type) {
        case Pen_Movement:
            return parse_pen_movement(tokens, string_tokens);
        case Pen_Color:
            return parse_pen_color_change(tokens);
        case Variable_Decleration:
            return parse_variable_decleration(tokens);
        case Add_Assign:
            return parse_add_assign(tokens);
        case Comment:
            return;
        case Erroneous:
            puts("This file seems messed up!!! Goodbye (:");
            exit(-1);
        default:
            puts("Trying to parse a type of statement we don't know how to handle yet... Goodbye (:");
    }
}

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

// This function assumes you've checked the variables map once before, and passed in the value.
optional<Logo_Value> get_variable_value(string name, Logo_Value initial_value) {
    if (holds_alternative<string>(initial_value)) {
        auto s = get<string>(initial_value);
        if (s[0] == ':') {
            // check if value is in the map
            if (variables.contains(s)) {
                return get_variable_value(s, variables[s]);
            } else {
                return nullopt;
            }
        } else {
            // variables cannot be set to normal strings.
            return nullopt;
        }
    } else {
        return initial_value;
    }
}

// This will draw a line if the pen is down. This is the only
// function that draws a line.
void move_pen(Direction direction, float amount) {
    auto  angle_radians = degree_to_radians(pen_state.direction);
    float new_pos[2];
    switch (direction) {
        case Direction::Forward:
            new_pos[0] += cos(angle_radians) * amount;
            new_pos[1] += sin(angle_radians) * amount;
        case Direction::Back:
            new_pos[0] -= cos(angle_radians) * amount;
            new_pos[1] -= sin(angle_radians) * amount;
        case Direction::Left:
            new_pos[0] += cos(angle_radians + numbers::pi * 2) * amount;
            new_pos[1] += sin(angle_radians + numbers::pi * 2) * amount;
        case Direction::Right:
            new_pos[0] += cos(angle_radians - numbers::pi * 2) * amount;
            new_pos[1] += sin(angle_radians - numbers::pi * 2) * amount;
    }

    if (pen_state.down) {
        cout << "drawing line from " << pen_state.pos << " - " << new_pos << endl;
        draw_line(pen_state.pos, new_pos);
    }
    pen_state.pos[0] = new_pos[0];
    pen_state.pos[1] = new_pos[1];
}

void parse_pen_movement(vector<Token_Type> tokens, vector<string> string_tokens) {
    float amount;
    using enum Token_Type;
    switch (tokens[0]) {
        case PENDOWN:
            pen_state.down = true;
            return;
        case PENUP:
            pen_state.down = false;
            return;
        default:
            // Indicates wheter amount comes from a variable.
            if (string_tokens[1][0] == ':') {
                auto variable_name = string_tokens[1].substr(1);
                if (variables.contains(variable_name)) {
                    auto res = get_variable_value(variable_name, variables[variable_name]);
                    if (res) {
                        amount = get<float>(*res);
                    } else {
                        printf("ERROR! Tried to get value of '%s', but no variable with name '%s' exists.",
                               variable_name.c_str(), variable_name.c_str());
                    }
                } else {
                    printf("ERROR! Tried to get value of '%s', but no variable with name '%s' exists.",
                           variable_name.c_str(), variable_name.c_str());
                    exit(-1);
                }
            } else {
                cout << "trying to convert to float: " << string_tokens[1];
                amount = stof(string_tokens[1].substr(1));
                cout << endl << "found amount: " << amount << endl;
            }
    }
    switch (tokens[0]) {
        case FORWARD:
            move_pen(Direction::Forward, amount);
        case BACK:
            move_pen(Direction::Back, amount);
        case LEFT:
            move_pen(Direction::Left, amount);
        case RIGHT:
            move_pen(Direction::Right, amount);
        case TURN:
            pen_state.direction += amount;
        case SETHEADING:
            pen_state.direction = amount;
            return;
        case SETX:
            pen_state.pos[0] = amount;
            return;
        case SETY:
            pen_state.pos[1] = amount;
            return;
        default:
            cout << "you done fucked up!!!" << endl;
            exit(-1);
    }
}

void parse_pen_color_change(std::vector<Token_Type> tokens) {}
void parse_variable_decleration(std::vector<Token_Type> tokens) {}
void parse_function_decleration(std::vector<Token_Type> tokens) {}
void parse_add_assign(std::vector<Token_Type> tokens) {}
int  main(void) {
    // Initialize canvas.
    uint32_t pixels[IMG_WIDTH * IMG_HEIGHT];
    canvas = olivec_canvas(pixels, IMG_WIDTH, IMG_HEIGHT, IMG_WIDTH);

    // Initialize app state.
    pen_state.direction = 0;
    pen_state.down      = false;
    pen_state.pos[0]    = 1.0;
    pen_state.pos[1]    = 0.0;
    auto contents       = read_file("./logo_examples/1_08_harder_combo.lg");
    auto lines          = split(contents, '\n');
    for (auto line : lines) {
        cout << line;
        parse_line(line);
    }
}
/*
    Renderer stuff.
*/
void draw_line(float start[2], float end[2]) {
    // need to do proper translation of pen colors (4 bits) to hex(32 bits).
    olivec_line(canvas, start[0], start[1], start[0], start[1], 0xffaa27ff);
}