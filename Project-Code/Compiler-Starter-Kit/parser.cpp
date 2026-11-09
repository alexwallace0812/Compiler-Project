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

    while(statement_can_start())
    {
        parse_statement();
        scan->must_be(symbol::semicolon_sym);
    }
}
void parser::parse_statement()
{

    if (scan->have(symbol::write_sym))
    {
        parse_write();
    }
    else if (scan->have(symbol::writeln_sym))
    {
        parse_write_line();
    }
    else if (scan->have(symbol::identifier))
    {
        parse_simp_statement();
    }
    else if (scan->have(symbol::exit_sym))
    {
        parse_exit();
    }
    else if (scan->have(symbol::return_sym))
    {
        parse_return();
    }
    else if (scan->have(symbol::read_sym))
    {
        parse_read();
    }
    else if (scan->have(symbol::if_sym))
    {
        parse_if();
    }
    else if (scan->have(symbol::loop_sym))
    {
        parse_loop();
    }
    else if (scan->have(symbol::while_sym))
    {
        parse_while();
    }
    else if (scan->have(symbol::for_sym))
    {
        parse_for();
    }
    else
    {
        scan->must_be(symbol::null_sym);
    }
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
    if(relation_op_follows())
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
    
    while(scan->have(symbol::plus_sym) or scan->have(symbol::minus_sym) or scan->have(symbol::or_sym))
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
    parse_factor();
    while (scan->have(symbol::asterisk_sym) 
    or scan->have(symbol::slash_sym) 
    or scan->have(symbol::and_sym))
    {
        scan->get_token();
        parse_factor();
    }
}

void parser::parse_factor()
{
    if (scan->have(symbol::plus_sym) or scan->have(symbol::minus_sym))
    {
        scan->get_token();                  // optional leading sign
        parse_primary();
    }
    else
    {
        parse_primary();
        if (scan->have(symbol::power_sym))
        {
            scan->must_be(symbol::power_sym);
            parse_primary();
        }
    }
}

void parser::parse_primary()
{
    if (scan->have(symbol::not_sym))
    { 
        scan->must_be(symbol::not_sym);  
        parse_expression();
    }
    else if (scan->have(symbol::odd_sym)) 
    { 
        scan->must_be(symbol::odd_sym); 
        parse_expression(); 
    }
    else if (scan->have(symbol::left_paren_sym))
    {
        scan->must_be(symbol::left_paren_sym);
        parse_simp_expression();
        scan->must_be(symbol::right_paren_sym);
    }
    else if (scan->have(symbol::identifier))
    {
        scan->must_be(symbol::identifier);
        if (scan->have(symbol::left_paren_sym))
        {
            scan->must_be(symbol::left_paren_sym);
            parse_expression();
            while (scan->have(symbol::comma_sym)) 
            { 
                scan->must_be(symbol::comma_sym); 
                parse_expression(); 
            }
            scan->must_be(symbol::right_paren_sym);
        }
    }
    else if (scan->have(symbol::integer) or scan->have(symbol::real_num))  
    {
        scan->get_token();
    }
    else if (scan->have(symbol::strng))
    {
        scan->get_token();
    }
    else if (scan->have(symbol::true_sym) or scan->have(symbol::false_sym)) 
    {
        scan->get_token();
    }
    else 
    {
        scan->must_be(symbol::identifier);
    } 
}

bool parser::relation_op_follows()
{
    symbol::symbol_type s = scan->this_token()->get_sym();
        return s == symbol::greater_than_sym or s == symbol::less_than_sym or s == symbol::equals_sym or s == symbol::not_equals_sym or s == symbol::less_or_equal_sym or s == symbol::greater_or_equal_sym;
}

bool parser::expression_can_start()
{
    symbol::symbol_type s = scan->this_token()->get_sym();
    return s == symbol::identifier or s == symbol::integer or s == symbol::real_num or s == symbol::strng or s == symbol::true_sym or s == symbol::false_sym or s == symbol::not_sym or s == symbol::odd_sym or s == symbol::left_paren_sym or s == symbol::plus_sym or s == symbol::minus_sym;
}

void parser::parse_write()
{
    scan->must_be(symbol::write_sym);

    bool parentheses = scan->have(symbol::left_paren_sym);
    if (parentheses) 
    {
        scan->must_be(symbol::left_paren_sym);
    }

    parse_expression();
    while (scan->have(symbol::comma_sym)) 
    { 
        scan->must_be(symbol::comma_sym); parse_expression(); 
    }

    if (parentheses) 
    {
        scan->must_be(symbol::right_paren_sym);
    }
}

void parser::parse_write_line()
{
    scan->must_be(symbol::writeln_sym);

    bool parentheses = scan->have(symbol::left_paren_sym);
    if (parentheses) 
    {
        scan->must_be(symbol::left_paren_sym);
    }

    if(expression_can_start())
    {
        parse_expression();
        while (scan->have(symbol::comma_sym)) 
        { 
            scan->must_be(symbol::comma_sym); parse_expression(); 
        }
    }

    if (parentheses) 
    {
        scan->must_be(symbol::right_paren_sym);
    }
}

void parser::parse_simp_statement()
{
    scan->must_be(symbol::identifier);

    if(scan->have(symbol::becomes_sym))
    {
        scan->must_be(symbol::becomes_sym);
        parse_expression();
    }
    else if (scan->have(symbol::left_paren_sym))
    {
        scan->must_be(symbol::left_paren_sym);
        parse_expression();

        while (scan->have(symbol::comma_sym)) 
        { 
            scan->must_be(symbol::comma_sym); 
            parse_expression(); 
        }

        scan->must_be(symbol::right_paren_sym);
    }
}

void parser::parse_exit()
{
    scan->must_be(symbol::exit_sym);
    if (scan->have(symbol::when_sym)) 
    { 
        scan->must_be(symbol::when_sym); 
        parse_expression(); 
    }
}

void parser::parse_return()
{
    scan->must_be(symbol::return_sym);
    if (expression_can_start())
    {
        parse_expression();
    }
}

void parser::parse_read()
{
    scan->must_be(symbol::read_sym);
    bool parentheses = scan->have(symbol::left_paren_sym);
    if (parentheses) 
    {
        scan->must_be(symbol::left_paren_sym);
    }
    scan->must_be(symbol::identifier);
    while (scan->have(symbol::comma_sym)) 
    { 
        scan->must_be(symbol::comma_sym); 
        scan->must_be(symbol::identifier); 
    }
    if (parentheses) 
    {
        scan->must_be(symbol::right_paren_sym);
    }
}

bool parser::statement_can_start()
{
    symbol::symbol_type s = scan->this_token()->get_sym();
    return s == symbol::null_sym or s == symbol::write_sym or s == symbol::writeln_sym or s == symbol::identifier or s == symbol::exit_sym or s == symbol::return_sym or s == symbol::read_sym or s == symbol::read_sym or s == symbol::if_sym or s == symbol::loop_sym or s == symbol::while_sym or s == symbol::for_sym;
}

void parser::parse_if()
{
    scan->must_be(symbol::if_sym);
    parse_expression();
    scan->must_be(symbol::then_sym);
    parse_statement_list();

    while (scan->have(symbol::elsif_sym))
    {
        scan->must_be(symbol::elsif_sym);
        parse_expression();
        scan->must_be(symbol::then_sym);
        parse_statement_list();
    }

    if (scan->have(symbol::else_sym))
    {
        scan->must_be(symbol::else_sym);
        parse_statement_list();
    }

    scan->must_be(symbol::end_sym);
    scan->must_be(symbol::if_sym);
}

void parser::parse_loop()
{
    scan->must_be(symbol::loop_sym);
    parse_statement_list();
    scan->must_be(symbol::end_sym);
    scan->must_be(symbol::loop_sym);                   // "end loop"
}

void parser::parse_while()
{
    scan->must_be(symbol::while_sym);
    parse_expression();
    parse_loop();
}

void parser::parse_for()
{
    scan->must_be(symbol::for_sym);
    scan->must_be(symbol::identifier);
    scan->must_be(symbol::in_sym);

    if (scan->have(symbol::reverse_sym)) 
    {
        scan->must_be(symbol::reverse_sym);
    }

    parse_range();
    parse_loop();
}