/*
 * File by: Alex Wallace
 * Course: CSC-4510 Programming Language Design and Translation
 * Date: November 2026
 *
 * parser.h — recursive-descent LL(1) parser for lille. 
 * One function per grammar nonterminal
 * FIRST and FOLLOW sets drive main design and grammar decisions
 */



#ifndef PARSER_H
#define PARSER_H


#include "scanner.h"
#include "error_handler.h"
#include "id_table.h"


struct ident_info {
    string name;
    int line, pos;
};

struct param_info {
    ident_info id;
    lille_type type;
    lille_kind mode;
};

class parser {
    private:

    scanner* scan;
    id_table* id_tab;
    error_handler* error;
    int loop_depth {0};
    vector<lille_type> callable_stack;
    vector<string> block_name_stack; 

    // Main parser grammar function declarations

    void parse_block(vector<param_info> params = {});
    void parse_statement_list();
    void parse_statement();
    void parse_declaration();
    vector<ident_info> parse_ident_list();
    lille_type parse_type();
    vector<param_info> parse_param_list();
    vector<param_info> parse_param();
    lille_kind parse_param_kind();
    lille_type parse_expression();
    lille_type parse_simp_expression();
    void parse_range();
    lille_type parse_expr_2();
    lille_type parse_term();
    lille_type parse_factor();
    lille_type parse_primary();
    void parse_write();
    void parse_write_line();
    void parse_simp_statement();
    void parse_exit();
    void parse_return();
    void parse_read();
    void parse_if();
    void parse_loop();
    void parse_while();
    void parse_for();

    // Helper functions

    bool relation_op_follows();
    bool expression_can_start();
    bool statement_can_start();
    bool is_arith(lille_type t);
    bool is_ordered(lille_type t);
    bool assignable(id_table_node* n);
    bool compatible(lille_type target, lille_type expr);
    void check_args(id_table_node* n, vector<lille_type> args, vector<bool> bare);

    public:
    parser(scanner* s, id_table* id_t, error_handler* e); // Constructor
    void parse_program(); // Calls parser
};


#endif