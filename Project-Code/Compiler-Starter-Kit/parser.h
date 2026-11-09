/*
 * parser.h
 *
 * Written:            4 November 2026
 * Last modified:      4 November 2026
 *      Author: Alex Wallace
 */



#ifndef PARSER_H
#define PARSER_H


#include <iostream>
#include <fstream>
#include <string>

#include "symbol.h"
#include "token.h"
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