/*
 * idtable.cpp
 *
 * Written:            1 June 2020
 * Last modified:      1 June 2023
 *      Author: Michael Oudshoorn
 */



#include <iostream>
#include <string>

#include "token.h"
#include "error_handler.h"
#include "id_table.h"

using namespace std;


id_table::id_table(error_handler* err)
{
    error = err;
    scopes.push_back(nullptr);
}


void id_table::dump_id_table(bool dump_all)
{
	if (!dump_all)
	{
		cout << "Dump of idtable for current scope only." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		
        // INSERT CODE HERE
	}
	else
	{
		cout << "Dump of the entire symbol table." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		
        // INSERT CODE HERE
	}
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
