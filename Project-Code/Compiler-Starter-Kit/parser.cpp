/*
 * File by: Alex Wallace
 * Course: CSC-4510 Programming Language Design and Translation
 * Date: November 2026
 *
 * parser.h — recursive-descent LL(1) parser for lille. 
 * One function per grammar nonterminal
 * FIRST and FOLLOW sets drive main design and grammar decisions
 * Each function relates to a specific syntax of Lille
 * Takes tokens from scanner without rescanning file
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

bool parser::is_arith(lille_type t)
{
    if(t.get_type() == lille_type::type_integer or t.get_type() == lille_type::type_real)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

bool parser::is_ordered(lille_type t)
{
    if(is_arith(t) or t.get_type() ==lille_type::type_string or t.get_type() == lille_type::type_boolean)
    {
        return true;
    }
    else 
    {
        return false;
    }
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
    // FIRST set

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
    ident_info name;                                            // capture BEFORE consuming
    name.name = scan->this_token()->get_identifier_value();
    name.line = scan->this_token()->get_line_number();
    name.pos  = scan->this_token()->get_pos_on_line();
    scan->must_be(symbol::identifier);

    id_tab->enter_name(name.name, lille_type(lille_type::type_prog),lille_kind(lille_kind::unknown), name.line, name.pos);

    scan->must_be(symbol::is_sym);
    parse_block();
    scan->must_be(symbol::semicolon_sym);
    if(!scan->have(symbol::end_of_program)) // Nothing after program line
    {
        error->flag(scan->this_token(), 77);
    }
}

void parser::parse_block(vector<param_info> params) // Block syntax check
{
    // <block> ::= { <declaration> }* begin <statement_list> end [ <ident> ]

    id_tab->enter_scope();      
    for (param_info param : params)  
    {
        id_tab->enter_name(param.id.name, param.type, lille_kind(param.mode), param.id.line, param.id.pos);
    }
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
    id_tab->exit_scope();
}

void parser::parse_statement_list() // Grammar check statement list
{
    // <statement_list> ::= <statement> ; { <statement> ; }*

    parse_statement();
    scan->must_be(symbol::semicolon_sym);

    while(statement_can_start()) // FOLLOW
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
        ident_info name;                                    // flavor-1 capture, single
        name.name = scan->this_token()->get_identifier_value();
        name.line = scan->this_token()->get_line_number();
        name.pos  = scan->this_token()->get_pos_on_line();
        scan->must_be(symbol::identifier);

        vector<param_info> params;
        if(scan->have(symbol::left_paren_sym))
        {
            scan->must_be(symbol::left_paren_sym);
            params = parse_param_list();
            scan->must_be(symbol::right_paren_sym);
        }
        
        id_table_node* n = id_tab->enter_name(name.name, lille_type(lille_type::type_proc), lille_kind(lille_kind::unknown), name.line, name.pos);
        if (n != nullptr)
        {
            for (param_info p : params)
            {
                n->param_types.push_back(p.type);
                n->param_modes.push_back(p.mode);
            }
        }

        scan->must_be(symbol::is_sym);
        parse_block(params);
        scan->must_be(symbol::semicolon_sym);
    }
    else if(scan->have(symbol::function_sym))
    {
        scan->must_be(symbol::function_sym);
        ident_info name;
        name.name = scan->this_token()->get_identifier_value();
        name.line = scan->this_token()->get_line_number();
        name.pos  = scan->this_token()->get_pos_on_line();
        scan->must_be(symbol::identifier);

        vector<param_info> params;
        if(scan->have(symbol::left_paren_sym))
        {
            scan->must_be(symbol::left_paren_sym);
            params = parse_param_list();
            scan->must_be(symbol::right_paren_sym);
        }

        scan->must_be(symbol::return_sym);
        lille_type ret = parse_type();

        id_table_node* n = id_tab->enter_name(name.name, lille_type::type_func, lille_kind(lille_kind::unknown), name.line, name.pos);

        if (n != nullptr)
        {
            for (param_info p : params)
            {
                n->param_types.push_back(p.type);
                n->param_modes.push_back(p.mode);
            }
            n->return_type = ret;
        }

        scan->must_be(symbol::is_sym);
        parse_block(params);
        scan->must_be(symbol::semicolon_sym);
    }
    else
    {
        int int_val;
        float real_val;
        string str_val;
        bool bool_val;
        vector<ident_info> ids = parse_ident_list();
        scan->must_be(symbol::colon_sym);
        bool is_const = scan->have(symbol::constant_sym); 
        if(is_const)
        {
            scan->get_token();
        }
        lille_type type = parse_type();
        if(scan->have(symbol::becomes_sym))
        {
            scan->must_be(symbol::becomes_sym);
            if(scan->have(symbol::integer))
            {
                int_val = scan->this_token()->get_integer_value();
                scan->get_token();
            }
            else if(scan->have(symbol::real_num))
            {
                real_val = scan->this_token()->get_real_value();
                scan->get_token();
            }
            else if(scan->have(symbol::strng))
            {
                str_val = scan->this_token()->get_string_value();
                scan->get_token();
            }
            else if(scan->have(symbol::true_sym) or scan->have(symbol::false_sym))
            {
                bool_val = scan->have(symbol::true_sym);
                scan->get_token();
            }
            else
            {
                error->flag(scan->this_token(), 3);
            }
        }
        for (ident_info id : ids)
        {
        id_table_node* n = id_tab->enter_name(id.name, type, lille_kind(is_const ? lille_kind::constant : lille_kind::variable), id.line, id.pos);
            if (n != nullptr and is_const)
            {
                if(type.is_type(lille_type::type_integer)) 
                {
                    n->int_value = int_val;
                }
                else if(type.is_type(lille_type::type_real))    
                {
                    n->real_value = real_val;
                }
                else if(type.is_type(lille_type::type_string))  
                {
                    n->string_value = str_val;
                }
                else if(type.is_type(lille_type::type_boolean)) 
                {
                    n->bool_value = bool_val;
                }
            }
        }
        scan->must_be(symbol::semicolon_sym);
    }
}



vector<ident_info> parser::parse_ident_list() // Identifier list grammar
{
    vector<ident_info> result;
    ident_info id;
    id.name = scan->this_token()->get_identifier_value();
    id.line = scan->this_token()->get_line_number();
    id.pos = scan->this_token()->get_pos_on_line();
    result.push_back(id);
    scan->must_be(symbol::identifier);
    while(scan->have(symbol::comma_sym))
    {
        scan->must_be(symbol::comma_sym);
        id.name = scan->this_token()->get_identifier_value();
        id.line = scan->this_token()->get_line_number();
        id.pos = scan->this_token()->get_pos_on_line();
        result.push_back(id);
        scan->must_be(symbol::identifier);
    }
    return result;
}

lille_type parser::parse_type() // Grammar for identifiers
{
    
    if(scan->have(symbol::integer_sym))
    {
        scan->get_token();
        return lille_type(lille_type::type_integer);
    }
    else if(scan->have(symbol::real_sym))
    {
        scan->get_token();
        return lille_type(lille_type::type_real);
    }
    else if(scan->have(symbol::string_sym))
    {
        scan->get_token();
        return lille_type(lille_type::type_string);
    }
    else if(scan->have(symbol::boolean_sym))
    {
        scan->get_token();
        return lille_type(lille_type::type_boolean);
    }
    else
    {
        scan->must_be(symbol::integer_sym);
        return lille_type(lille_type::type_unknown);
    }
}

vector<param_info> parser::parse_param_list() // Grammar for parameters

{
    vector<param_info> result;

    vector<param_info> ids = parse_param();
    for(param_info id : ids)
    {
        result.push_back(id);
    }

    while(scan->have(symbol::semicolon_sym))
    {
        scan->must_be(symbol::semicolon_sym);
        ids = parse_param();
        for(param_info id : ids)
        {
            result.push_back(id);
        }
    }
    
    return result;
}

lille_kind parser::parse_param_kind() // grammar for parameter types, fall back on last section
{
    if(scan->have(symbol::value_sym))
    {
        scan->must_be(symbol::value_sym);
        return lille_kind(lille_kind::value_param);
    }
    else if(scan->have(symbol::ref_sym))
    {
        scan->must_be(symbol::ref_sym);
        return lille_kind(lille_kind::ref_param);
    }
    else 
    {
        scan->must_be(symbol::value_sym);
        return lille_kind(lille_kind::unknown);
    }
}

vector<param_info> parser::parse_param() // Specific parameters, ident -> : -> param kind
{
    vector<ident_info> ids = parse_ident_list();
    scan->must_be(symbol::colon_sym);
    lille_kind mode = parse_param_kind();
    lille_type type = parse_type();

    vector<param_info> result;
    for(ident_info id : ids)
    {
        result.push_back({id, type, mode});
    }
    return result;
}

lille_type parser::parse_expression() // expression grammar
{
    lille_type l = parse_simp_expression(), r;

    if(relation_op_follows())
    {
        scan->get_token();
        r = parse_simp_expression();
        if(l.get_type() != lille_type::type_unknown and r.get_type() != lille_type::type_unknown)
        {
            if(!is_ordered(l) or !is_ordered(r))
            {
                error->flag(scan->this_token(), 84); 
            }
            else if(!((is_arith(l) and is_arith(r)) or (l.get_type() == r.get_type())))
            {
                error->flag(scan->this_token(), 84); 
            }
        }
        return lille_type::type_boolean;
    }
    else if(scan->have(symbol::in_sym))
    {
        scan->must_be(symbol::in_sym);
        parse_range();
        return lille_type::type_boolean;
    }
    return l;
}

lille_type parser::parse_simp_expression() // Helper for expression grammar/simple expression
{
    lille_type l = parse_expr_2(), r;

    while(scan->have(symbol::ampersand_sym))
    {
        scan->must_be(symbol::ampersand_sym);
        r = parse_expr_2();
        if(l.get_type() == lille_type::type_unknown or r.get_type() == lille_type::type_unknown)
        {
            l = lille_type::type_unknown;
        }
        else
        {
            if(l.get_type() != lille_type::type_string or r.get_type() != lille_type::type_string)
            {
                error->flag(scan->this_token(), 84); 
            }
            l = lille_type::type_string;
        }
    }
    return l;
}

lille_type parser::parse_expr_2() // Expression 2
{
    lille_type l = parse_term(), r;
    bool is_or;
    while(scan->have(symbol::plus_sym) or scan->have(symbol::minus_sym) or scan->have(symbol::or_sym))
    {
        is_or = scan->have(symbol::or_sym);
        scan->get_token();
        r = parse_term();

        if(l.get_type() == lille_type::type_unknown or r.get_type() == lille_type::type_unknown)
        {
            l = lille_type::type_unknown;
        }
        else if(is_or)
        {
            if(l.get_type() != lille_type::type_boolean or r.get_type() != lille_type::type_boolean)
            {
                error->flag(scan->this_token(), 84); 
            }
            l = lille_type::type_boolean;
        }
        else 
        {
            if(!is_arith(l) or !is_arith(r))
            {
                error->flag(scan->this_token(), 84); 
            }
            l = (l.get_type() == lille_type::type_integer and r.get_type() == lille_type::type_integer) ? lille_type::type_integer : lille_type::type_real;
        }
    }
    return l;
}

void parser::parse_range() // Range grammar
{
    lille_type l = parse_simp_expression();
    scan->must_be(symbol::range_sym); // Must have range symbol
    lille_type u = parse_simp_expression();
    if(l.get_type() != lille_type::type_unknown and l.get_type() != lille_type::type_integer)
    {
        error->flag(scan->this_token(), 84); 
    }
    if(u.get_type() != lille_type::type_unknown and u.get_type() != lille_type::type_integer)
    {
        error->flag(scan->this_token(), 84); 
    }
}

lille_type parser::parse_term() // Term grammar
{
    bool is_and;
    lille_type left = parse_factor(), right;
    while (scan->have(symbol::asterisk_sym) or scan->have(symbol::slash_sym) or scan->have(symbol::and_sym))
    {
        is_and = scan->have(symbol::and_sym);
        scan->get_token();
        right = parse_factor();

        if(left.get_type() == lille_type::type_unknown or right.get_type() == lille_type::type_unknown)
        {
            left = lille_type::type_unknown;
        }
        else if(is_and)
        {
            if(left.get_type() != lille_type::type_boolean or right.get_type() != lille_type::type_boolean)
            {
                error->flag(scan->this_token(), 84); 
            }
            left = lille_type::type_boolean;
        }
        else
        {
            if(!is_arith(left) or !is_arith(right))
            {
                error->flag(scan->this_token(), 84); 
            }
            left = (left.get_type() == lille_type::type_integer and right.get_type() == lille_type::type_integer) ? lille_type::type_integer : lille_type::type_real;
        }
    }
    return left;
}

lille_type parser::parse_factor() // Factor grammar
{
    if (scan->have(symbol::plus_sym) or scan->have(symbol::minus_sym))
    {
        scan->get_token();                  // optional leading sign
        lille_type t = parse_primary();
        if(t.get_type() != lille_type::type_unknown and !is_arith(t))
        {
            error->flag(scan->this_token(), 84); 
        }
        return t;
    }
    else
    {
        lille_type t = parse_primary();
        if (scan->have(symbol::power_sym))
        {
            scan->must_be(symbol::power_sym);
            lille_type p = parse_primary();
            if(t.get_type() == lille_type::type_unknown or p.get_type() == lille_type::type_unknown)
            {
                return lille_type::type_unknown;
            }
            if(p.get_type() != lille_type::type_integer)
            {
                error->flag(scan->this_token(), 84); 
                return lille_type::type_unknown;
            }
            if(!is_arith(t))
            {
                error->flag(scan->this_token(), 84); 
                return lille_type::type_unknown;
            }
            if(t.get_type() == lille_type::type_integer)
            {
                return lille_type::type_integer;
            }
            else
            {
                return lille_type::type_real;
            }
        }
        return t;
    }
}

lille_type parser::parse_primary() // Primary syntax in Lille using have and must_be
{
    if (scan->have(symbol::not_sym))
    { 
        scan->must_be(symbol::not_sym);  
        lille_type t = parse_expression();
        if(t.get_type() != lille_type::type_unknown and t.get_type() != lille_type::type_boolean)
        {
            error->flag(scan->this_token(), 84); 
        }
        return lille_type::type_boolean;
    }
    else if (scan->have(symbol::odd_sym)) 
    { 
        scan->must_be(symbol::odd_sym); 
        lille_type t = parse_expression();
        if(t.get_type() != lille_type::type_unknown and t.get_type() != lille_type::type_integer)
        {
            error->flag(scan->this_token(), 84); 
        }
        return lille_type::type_boolean;
    }
    else if (scan->have(symbol::left_paren_sym))
    {
        scan->must_be(symbol::left_paren_sym);
        lille_type t = parse_simp_expression();
        scan->must_be(symbol::right_paren_sym);
        return t;
    }
    else if (scan->have(symbol::identifier))
    {
        ident_info id;
        
        id.name = scan->this_token()->get_identifier_value();
        id.line = scan->this_token()->get_line_number();
        id.pos = scan->this_token()->get_pos_on_line();
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
            id_table_node* n = id_tab->find_name(id.name);
            if(n == nullptr)
            {
                error->flag(scan->this_token(), 81); 
                return lille_type(lille_type::type_unknown);
            }
            if(n->type.is_type(lille_type::type_func))
            {
                return n->return_type;
            }
            else
            {
                error->flag(scan->this_token(), 91); 
                return lille_type::type_unknown;
            }
        }
        else
        {
            id_table_node* n = id_tab->find_name(id.name);
            if(n == nullptr)
            {
                error->flag(scan->this_token(), 81); 
            }
            if(n->type.is_type(lille_type::type_proc) or n->type.is_type(lille_type::type_func))
            {
                error->flag(scan->this_token(), 91); 
            }
            return n->type;
        }
    }
    else if (scan->have(symbol::integer))  
    {
        scan->get_token();
        return lille_type::type_integer;
    }
    else if (scan->have(symbol::real_num))
    {
        scan->get_token();
        return lille_type::type_real;
    }
    else if (scan->have(symbol::strng))
    {
        scan->get_token();
        return lille_type::type_string;
    }
    else if (scan->have(symbol::true_sym) or scan->have(symbol::false_sym)) 
    {
        scan->get_token();
        return lille_type::type_boolean;
    }
    else 
    {
        scan->must_be(symbol::identifier);
        return lille_type::type_unknown;
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