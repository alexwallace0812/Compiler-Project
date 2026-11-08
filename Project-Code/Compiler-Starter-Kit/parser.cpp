/*
 * parser.cpp
 *
 * Written:            4 November 2026
 * Last modified:      4 November 2026
 *      Author: Alex Wallace
 */

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cctype>
#include <cmath>
#include <climits>

#include "symbol.h"
#include "parser.h"
#include "error_handler.h"
#include "token.h"
#include "scanner.h"
#include "lille_exception.h"




parser::parser(scanner* s, id_table* id_t, error_handler* e)
{
    scan = s;
    id_tab = id_t;
    error = e;
    scan->get_token();
}



void parser::parse_program()
{
    scan->must_be(symbol::program_sym);
    scan->must_be(symbol::identifier);
    scan->must_be(symbol::is_sym);
    parse_block();
    scan->must_be(symbol::semicolon_sym);
    if(!scan->have(symbol::end_of_program))
    {
        error->flag(scan->this_token(), 77);
    }
}
void parser::parse_block()
{
    scan->must_be(symbol::begin_sym);
    parse_statement_list();
    scan->must_be(symbol::end_sym);

    if (scan->have(symbol::identifier))
        scan->get_token(); 
}
void parser::parse_statement_list()
{
    parse_statement();
    scan->must_be(symbol::semicolon_sym);

    while(scan->have(symbol::null_sym))
    {
        parse_statement();
        scan->must_be(symbol::semicolon_sym);
    }
}
void parser::parse_statement()
{
    scan->must_be(symbol::null_sym);
}


