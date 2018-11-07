#include <iostream>
#include <cstdlib>
#include "parser.h"

using namespace std;

int main(){
    Parser p;
    p.parse_program();
    if(p.ec.decl_error==true){
        cout << p.ec.decl_print << endl;
        return 0;
    }
    if(p.ec.mismatch==true){
        cout << p.ec.mismatch_print << endl;
        return 0;
    }
    else if(p.Uninitialized.size()>0){
        p.print_uninitial();
        return 0;
    }
    else{
        p.print_result();
        return 0;
    }
}
void Parser::print_uninitial(){
    for(auto i=Uninitialized.begin();i!=Uninitialized.end();++i){
        cout<<"UNINITIALIZED "<<i->lexeme<<" "<<i->line_no<<endl;
    }
}
void Parser::print_result(){
    for(auto i=result_list.begin();i!=result_list.end();++i){
        cout << i->first.lexeme <<" "<<i->second.line_no<<" "<<i->first.line_no<<endl; 
    }
}
void Parser::syntax_error(){
    cout << "Syntax Error" << endl;
    exit(1);
}//done!

Token Parser::expect(TokenType expected_type){
    Token t = lex.GetToken();
    if(t.token_type != expected_type){
        syntax_error();
    }
    return t;
}//done!

void Parser::parse_scope(){
    //Entry scope
    Scope *s = new Scope();
    s->parent = current;
    current = s;
    //Parser
    expect(LBRACE);
    parse_scope_list();
    expect(RBRACE);
    //There are some var declared in this scope, check 1.3
    if(current->var_list.size()>0){
        //Check declare error III:
        Token t1;
        bool has_error = true;
        for(auto i=current->var_list.begin();i!=current->var_list.end();++i){
            //has_error = true;
            //for each declared vars in the current scope
            for(auto j=result_list.begin();j!=result_list.end();++j){
                //find the same lexeme and the same line_no
                if(i->lexeme==j->first.lexeme&&i->line_no==j->first.line_no){
                    has_error = false;
                    break;
                }
            }
            if(has_error==true){
                t1 = *i;
                break;
            }
        }
        if(has_error==true&&ec.decl_error==false){
            ec.decl_error = true;
            ec.decl_print = "ERROR CODE 1.3 " + t1.lexeme;
        }
    }
    //Exit scope
    Scope *temp;
    temp = current;
    current = current->parent;
    free(temp);
}//done!

void Parser::parse_scope_list(){
    //first is LBRACE -> scope
    //first is WHILE -> stmt
    //first is ID+EQUAL -> stmt
    //first is ID+else -> var_decl
    Token t = lex.GetToken();
    if(t.token_type==LBRACE){
        lex.UngetToken(t);
        parse_scope();
        t = lex.GetToken();
        if(t.token_type==RBRACE){
            lex.UngetToken(t);
            return;
        }
        else{
            lex.UngetToken(t);
            parse_scope_list();
        }
    }
    else if(t.token_type==WHILE){
        lex.UngetToken(t);
        parse_stmt();
        t = lex.GetToken();
        if(t.token_type==RBRACE){
            lex.UngetToken(t);
            return;
        }
        else{
            lex.UngetToken(t);
            parse_scope_list();
        }
    }
    else if(t.token_type==ID){
        Token t1 = lex.GetToken();
        if(t1.token_type==EQUAL){
            lex.UngetToken(t1);
            lex.UngetToken(t);
            parse_stmt();
            t = lex.GetToken();
            if(t.token_type==RBRACE){
                lex.UngetToken(t);
                return;
            }
            else{
                lex.UngetToken(t);
                parse_scope_list();
            }
        }
        else{
            lex.UngetToken(t1);
            lex.UngetToken(t);
            parse_var_decl();
            t = lex.GetToken();
            if(t.token_type==RBRACE){
                lex.UngetToken(t);
            }
            else{
                lex.UngetToken(t);
                parse_scope_list();
            }
        }
    }
    else{
        syntax_error();
    }
    
}//done!

void Parser::parse_var_decl(){
    vector<Token> symbols;
    TokenType tp;
    parse_id_list(symbols);
    expect(COLON);
    tp = parse_type_name();
    expect(SEMICOLON);
    for(auto i=symbols.begin();i!=symbols.end();++i){
        i->token_type = tp;
    }//change token type to var type.
    for(auto i=symbols.begin();i!=symbols.end();++i){
        current->var_list.push_back(*i);
    }//add to scope var list.
}//done!

void Parser::parse_id_list(vector<Token> &Symbols){
    Token t = lex.GetToken();
    if(t.token_type == ID){
        //Check declare error I:
        for(auto i=current->var_list.begin();i!=current->var_list.end();++i){
            if(t.lexeme==i->lexeme&&ec.decl_error==false){
                ec.decl_error = true;
                ec.decl_print = "ERROR CODE 1.1 " + t.lexeme;
                break;
            }
        }//not declared
        if(ec.decl_error==false){
            for(auto i=Symbols.begin();i!=Symbols.end();++i){
                if(t.lexeme==i->lexeme){
                    ec.decl_error = true;
                    ec.decl_print = "ERROR CODE 1.1 " + t.lexeme;
                    break;
                }
            }//not in one id_list
            if(ec.decl_error==false){
                Symbols.push_back(t);
            }//no repeat
        }
        t = lex.GetToken();
        if(t.token_type==COLON){
            lex.UngetToken(t);
        }
        else if(t.token_type==COMMA){
            parse_id_list(Symbols);
        }
        else{
            syntax_error();
        }
    }
    else{
        syntax_error();
    }
}//done!

TokenType Parser::parse_type_name(){
    Token t = lex.GetToken();
    if(t.token_type==REAL||t.token_type==INT||t.token_type==BOOLEAN||t.token_type==STRING)
    {
        return t.token_type;
    }
    else{
        syntax_error();
    }
    
}//done!

void Parser::parse_stmt_list(){
    Token t = lex.GetToken();
    if(t.token_type==ID||t.token_type==WHILE){
        lex.UngetToken(t);
        parse_stmt();
        t = lex.GetToken();
        if(t.token_type==RBRACE){
            lex.UngetToken(t);
        }
        else{
            lex.UngetToken(t);
            parse_stmt_list();
        }
    }
    else{
        syntax_error();
    }
}//done!

void Parser::parse_stmt(){
    Token t = lex.GetToken();
    if(t.token_type==ID){
        lex.UngetToken(t);
        parse_assign_stmt();
    }
    else if(t.token_type==WHILE){
        lex.UngetToken(t);
        parse_while_stmt();
    }
    else{
        syntax_error();
    }
}//done!

void Parser::parse_assign_stmt(){
    TokenType tp;
    Token t = lex.GetToken();
    if(t.token_type==ID){
        //Check declare error type II:
        Token t1 = current->lookup(t);
        if(t1.token_type==ERROR&&ec.decl_error==false){
            ec.decl_error = true;
            ec.decl_print = "ERROR CODE 1.2 "+t.lexeme;
        }//undeclared
        else if(t1.token_type!=ERROR){
            pair<Token, Token> p;
            p.first=t1;
            p.second=t;
            result_list.push_back(p);
        }
        //Initialization Section
        current->init_list.push_back(t);//var appears on the lhs.
        expect(EQUAL);
        tp = parse_expr();
        expect(SEMICOLON);
        //Type Mismatch
        if(t1.token_type == REAL){
            if(tp!=REAL&&tp!=INT&&ec.mismatch==false){
                ec.mismatch = true;
                ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C2";
            }//C2
        }
        else if(t1.token_type!=tp&&ec.mismatch==false){
            ec.mismatch = true;
            ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C1";
        }//C1    
    }
    else{
        syntax_error();
    }
    
}//done!

void Parser::parse_while_stmt(){
    expect(WHILE);
    parse_condition();
    Token t = lex.GetToken();
    if(t.token_type==LBRACE){
        //Entry scope
        Scope *s = new Scope();
        s->parent = current;
        current = s;
        parse_stmt_list();
        expect(RBRACE);
        //There are some var declared in this scope, check 1.3 error
        if(current->var_list.size()>0){
            //Check declare error III:
            Token t1;
            bool has_error = true;
            for(auto i=current->var_list.begin();i!=current->var_list.end();++i){
                //has_error = true;
                //for each declared vars in the current scope
                for(auto j=result_list.begin();j!=result_list.end();++j){
                    //find the same lexeme and the same line_no
                    if(i->lexeme==j->first.lexeme&&i->line_no==j->first.line_no){
                        has_error = false;
                        break;
                    }
                }
                if(has_error==true){
                    t1 = *i;
                    break;
                }
            }
            if(has_error==true&&ec.decl_error==false){
                ec.decl_error = true;
                ec.decl_print = "ERROR CODE 1.3 " + t1.lexeme;
            }
        }
        //Exit scope
        Scope *temp;
        temp = current;
        current = current->parent;
        free(temp);
    }
    else if(t.token_type==WHILE||t.token_type==ID){
        lex.UngetToken(t);
        parse_stmt();
    }
    else{
        syntax_error();
    }
}//done!

TokenType Parser::parse_expr(){
    Token t = lex.GetToken();
    TokenType tp1;
    TokenType tp2;
    if(t.token_type==PLUS||t.token_type==MINUS||t.token_type==MULT||t.token_type==DIV){
        lex.UngetToken(t);
        parse_arithmetic_operator();
        tp1 = parse_expr();
        tp2 = parse_expr();
        //Type mismatch C3
        if(tp1==REAL){
            if(tp2==INT||tp2==REAL){
                return REAL;
            }//I1
            else if(ec.mismatch==false){
                ec.mismatch = true;
                ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C3";
                return ERROR;
            }
        }
        else if(tp1==INT){
            if(tp2==INT){
                if(t.token_type==DIV){
                    return REAL;
                }//I3
                else{
                    return INT;
                }//I2
            }
            else if(tp2==REAL){
                return REAL;
            }//I1
            else if(ec.mismatch==false){
                ec.mismatch = true;
                ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C3";
                return ERROR;
            }
        }
        else if(ec.mismatch==false){
            ec.mismatch = true;
            ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C3";
            return ERROR;
        }
    }
    else if(t.token_type==AND||t.token_type==OR||t.token_type==XOR){
        lex.UngetToken(t);
        parse_binary_boolean_operator();
        tp1 = parse_expr();
        tp2 = parse_expr();
        //Type Mismatch
        if(tp1==BOOLEAN&&tp2==BOOLEAN){
            return BOOLEAN;//I4
        }//C4
        else if(ec.mismatch==false){
            ec.mismatch = true;
            ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C4";
            return ERROR;
        }
    }
    else if(t.token_type==GREATER||t.token_type==GTEQ||t.token_type==LESS||t.token_type==NOTEQUAL||t.token_type==LTEQ){
        lex.UngetToken(t);
        parse_relational_operator();
        tp1 = parse_expr();
        tp2 = parse_expr();
        //Type Mismatch
        if((tp1==STRING&&tp2==STRING)||(tp1==BOOLEAN&&tp2==BOOLEAN)){
            return BOOLEAN;
        }//C5
        else if(tp1==INT||tp1==REAL){
            if(tp2==INT||tp2==REAL){
                return BOOLEAN;
            }//C6
            else if(ec.mismatch==false){
                ec.mismatch = true;
                ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C6";
                return ERROR;
            }
        }
        else if(ec.mismatch==false){
            ec.mismatch = true;
            ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C5";
            return ERROR;
        }
    }
    else if(t.token_type==NOT){
        TokenType tp;
        tp = parse_expr();
        //Type Mismatch
        if(tp==BOOLEAN){
            return BOOLEAN;
        }
        /*else if(tp!=BOOLEAN&&ec.mismatch==false){
            ec.mismatch = true;
            ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C4";
            return ERROR;
        }*/
    }//C4
    else if(t.token_type==ID||t.token_type==NUM||t.token_type==REALNUM||t.token_type==STRING_CONSTANT||t.token_type==TRUE||t.token_type==FALSE){
        lex.UngetToken(t);
        return parse_primary();
    }
    else{
        syntax_error();
    }
}//done!

void Parser::parse_arithmetic_operator(){
    Token t = lex.GetToken();
    if(t.token_type!=PLUS&&t.token_type!=MINUS&&t.token_type!=MULT&&t.token_type!=DIV){
        syntax_error();
    }
}//done!

void Parser::parse_binary_boolean_operator(){
    Token t = lex.GetToken();
    if(t.token_type!=AND&&t.token_type!=OR&&t.token_type!=XOR){
        syntax_error();
    }
}//done!

void Parser::parse_relational_operator(){
    Token t = lex.GetToken();
    if(t.token_type!=GREATER&&t.token_type!=GTEQ&&t.token_type!=LESS&&t.token_type!=NOTEQUAL&&t.token_type!=LTEQ){
        syntax_error();
    }
}//done!

TokenType Parser::parse_primary(){
    Token t = lex.GetToken();
    if(t.token_type == TRUE||t.token_type == FALSE){
        lex.UngetToken(t);
        return parse_bool_const();
    }
    else if(t.token_type==ID){
        Token t1;
        t1 = current->lookup(t);
        //Check declare error II here:
        if(t1.token_type==ERROR&&ec.decl_error==false){
            ec.decl_error = true;
            ec.decl_print = "ERROR CODE 1.2 "+t.lexeme;
        }//undeclared
        else if(t1.token_type!=ERROR){
            pair<Token, Token> p;
            p.first=t1;
            p.second=t;
            result_list.push_back(p);
            Token t2 = current->lookup_init_list(t);
            if(t2.token_type==ERROR){
                Uninitialized.push_back(t);
            }//Uninitialized
            return t1.token_type;
        }//sccessfully use reference
    }
    else if(t.token_type==NUM){
        return INT;
    }
    else if(t.token_type==REALNUM){
        return REAL;
    }
    else if(t.token_type==STRING_CONSTANT){
        return STRING;
    }
    else{
        syntax_error();
    }
} //done!

TokenType Parser::parse_bool_const(){
    Token t = lex.GetToken();
    if(t.token_type==TRUE||t.token_type==FALSE){
        return BOOLEAN;
    }
    else{
        syntax_error();
    }
}//done!

void Parser::parse_condition(){
    TokenType tp;
    Token t;
    t = expect(LPAREN);
    tp = parse_expr();
    expect(RPAREN);
    if(tp!=BOOLEAN&&ec.mismatch==false){
        ec.mismatch = true;
        ec.mismatch_print = "TYPE MISMATCH "+to_string(t.line_no)+" C7";
    }//C7
}//done!

void Parser::parse_program(){
    parse_scope();
}//done!

Parser::Parser(){
    vector<pair<Token, Token> > result_list;//first is decl_var, second is refer_var.
    vector<Token> Uninitialized;
    ec.decl_error=false;
    ec.decl_print="";
    ec.mismatch=false;
    ec.mismatch_print="";
    this->current = (struct Scope*)malloc(sizeof(struct Scope));
    this->result_list = result_list;
    this->Uninitialized = Uninitialized;
}//constructor
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Token Scope::lookup(Token t){
    for(auto i=var_list.begin();i!=var_list.end();++i){
        if(t.lexeme==i->lexeme){
            return *i;
        }
    }
    if(parent!=NULL){
        return parent->lookup(t);
    }
    else{
        Token t1;
        t1.token_type = ERROR;
        return t1;
    }
}
Token Scope::lookup_init_list(Token t){
    for(auto i=init_list.begin();i!=init_list.end();++i){
        if(t.lexeme==i->lexeme){
            return *i;
        }
    }
    if(parent!=NULL){
        return parent->lookup(t);
    }
    else{
        Token t1;
        t1.token_type = ERROR;
        return t1;
    }
}