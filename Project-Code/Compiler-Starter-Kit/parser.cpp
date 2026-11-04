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
};



void parser::parse_program()
{

}
void parser::parse_block()
{

}
void parser::parse_statement_list()
void parser::parse_statement()
{
    
}


