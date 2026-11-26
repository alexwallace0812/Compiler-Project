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
	enter_predefined("INT2REAL",  lille_type::type_integer, lille_type::type_real);
	enter_predefined("REAL2INT",  lille_type::type_real,    lille_type::type_integer);
	enter_predefined("INT2STRING", lille_type::type_integer, lille_type::type_string);
	enter_predefined("REAL2STRING", lille_type::type_real,  lille_type::type_string);
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
	trace_set_by_pragma = false;
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
		cout << "Dump of idtxable for current scope only." << endl;
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
	cout << "Token Name: " << setw(15) << left << node->name
     << " Kind: "<< setw(12) << node->kind.to_string()
     << " Type: " << setw(10) << node->type.to_string()
     << " Level: " << node->lev
     << " Line Number: " << node->line_number << ", Position: " << node->pos_in_line;

if (node->kind.is_kind(lille_kind::constant))          // value, if it's a constant
{
    if      (node->type.is_type(lille_type::type_integer)) cout << "  Value: " << node->int_value;
    else if (node->type.is_type(lille_type::type_real))    cout << "  Value: " << node->real_value;
    else if (node->type.is_type(lille_type::type_string))  cout << "  Value: " << node->string_value;
    else if (node->type.is_type(lille_type::type_boolean)) cout << "  Value: " << node->bool_value;
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

	id_table_node* id_table::enter_predefined(string name, lille_type param, lille_type ret)
	{
		id_table_node* n = enter_name(name, lille_type(lille_type::type_func), lille_kind(lille_kind::unknown), 0, 0);
		if (n != nullptr)
		{
			n->param_types.push_back(param);
			n->param_modes.push_back(lille_kind(lille_kind::value_param));
			n->return_type = ret;
		}
		return n;
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

	void id_table::set_all_traced(bool on)
	{
		for (id_table_node* root : scopes)
			flag_all(root, on);
	}

	void id_table::flag_all(id_table_node* node, bool on)
	{
		if(node == nullptr) 
		{
			return;
		}
		if(on)  
		{
			node->traced = true;
		}
		else     
		{
			node->traced = node->trace_set_by_pragma; 
		}
		flag_all(node->left, on);
		flag_all(node->right, on);
	}

