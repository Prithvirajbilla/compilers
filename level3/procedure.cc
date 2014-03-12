
/*********************************************************************************************

                                cfglp : A CFG Language Processor
                                --------------------------------

           About:

           Implemented   by  Tanu  Kanvar (tanu@cse.iitb.ac.in) and Uday
           Khedker    (http://www.cse.iitb.ac.in/~uday)  for the courses
           cs302+cs306: Language  Processors  (theory and  lab)  at  IIT
           Bombay.

           Release  date  Jan  15, 2013.  Copyrights  reserved  by  Uday
           Khedker. This  implemenation  has been made  available purely
           for academic purposes without any warranty of any kind.

           Documentation (functionality, manual, and design) and related
           tools are  available at http://www.cse.iitb.ac.in/~uday/cfglp


***********************************************************************************************/

#include<string>
#include<fstream>
#include<iostream>

using namespace std;

#include"error-display.hh"
#include"local-environment.hh"

#include"symbol-table.hh"
#include"ast.hh"
#include"basic-block.hh"
#include"procedure.hh"
#include"program.hh"

Procedure::Procedure(Data_Type proc_return_type, string proc_name)
{
	return_type = proc_return_type;
	name = proc_name;
}
Procedure::Procedure(Data_Type proc_return_type, string proc_name,list<Symbol_Table_Entry *> symbl_entry)
{
	return_type = proc_return_type;
	name = proc_name;
	
}

Procedure::~Procedure()
{
	list<Basic_Block *>::iterator i;
	for (i = basic_block_list.begin(); i != basic_block_list.end(); i++)
		delete (*i);
}

string Procedure::get_proc_name()
{
	return name;
}

void Procedure::set_return_type(Data_Type ret)
{
	return_type = ret;
}
void Procedure::set_basic_block_list(list<Basic_Block *> & bb_list)
{
	basic_block_list = bb_list;
}

void Procedure::add_arg(Symbol_Table_Entry & s)
{
	arg_list.push_back(&s);
}
void Procedure::set_argument_list(list<Symbol_Table_Entry *> arg)
{
	arg_list = arg;
}
void Procedure::fill_argument_list()
{
	list<Symbol_Table_Entry *>::iterator i;
	for (i = arg_list.begin(); i != arg_list.end(); i++)
	{
		local_symbol_table.push_symbol(*i);
	}

}
void Procedure::set_local_list(Symbol_Table & new_list)
{
	if(new_list.get_variable_table().size() == 0)
		return ;
		
	list<Symbol_Table_Entry *>::iterator i;
	list<Symbol_Table_Entry *> l = new_list.get_variable_table();
	for (i = l.begin(); i != l.end(); i++)
	{
		local_symbol_table.push_symbol(*i);
	}
	local_symbol_table.set_table_scope(local);
}

Data_Type Procedure::get_return_type()
{
	return return_type;
}

bool Procedure::variable_in_symbol_list_check(string variable)
{
	return local_symbol_table.variable_in_symbol_list_check(variable);
}

Symbol_Table_Entry & Procedure::get_symbol_table_entry(string variable_name)
{
	return local_symbol_table.get_symbol_table_entry(variable_name);
}

void Procedure::print_ast(ostream & file_buffer)
{
	file_buffer << PROC_SPACE << "Procedure: "<<name<< "\n\n";

	list<Basic_Block *>::iterator i;
	for(i = basic_block_list.begin(); i != basic_block_list.end(); i++)
		(*i)->print_bb(file_buffer);
	file_buffer<<"\n";
}
	
Basic_Block & Procedure::get_start_basic_block()
{
	list<Basic_Block *>::iterator i;
	i = basic_block_list.begin();
	return **i;
}

Basic_Block * Procedure::get_next_bb(Basic_Block & current_bb)
{
	bool flag = false;

	list<Basic_Block *>::iterator i;
	for(i = basic_block_list.begin(); i != basic_block_list.end(); i++)
	{
		if((*i)->get_bb_number() == current_bb.get_bb_number())
		{
			flag = true;
			continue;
		}
		if (flag)
			return (*i);
	}
	
	return NULL;
}

Basic_Block* Procedure::get_bb_at(Basic_Block & current_bb,int ir)
{
	bool flag = false;
	list<Basic_Block *>::iterator i;
	for(i = basic_block_list.begin(); i != basic_block_list.end(); i++)
	{
		if((*i)->get_bb_number() == ir-1)
		{
			flag = true;
			continue;
		}
		if (flag)
			return (*i);
	}
	return NULL;
}
Eval_Result & Procedure::evaluate(ostream & file_buffer)
{
	Local_Environment & eval_env = *new Local_Environment();
	local_symbol_table.create(eval_env);
	
	Eval_Result * result = NULL;

	file_buffer << PROC_SPACE << "Evaluating Procedure " << name << "\n";
	file_buffer << LOC_VAR_SPACE << "Local Variables (before evaluating):\n";
	eval_env.print(file_buffer);
	file_buffer << "\n";
	
	Basic_Block * current_bb = &(get_start_basic_block());
	while (current_bb)
	{
		result = &(current_bb->evaluate(eval_env, file_buffer));
		if(result->get_result_enum() == 2)
			current_bb = get_bb_at(*current_bb,result->get_value());
		else
			current_bb = get_next_bb(*current_bb);
				
	}

	file_buffer << "\n\n";
	file_buffer << LOC_VAR_SPACE << "Local Variables (after evaluating):\n";
	eval_env.print(file_buffer);

	return *result;
}



Eval_Result & Procedure::evaluate(list<Eval_Result *> l,ostream & file_buffer)
{
	Local_Environment & eval_env = *new Local_Environment();
	local_symbol_table.create(eval_env);
	list<Eval_Result *>:: iterator it ;
	list<Symbol_Table_Entry *>:: iterator i=arg_list.begin();
	for(it = l.begin();it!=l.end();it++)
	{
		Eval_Result * e = *it;
		Eval_Result_Value * k;
		if(e->get_result_enum() == int_result)
		{
			k = new Eval_Result_Value_Int();
			k->set_value(e->get_value());
		}
		else if(e->get_result_enum() == float_result)
		{
			k = new Eval_Result_Value_Float();
			k->set_value(e->get_value());
		}
		eval_env.put_variable_value(*k,(*i)->get_variable_name());
		i++;
	}

	Eval_Result * result = NULL;

	file_buffer << PROC_SPACE << "Evaluating Procedure " << name << "\n";
	file_buffer << LOC_VAR_SPACE << "Local Variables (before evaluating):\n";
	eval_env.print(file_buffer);
	file_buffer << "\n";
	
	Basic_Block * current_bb = &(get_start_basic_block());
	while (current_bb)
	{
		result = &(current_bb->evaluate(eval_env, file_buffer));

		if(result->get_result_enum() == 2)
			current_bb = get_bb_at(*current_bb,result->get_value());
		else
			current_bb = get_next_bb(*current_bb);
	}
	file_buffer << "\n\n";
	file_buffer << LOC_VAR_SPACE << "Local Variables (after evaluating):\n";
	eval_env.print(file_buffer);

	return *result;

}