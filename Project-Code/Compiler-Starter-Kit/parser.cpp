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
    while(scan->have(symbol::identifier) or scan->have(symbol::procedure_sym) or scan->have(symbol::function_sym))
    {
        parse_declaration();
    }

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


void parser::parse_declaration()
{
    if(scan->have(symbol::procedure_sym))
    {
        scan->must_be(symbol::procedure_sym);
        scan->must_be(symbol::identifier);
        if(scan->have(symbol::left_paren_sym))
        {
            scan->must_be(symbol::left_paren_sym);
            parse_param_list();
            scan->must_be(symbol::right_paren_sym);
        }
        scan->must_be(symbol::is_sym);
        parse_block();
        scan->must_be(symbol::semicolon_sym);
    }
    else if(scan->have(symbol::function_sym))
    {
        scan->must_be(symbol::function_sym);
        scan->must_be(symbol::identifier);
        if(scan->have(symbol::left_paren_sym))
        {
            scan->must_be(symbol::left_paren_sym);
            parse_param_list();
            scan->must_be(symbol::right_paren_sym);
        }
        scan->must_be(symbol::return_sym);
        parse_type();
        scan->must_be(symbol::is_sym);
        parse_block();
        scan->must_be(symbol::semicolon_sym);
    }
    else
    {
        parse_ident_list();
        scan->must_be(symbol::colon_sym);
        if(scan->have(symbol::constant_sym))
        {
            scan->get_token();
        }
        parse_type();
        if(scan->have(symbol::becomes_sym))
        {
            scan->must_be(symbol::becomes_sym);
            if(scan->have(symbol::integer) or scan->have(symbol::real_num) or 
            scan->have(symbol::strng) or scan->have(symbol::true_sym) or scan->have(symbol::false_sym))
            {
                scan->get_token();
            }
            else
            {
                error->flag(scan->this_token(), 3);
            }
        }
        scan->must_be(symbol::semicolon_sym);
    }
}

void parser::parse_ident_list()
{
    scan->must_be(symbol::identifier);
    while(scan->have(symbol::comma_sym))
    {
        scan->must_be(symbol::comma_sym);
        scan->must_be(symbol::identifier);
    }
}

void parser::parse_type()
{
    
    if(scan->have(symbol::integer_sym))
    {
        scan->get_token();
    }
    else if(scan->have(symbol::real_sym))
    {
        scan->get_token();
    }
    else if(scan->have(symbol::string_sym))
    {
        scan->get_token();
    }
    else if(scan->have(symbol::boolean_sym))
    {
        scan->get_token();
    }
    else
    {
        scan->must_be(symbol::integer_sym);
    }
}

void parser::parse_param_list()
{
    parse_param();
    while(scan->have(symbol::semicolon_sym))
    {
        scan->must_be(symbol::semicolon_sym);
        parse_param();
    }
}

void parser::parse_param_kind()
{
    if(scan->have(symbol::value_sym))
    {
        scan->must_be(symbol::value_sym);
    }
    else if(scan->have(symbol::ref_sym))
    {
        scan->must_be(symbol::ref_sym);
    }
    else 
    {
        scan->must_be(symbol::value_sym);
    }
}

void parser::parse_param()
{
    parse_ident_list();
    scan->must_be(symbol::colon_sym);
    parse_param_kind();
    parse_type();
}

void parser::parse_expression()
{
    parse_simp_expression();
    if(relop_follows())
    {
        scan->get_token();
        parse_simp_expression();
    }
    else if(scan->have(symbol::in_sym))
    {
        scan->must_be(symbol::in_sym);
        parse_range();
    }
}

void parser::parse_simp_expression()
{
    parse_expr_2();

    while(scan->have(symbol::ampersand_sym))
    {
        scan->must_be(symbol::ampersand_sym);
        parse_expr_2();
    }
}

void parser::parse_expr_2()
{
    parse_term();
    
    if(scan->have(symbol::plus_sym) or scan->have(symbol::minus_sym) or scan->have(symbol::or_sym))
    {
        scan->get_token();
        parse_term();
    }
}

void parser::parse_range()
{
    parse_simp_expression();
    scan->must_be(symbol::range_sym);
    parse_simp_expression();
}

void parser::parse_term()
{

}

void parser::parse_factor()
{

}

void parser::parse_primary()
{

}

bool parser::relop_follows()
{

}
