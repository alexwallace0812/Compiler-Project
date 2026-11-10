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


class parser {
    private:

    scanner* scan;
    id_table* id_tab;
    error_handler* error;

    // Main parser grammar function declarations

    void parse_block();
    void parse_statement_list();
    void parse_statement();
    void parse_declaration();
    void parse_ident_list();
    void parse_type();
    void parse_param_list();
    void parse_param();
    void parse_param_kind();
    void parse_expression();
    void parse_simp_expression();
    void parse_range();
    void parse_expr_2();
    void parse_term();
    void parse_factor();
    void parse_primary();
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

    public:
    parser(scanner* s, id_table* id_t, error_handler* e); // Constructor
    void parse_program(); // Calls parser
};


#endif