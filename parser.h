#ifndef __PARSER_H_
#define __PARSER_H_
#include <vector>
#include "lexer.h"
using namespace std;
struct Error_Control{
    bool decl_error;
    string decl_print;
    bool mismatch;
    string mismatch_print;
};
//scope stuct has table of declared variables and their data types
//and a pointer pointed to its upper level scope called parent.
//When create a new scope, let the new scope parent be current scope,
//then let current scope equal the new scope where current scope is a global pointer.
class Scope{
    public:
        vector<Token> var_list;
        vector<Token> init_list;
        Scope *parent;
        Token lookup(Token t);
        Token lookup_init_list(Token t);
};

class Parser{
    private:
    //Parser Section:
        LexicalAnalyzer lex;
        void syntax_error();
        Token expect(TokenType expected_type);
        void parse_scope(); //create a scope var, check decl error III at end of this function
        void parse_scope_list();
        void parse_var_decl();
        void parse_id_list(vector<Token> &symbols);//check decl error I
        TokenType parse_type_name();//return value usd for declaration.
        void parse_stmt_list();
        void parse_stmt();
        void parse_assign_stmt();//reference place decl error II
        void parse_while_stmt();
        TokenType parse_expr();
        void parse_arithmetic_operator();
        void parse_binary_boolean_operator();
        void parse_relational_operator();
        TokenType parse_primary();//reference place decl error II
        TokenType parse_bool_const();
        void parse_condition();
    public:
        vector<pair<Token, Token> > result_list;//first is decl_var, second is refer_var.
        vector<Token> Uninitialized;
        Scope *current;
        Error_Control ec;
        Parser();
        void parse_program();
        void print_uninitial();
        void print_result();
};

#endif