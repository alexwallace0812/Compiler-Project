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

    void parse_block();
    void parse_statement_list();
    void parse_statement();


    public:
    parser(scanner* s, id_table* id_t, error_handler* e);
    void parse_program();



};


#endif