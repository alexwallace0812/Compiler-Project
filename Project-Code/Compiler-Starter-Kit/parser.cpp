/*
 * parser.cpp
 *
 * Written:            4 November 2026
 * Last modified:      9 November 2026
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
    scan = s; // Assign current scanner state to parser
    id_tab = id_t;
    error = e;
    scan->get_token(); // Parser usess input scanner to get the token at each scan.
}


bool parser::relation_op_follows() // Help boolean functions, how to recognize optional components/relational operators
{
    symbol::symbol_type s = scan->this_token()->get_sym();
        return s == symbol::greater_than_sym or s == symbol::less_than_sym or s == symbol::equals_sym or s == symbol::not_equals_sym or s == symbol::less_or_equal_sym or s == symbol::greater_or_equal_sym;
}

bool parser::expression_can_start()
{
    symbol::symbol_type s = scan->this_token()->get_sym();
    return s == symbol::identifier or s == symbol::integer or s == symbol::real_num or s == symbol::strng or s == symbol::true_sym or s == symbol::false_sym or s == symbol::not_sym or s == symbol::odd_sym or s == symbol::left_paren_sym or s == symbol::plus_sym or s == symbol::minus_sym;
}


bool parser::statement_can_start()
{
    symbol::symbol_type s = scan->this_token()->get_sym();
    return s == symbol::null_sym or s == symbol::write_sym or s == symbol::writeln_sym or s == symbol::identifier or s == symbol::exit_sym or s == symbol::return_sym or s == symbol::read_sym or s == symbol::read_sym or s == symbol::if_sym or s == symbol::loop_sym or s == symbol::while_sym or s == symbol::for_sym;
}



void parser::parse_statement() // Dispatcher, calls specific parsing grammar checks
{

    if (scan->have(symbol::write_sym))
    {
        parse_write(); // Parses write function
    }
    else if (scan->have(symbol::writeln_sym))
    {
        parse_write_line(); // Parses write_line function
    }
    else if (scan->have(symbol::identifier))
    {
        parse_simp_statement(); // Parses simple statement function
    }
    else if (scan->have(symbol::exit_sym))
    {
        parse_exit(); // Parses exit
    }
    else if (scan->have(symbol::return_sym))
    {
        parse_return(); // Parses return
    }
    else if (scan->have(symbol::read_sym))
    {
        parse_read(); // Parses read function
    }
    else if (scan->have(symbol::if_sym))
    {
        parse_if(); // Parses if
    }
    else if (scan->have(symbol::loop_sym))
    {
        parse_loop(); // Parses loop
    }
    else if (scan->have(symbol::while_sym))
    {
        parse_while(); // Parses while
    }
    else if (scan->have(symbol::for_sym))
    {
        parse_for(); // Parses for loop
    }
    else
    {
        scan->must_be(symbol::null_sym); // Dispatcher falls back on null sym
    }
}

void parser::parse_program() // Grammar check for opening program syntax
{
    scan->must_be(symbol::program_sym); // <prog> ::= program <ident> is <block> 
    scan->must_be(symbol::identifier);
    scan->must_be(symbol::is_sym);
    parse_block();
    scan->must_be(symbol::semicolon_sym);
    if(!scan->have(symbol::end_of_program)) // Nothing after program line
    {
        error->flag(scan->this_token(), 77);
    }
}

void parser::parse_block() // Block syntax check
{
    // <block> ::= { <declaration> }* begin <statement_list> end [ <ident> ]
    
    while(scan->have(symbol::identifier) or scan->have(symbol::procedure_sym) or scan->have(symbol::function_sym))
    {
        parse_declaration();
    }
    scan->must_be(symbol::begin_sym);
    parse_statement_list();
    scan->must_be(symbol::end_sym);

    if (scan->have(symbol::identifier))
    {
        scan->get_token(); 
    }
}

void parser::parse_statement_list() // Grammar check statement list
{
    // <statement_list> ::= <statement> ; { <statement> ; }*

    parse_statement();
    scan->must_be(symbol::semicolon_sym);

    while(statement_can_start())
    {
        parse_statement();
        scan->must_be(symbol::semicolon_sym);
    }
}



void parser::parse_declaration() // Grammar check for declarations, three branches: procedure, function, or ident list as fall back
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



void parser::parse_ident_list() // Identifier list grammar
{
    scan->must_be(symbol::identifier);
    while(scan->have(symbol::comma_sym))
    {
        scan->must_be(symbol::comma_sym);
        scan->must_be(symbol::identifier);
    }
}

void parser::parse_type() // Grammar for identifiers
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

void parser::parse_param_list() // Grammar for parameters

{
    parse_param();
    while(scan->have(symbol::semicolon_sym))
    {
        scan->must_be(symbol::semicolon_sym);
        parse_param();
    }
}

void parser::parse_param_kind() // grammar for parameter types, fall back on last section
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

void parser::parse_param() // Specific parameters, ident -> : -> param kind
{
    parse_ident_list();
    scan->must_be(symbol::colon_sym);
    parse_param_kind();
    parse_type();
}

void parser::parse_expression() // expression grammar
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

void parser::parse_simp_expression() // Helper for expression grammar/simple expression
{
    parse_expr_2();

    while(scan->have(symbol::ampersand_sym))
    {
        scan->must_be(symbol::ampersand_sym);
        parse_expr_2();
    }
}

void parser::parse_expr_2() // Expression 2
{
    parse_term();
    
    while(scan->have(symbol::plus_sym) or scan->have(symbol::minus_sym) or scan->have(symbol::or_sym))
    {
        scan->get_token();
        parse_term();
    }
}

void parser::parse_range() // Range grammar
{
    parse_simp_expression();
    scan->must_be(symbol::range_sym); // Must have range symbol
    parse_simp_expression();
}

void parser::parse_term() // Term grammar
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

void parser::parse_factor() // Factor grammar
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



void parser::parse_write() // Write function grammar, allows for optional parentheses which is why bool flag is used.
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

void parser::parse_write_line() // Same as write but the middle expression is optional, expr can start helper
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

void parser::parse_simp_statement() // Simple statement grammar
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

void parser::parse_exit() // Exit grammar
{
    scan->must_be(symbol::exit_sym);
    if (scan->have(symbol::when_sym)) 
    { 
        scan->must_be(symbol::when_sym); 
        parse_expression(); 
    }
}

void parser::parse_return() // Return grammar
{
    scan->must_be(symbol::return_sym);
    if (expression_can_start())
    {
        parse_expression();
    }
}

void parser::parse_read() // Read
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

void parser::parse_if() // if grammar, in depth the while loop shows elsif repetition
// Falls back on else
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

void parser::parse_loop() // Loop grammar, second loop symbol declares end
{
    scan->must_be(symbol::loop_sym);
    parse_statement_list();
    scan->must_be(symbol::end_sym);
    scan->must_be(symbol::loop_sym);
}

void parser::parse_while() // While grammar
{
    scan->must_be(symbol::while_sym);
    parse_expression();
    parse_loop();
}

void parser::parse_for() // For grammar, includes range and loop functionality
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