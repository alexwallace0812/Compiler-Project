/*
 * idtable.h
 *
 * Written:            1 June 2020
 * Last modified:      1 June 2023
 *      Author: Michael Oudshoorn
 */

#ifndef ID_TABLE_H_
#define ID_TABLE_H_

#include <iostream>
#include <string>

#include <lille_kind.h>
#include <lille_type.h>
#include "token.h"
#include "error_handler.h"

using namespace std;

class id_table {
private:
	error_handler* error;
    vector<id_table_node*> scopes;

public:
    id_table(error_handler* err);
    
    void dump_id_table(bool dump_all = true);
    void enter_scope();
    void exit_scope();
    id_table_node* enter_name(string name, lille_type t, lille_kind k, int line, int pos);
    id_table_node* find_name(string name);
    void dump_id_table();
};


class id_table_node 
{
    private:
    string name;
    lille_type type;
    lille_kind kind;
    int level;
    int line_number, pos_in_line;

    int int_value;
    float real_value;
    string string_value;
    bool bool_value;

    vector<lille_type> param_types;
    vector<lille_kind> param_modes;
    lille_type return_type;
    bool traced;

    id_table_node *left, *right;

    id_table_node(string n, lille_type t, lille_kind k, int level, int line, int pos);

};

#endif /* ID_TABLE_H_ */
