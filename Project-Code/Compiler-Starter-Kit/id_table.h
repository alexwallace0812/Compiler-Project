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

#include "lille_kind.h"
#include "lille_type.h"
#include "token.h"
#include "error_handler.h"

using namespace std;

class id_table_node 
{
    public:
    string name;
    lille_type type;
    lille_kind kind;
    int lev;
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

class id_table {
private:
	error_handler* error;
    vector<id_table_node*> scopes;

    void destroy_tree(id_table_node* node);
    id_table_node* bst_search(id_table_node* node, string name);
    id_table_node* bst_insert(id_table_node* node, id_table_node* new_id);
    void inorder_print(id_table_node* node);

public:
    id_table(error_handler* err);
    void enter_scope();
    void exit_scope();
    id_table_node* enter_name(string name, lille_type t, lille_kind k, int line, int pos);
    id_table_node* find_name(string name);
    void dump_id_table(bool dump_all = true);
};

#endif /* ID_TABLE_H_ */
