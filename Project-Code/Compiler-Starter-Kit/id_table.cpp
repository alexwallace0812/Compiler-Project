/*
 * idtable.cpp
 *
 * Written:            1 June 2020
 * Last modified:      1 June 2023
 *      Author: Michael Oudshoorn
 */



#include <iostream>
#include <string>
#include <iomanip>

#include "token.h"
#include "error_handler.h"
#include "id_table.h"

using namespace std;


id_table::id_table(error_handler* err)
{
    error = err;
    scopes.push_back(nullptr);
}

id_table_node::id_table_node(string n, lille_type t, lille_kind k, int level, int line, int pos)
{
	name = n;
	type = t;
	kind = k;
	lev = level;
	line_number = line;
	pos_in_line = pos;
	int_value = 0;
	real_value = 0;
	string_value = "";
	bool_value = false;
	traced = false;
	left = nullptr;
	right = nullptr;
}

id_table_node* id_table::bst_search(id_table_node* node, string name)
{
	if(node == nullptr)
	{
		return nullptr;
	}
	if(name == node->name)
	{
		return node;
	}
	if(name < node->name)
	{
		return bst_search(node->left, name);
	}
	else
	{
		return bst_search(node->right, name);
	}
}

id_table_node* id_table::bst_insert(id_table_node* node, id_table_node* new_id)
{
	if (node == nullptr) return new_id;              // empty slot: the new node becomes the subtree
    if (new_id->name < node->name)
        node->left = bst_insert(node->left, new_id); // reattach whatever comes back
    else
        node->right = bst_insert(node->right, new_id);
    return node;  
}

void id_table::destroy_tree(id_table_node* node)
{
	if(node == nullptr)
	{
		return;
	}
	destroy_tree(node->left);
	destroy_tree(node->right);
	delete node;
}


void id_table::dump_id_table(bool dump_all)
{
	if (!dump_all)
	{
		cout << "Dump of idtable for current scope only." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        int lev = scopes.size() - 1;
        cout << "Scope level " << lev << endl << "-------------" << endl;
        inorder_print(scopes[lev]);
	}
	else
	{
		cout << "Dump of the entire symbol table." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
        for (int lev = scopes.size() - 1; lev >= 0; lev--)
        {
            cout << "Scope level " << lev << endl << "-------------" << endl;
            inorder_print(scopes[lev]);
        }
	}
}

void id_table::inorder_print(id_table_node* node)
{
	    if (node == nullptr) return;
    inorder_print(node->left);
	cout << setw(15) << left << node->name
     << setw(12) << node->kind.to_string()
     << setw(10) << node->type.to_string()
     << " level " << node->lev
     << " (" << node->line_number << "," << node->pos_in_line << ")";

if (node->kind.is_kind(lille_kind::constant))          // value, if it's a constant
{
    if      (node->type.is_type(lille_type::type_integer)) cout << "  value: " << node->int_value;
    else if (node->type.is_type(lille_type::type_real))    cout << "  value: " << node->real_value;
    else if (node->type.is_type(lille_type::type_string))  cout << "  value: " << node->string_value;
    else if (node->type.is_type(lille_type::type_boolean)) cout << "  value: " << node->bool_value;
}

if (node->type.is_type(lille_type::type_proc) or node->type.is_type(lille_type::type_func))
{
    cout << "  params: (";
    for (int i = 0; i < node->param_types.size(); i++)
        cout << node->param_modes[i].to_string() << " " << node->param_types[i].to_string()
             << (i < node->param_types.size() - 1 ? ", " : "");
    cout << ")";
    if (node->type.is_type(lille_type::type_func))
        cout << "  returns " << node->return_type.to_string();
}
cout << endl;
    inorder_print(node->right);   
}


    void id_table::enter_scope()
	{
		scopes.push_back(nullptr);
	}



    void id_table::exit_scope()
	{
		dump_id_table(true);
		destroy_tree(scopes.back());
		scopes.pop_back();
	}

	id_table_node* id_table::enter_name(string name, lille_type t, lille_kind k, int line, int pos)
	{
		if(bst_search(scopes.back(), name) != nullptr)
		{
			error->flag(line, pos, 82);
		}

		id_table_node* new_id = new id_table_node(name, t, k, scopes.size() - 1, line, pos);
		scopes.back() = bst_insert(scopes.back(), new_id);
    	return new_id;
	}
	
	id_table_node* id_table::find_name(string name)
	{
		for(int lev = scopes.size() - 1; lev >= 0; lev--)
		{
			id_table_node* n = bst_search(scopes[lev], name);
			if (n != nullptr) return n;
		}
		return nullptr;
	}

