
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

#include<iostream>
#include<fstream>
#include<typeinfo>

using namespace std;

#include"common-classes.hh"
#include"error-display.hh"
#include"user-options.hh"
#include"local-environment.hh"
#include"icode.hh"
#include"reg-alloc.hh"
#include"symbol-table.hh"
#include"ast.hh"
#include"basic-block.hh"
#include"procedure.hh"
#include"program.hh"

Ast::Ast()
{}

Ast::~Ast()
{}

bool Ast::check_ast()
{
	stringstream msg;
	msg << "No check_ast() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

Data_Type Ast::get_data_type()
{
	stringstream msg;
	msg << "No get_data_type() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

Symbol_Table_Entry & Ast::get_symbol_entry()
{
	stringstream msg;
	msg << "No get_symbol_entry() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

void Ast::print_value(Local_Environment & eval_env, ostream & file_buffer)
{
	stringstream msg;
	msg << "No print_value() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

Eval_Result & Ast::get_value_of_evaluation(Local_Environment & eval_env)
{
	stringstream msg;
	msg << "No get_value_of_evaluation() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

void Ast::set_value_of_evaluation(Local_Environment & eval_env, Eval_Result & result)
{
	stringstream msg;
	msg << "No set_value_of_evaluation() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

Code_For_Ast & Ast::create_store_stmt(Register_Descriptor * store_register)
{
	stringstream msg;
	msg << "No create_store_stmt() function for " << typeid(*this).name();
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, msg.str());
}

////////////////////////////////////////////////////////////////

Assignment_Ast::Assignment_Ast(Ast * temp_lhs, Ast * temp_rhs, int line)
{
	lhs = temp_lhs;
	rhs = temp_rhs;

	ast_num_child = binary_arity;
	node_data_type = void_data_type;

	lineno = line;
}

Assignment_Ast::~Assignment_Ast()
{
	delete lhs;
	delete rhs;
}

bool Assignment_Ast::check_ast()
{
	CHECK_INVARIANT((rhs != NULL), "Rhs of Assignment_Ast cannot be null");
	CHECK_INVARIANT((lhs != NULL), "Lhs of Assignment_Ast cannot be null");

	if (lhs->get_data_type() == rhs->get_data_type())
	{
		node_data_type = lhs->get_data_type();
		return true;
	}

	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, 
		"Assignment statement data type not compatible");
}

void Assignment_Ast::print(ostream & file_buffer)
{
	file_buffer << "\n" << AST_SPACE << "Asgn:";

	file_buffer << "\n" << AST_NODE_SPACE << "LHS (";
	lhs->print(file_buffer);
	file_buffer << ")";

	file_buffer << "\n" << AST_NODE_SPACE << "RHS (";
	rhs->print(file_buffer);
	file_buffer << ")";
}

Eval_Result & Assignment_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	CHECK_INVARIANT((rhs != NULL), "Rhs of Assignment_Ast cannot be null");
	Eval_Result & result = rhs->evaluate(eval_env, file_buffer);

	CHECK_INPUT_AND_ABORT(result.is_variable_defined(), "Variable should be defined to be on rhs of Assignment_Ast", lineno);

	CHECK_INVARIANT((lhs != NULL), "Lhs of Assignment_Ast cannot be null");

	lhs->set_value_of_evaluation(eval_env, result);

	// Print the result

	print(file_buffer);

	lhs->print_value(eval_env, file_buffer);

	return result;
}

Code_For_Ast & Assignment_Ast::compile()
{
	/* 
		An assignment x = y where y is a variable is 
		compiled as a combination of load and store statements:
		(load) R <- y 
		(store) x <- R
		If y is a constant, the statement is compiled as:
		(imm_Load) R <- y 
		(store) x <- R
		where imm_Load denotes the load immediate operation.
	*/

	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");

	Code_For_Ast & load_stmt = rhs->compile();

	Register_Descriptor * load_register = load_stmt.get_reg();

	Code_For_Ast store_stmt = lhs->create_store_stmt(load_register);

	load_register->clear_lra_symbol_list();

	// Store the statement in ic_list

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (load_stmt.get_icode_list().empty() == false)
		ic_list = load_stmt.get_icode_list();

	if (store_stmt.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(), store_stmt.get_icode_list());

	Code_For_Ast * assign_stmt;
	if (ic_list.empty() == false)
		assign_stmt = new Code_For_Ast(ic_list, load_register);

	return *assign_stmt;
}

Code_For_Ast & Assignment_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");
	bool s = false;
	if ((typeid(*rhs) == typeid(Name_Ast)) || (typeid(*rhs) == typeid(Number_Ast<int>)))
	{	
		lra.optimize_lra(mc_2m, lhs, rhs);
		s = true;
	}

	Code_For_Ast load_stmt = rhs->compile_and_optimize_ast(lra);

	Register_Descriptor * result_register = load_stmt.get_reg();

	Code_For_Ast store_stmt = lhs->create_store_stmt(result_register);

	list<Icode_Stmt *> ic_list;
	if(!s)
	{
		result_register->clear_lra_symbol_list();
	}
	lhs->get_symbol_entry().update_register(result_register);
	if (load_stmt.get_icode_list().empty() == false)
		ic_list = load_stmt.get_icode_list();

	if (store_stmt.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(), store_stmt.get_icode_list());

	Code_For_Ast * assign_stmt;
	if (ic_list.empty() == false)
		assign_stmt = new Code_For_Ast(ic_list, result_register);

	return *assign_stmt;
}

/////////////////////////////////////////////////////////////////

Name_Ast::Name_Ast(string & name, Symbol_Table_Entry & var_entry, int line)
{
	variable_symbol_entry = &var_entry;

	CHECK_INVARIANT((variable_symbol_entry->get_variable_name() == name),
		"Variable's symbol entry is not matching its name");

	ast_num_child = zero_arity;
	node_data_type = variable_symbol_entry->get_data_type();
	lineno = line;
}

Name_Ast::~Name_Ast()
{}

Data_Type Name_Ast::get_data_type()
{
	return variable_symbol_entry->get_data_type();
}

Symbol_Table_Entry & Name_Ast::get_symbol_entry()
{
	return *variable_symbol_entry;
}

void Name_Ast::print(ostream & file_buffer)
{
	file_buffer << "Name : " << variable_symbol_entry->get_variable_name();
}

void Name_Ast::print_value(Local_Environment & eval_env, ostream & file_buffer)
{
	string variable_name = variable_symbol_entry->get_variable_name();

	Eval_Result * loc_var_val = eval_env.get_variable_value(variable_name);
	Eval_Result * glob_var_val = interpreter_global_table.get_variable_value(variable_name);

	file_buffer << "\n" << AST_SPACE << variable_name << " : ";

	if (!eval_env.is_variable_defined(variable_name) && !interpreter_global_table.is_variable_defined(variable_name))
		file_buffer << "undefined";

	else if (eval_env.is_variable_defined(variable_name) && loc_var_val != NULL)
	{
		CHECK_INVARIANT(loc_var_val->get_result_enum() == int_result, "Result type can only be int");
		file_buffer << loc_var_val->get_int_value() << "\n";
	}

	else
	{
		CHECK_INVARIANT(glob_var_val->get_result_enum() == int_result, 
			"Result type can only be int and float");

		if (glob_var_val == NULL)
			file_buffer << "0\n";
		else
			file_buffer << glob_var_val->get_int_value() << "\n";
	}
	file_buffer << "\n";
}

Eval_Result & Name_Ast::get_value_of_evaluation(Local_Environment & eval_env)
{
	string variable_name = variable_symbol_entry->get_variable_name();

	if (eval_env.does_variable_exist(variable_name))
	{
		CHECK_INPUT_AND_ABORT((eval_env.is_variable_defined(variable_name) == true), 
					"Variable should be defined before its use", lineno);

		Eval_Result * result = eval_env.get_variable_value(variable_name);
		return *result;
	}

	CHECK_INPUT_AND_ABORT((interpreter_global_table.is_variable_defined(variable_name) == true), 
				"Variable should be defined before its use", lineno);

	Eval_Result * result = interpreter_global_table.get_variable_value(variable_name);
	return *result;
}

void Name_Ast::set_value_of_evaluation(Local_Environment & eval_env, Eval_Result & result)
{
	Eval_Result * i;
	string variable_name = variable_symbol_entry->get_variable_name();

	if (variable_symbol_entry->get_data_type() == int_data_type)
		i = new Eval_Result_Value_Int();
	else
		CHECK_INPUT_AND_ABORT(CONTROL_SHOULD_NOT_REACH, "Type of a name can be int/float only", lineno);

	if (result.get_result_enum() == int_result)
	 	i->set_value(result.get_int_value());
	else
		CHECK_INPUT_AND_ABORT(CONTROL_SHOULD_NOT_REACH, "Type of a name can be int/float only", lineno);

	if (eval_env.does_variable_exist(variable_name))
		eval_env.put_variable_value(*i, variable_name);
	else
		interpreter_global_table.put_variable_value(*i, variable_name);
}

Eval_Result & Name_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	return get_value_of_evaluation(eval_env);
}

Code_For_Ast & Name_Ast::compile()
{
	Ics_Opd * opd = new Mem_Addr_Opd(*variable_symbol_entry);
	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register = machine_dscr_object.get_new_register(float_num);
	else
		result_register = machine_dscr_object.get_new_register(int_num);
		

	if (command_options.is_do_lra_selected() == false)
		variable_symbol_entry->update_register(result_register);
	
	Ics_Opd * register_opd = new Register_Addr_Opd(result_register);

	Icode_Stmt * load_stmt;
	if(node_data_type == int_data_type)
		load_stmt = new Move_IC_Stmt(load, opd, register_opd);
	else
		load_stmt = new Move_IC_Stmt(load_d, opd, register_opd);


	list<Icode_Stmt *> ic_list;
	ic_list.push_back(load_stmt);

	Code_For_Ast & load_code = *new Code_For_Ast(ic_list, result_register);

	return load_code;
}

Code_For_Ast & Name_Ast::create_store_stmt(Register_Descriptor * store_register)
{
	CHECK_INVARIANT((store_register != NULL), "Store register cannot be null");

	Ics_Opd * register_opd = new Register_Addr_Opd(store_register);
	Ics_Opd * opd = new Mem_Addr_Opd(*variable_symbol_entry);

	Icode_Stmt * store_stmt;
/*	cout<<variable_symbol_entry->get_variable_name()<< " " <<node_data_type<<endl;
*/	if(node_data_type == float_data_type)
		store_stmt= new Move_IC_Stmt(store_d, register_opd, opd);
	else
		store_stmt = new Move_IC_Stmt(store, register_opd, opd);

	if (command_options.is_do_lra_selected() == false)
		variable_symbol_entry->free_register(store_register);

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;
	ic_list.push_back(store_stmt);

	Code_For_Ast & name_code = *new Code_For_Ast(ic_list, store_register);

	return name_code;
}

Code_For_Ast & Name_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{
	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;;

	bool load_needed = lra.is_load_needed();

	Register_Descriptor * result_register = lra.get_register();
	CHECK_INVARIANT((result_register != NULL), "Register cannot be null");
	Ics_Opd * register_opd = new Register_Addr_Opd(result_register);

	Icode_Stmt * load_stmt = NULL;
	if (load_needed)
	{
		Ics_Opd * opd = new Mem_Addr_Opd(*variable_symbol_entry);

		load_stmt = new Move_IC_Stmt(load, opd, register_opd);
			
		ic_list.push_back(load_stmt);
	}

	Code_For_Ast & load_code = *new Code_For_Ast(ic_list, result_register);

	return load_code;
}

///////////////////////////////////////////////////////////////////////////////

template <class DATA_TYPE>
Number_Ast<DATA_TYPE>::Number_Ast(DATA_TYPE number, Data_Type constant_data_type, int line)
{
	constant = number;
	node_data_type = constant_data_type;

	ast_num_child = zero_arity;

	lineno = line;
}

template <class DATA_TYPE>
Number_Ast<DATA_TYPE>::~Number_Ast()
{}

template <class DATA_TYPE>
Data_Type Number_Ast<DATA_TYPE>::get_data_type()
{
	return node_data_type;
}

template <class DATA_TYPE>
void Number_Ast<DATA_TYPE>::print(ostream & file_buffer)
{
	file_buffer << std::fixed;
	file_buffer << std::setprecision(2);

	file_buffer << "Num : " << constant;
}

template <class DATA_TYPE>
Eval_Result & Number_Ast<DATA_TYPE>::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	if (node_data_type == int_data_type)
	{
		Eval_Result & result = *new Eval_Result_Value_Int();
		result.set_value(constant);

		return result;
	}
}

template <class DATA_TYPE>
Code_For_Ast & Number_Ast<DATA_TYPE>::compile()
{
	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register = machine_dscr_object.get_new_register(float_num);
	else
		result_register = machine_dscr_object.get_new_register(int_num);

	CHECK_INVARIANT((result_register != NULL), "Result register cannot be null");
	Ics_Opd * load_register = new Register_Addr_Opd(result_register);
	Ics_Opd * opd = new Const_Opd<DATA_TYPE>(constant);
	Icode_Stmt * load_stmt;

	if(node_data_type == int_data_type)
		load_stmt = new Move_IC_Stmt(imm_load, opd, load_register);
	else
		load_stmt = new Move_IC_Stmt(imm_load_d, opd, load_register);

	Symbol_Table_Entry & res = *new Symbol_Table_Entry();
	// res.set_register(result_register);
	result_register->update_symbol_information(res);	

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;
	ic_list.push_back(load_stmt);

	Code_For_Ast & num_code = *new Code_For_Ast(ic_list, result_register);

	return num_code;
}

template <class DATA_TYPE>
Code_For_Ast & Number_Ast<DATA_TYPE>::compile_and_optimize_ast(Lra_Outcome & lra)
{
	CHECK_INVARIANT((lra.get_register() != NULL), "Register assigned through optimize_lra cannot be null");
	Ics_Opd * load_register = new Register_Addr_Opd(lra.get_register());
	Ics_Opd * opd = new Const_Opd<int>(constant);

	Icode_Stmt * load_stmt = new Move_IC_Stmt(imm_load, opd, load_register);

	list<Icode_Stmt *> ic_list;
	ic_list.push_back(load_stmt);

	Code_For_Ast & num_code = *new Code_For_Ast(ic_list, lra.get_register());

	return num_code;
}

///////////////////////////////////////////////////////////////////////////////

Return_Ast::Return_Ast(int line)
{
	lineno = line;
	node_data_type = void_data_type;
	ast_num_child = zero_arity;
}

Return_Ast::~Return_Ast()
{}

void Return_Ast::print(ostream & file_buffer)
{
	file_buffer << "\n" << AST_SPACE << "Return <NOTHING>\n";
}

Eval_Result & Return_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	Eval_Result & result = *new Eval_Result_Value_Int();
	return result;
}

Code_For_Ast & Return_Ast::compile()
{
	Code_For_Ast & ret_code = *new Code_For_Ast();
	return ret_code;
}

Code_For_Ast & Return_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{
	Code_For_Ast & ret_code = *new Code_For_Ast();
	return ret_code;
}

template class Number_Ast<int>;

////////////////////////////////////////////////////////////////
//int block_num;

Goto_Ast::Goto_Ast(int a, int k)
{
	block_num = a;
	lineno = k;
}

void Goto_Ast::print(ostream & file_buffer)
{
	file_buffer << AST_SPACE << "Goto statement:\n";
	file_buffer	<< AST_NODE_SPACE"Successor: ";
	file_buffer << block_num << "\n";
}
bool Goto_Ast::check_ast()
{


}
Data_Type Goto_Ast::get_data_type()
{
	return node_data_type;
}

int Goto_Ast::blocknum()
{
	return block_num;
}

Eval_Result & Goto_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	Eval_Result & result = * new Eval_Result_Value_Int();
	print(file_buffer);
	file_buffer << AST_SPACE << "GOTO (BB "<<this->block_num<<")\n";
	result.set_value(this->block_num);
	result.set_result_enum(void_result);
	return result;
}

Code_For_Ast & Goto_Ast::compile()
{
	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register = machine_dscr_object.get_new_register(float_num);
	else
		result_register = machine_dscr_object.get_new_register(int_num);

	Icode_Stmt * goto_stmt = new Goto_IC_Stmt(g, blocknum());

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;
	ic_list.push_back(goto_stmt);

	Code_For_Ast * goto_expr;
	if(ic_list.empty()==false)
	{
		goto_expr = new Code_For_Ast(ic_list, result_register);
	}

	return *goto_expr;
}

Code_For_Ast & Goto_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{
	Register_Descriptor * result_register = machine_dscr_object.get_new_register();

	Icode_Stmt * goto_stmt = new Goto_IC_Stmt(g, blocknum());

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;
	ic_list.push_back(goto_stmt);

	Code_For_Ast * goto_expr;
	if(ic_list.empty()==false)
	{
		goto_expr = new Code_For_Ast(ic_list, result_register);
	}

	return *goto_expr;	
}
/////////////////////////////////////////////////////////////////
Relational_Ast::Relational_Ast(Ast * temp_lhs, Ast * temp_rhs, rop oper, int l)
{
	lhs = temp_lhs;
	rhs = temp_rhs;
	rel_oper = oper;
	lineno = l;
}

Relational_Ast::~Relational_Ast()
{
	delete lhs;
	delete rhs;
}

Data_Type Relational_Ast::get_data_type()
{
	return node_data_type;
}

bool Relational_Ast::check_ast()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");
	if (lhs->get_data_type() == int_data_type || rhs->get_data_type() == int_data_type ||
		lhs->get_data_type() == float_data_type || rhs->get_data_type() == float_data_type)
	{
		node_data_type = int_data_type;
		return true;
	}
	
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH,"Relational Expression lhs and rhs data types doesn't match");
}

void Relational_Ast::print(ostream & file_buffer)
{
	file_buffer << "\n" << AST_SPACE << "Condition: ";
	//LE, LT, GT, GE, NE, EE
	switch(rel_oper)
	{
		case 0:
		{
			file_buffer << "LE\n";
			break;
		}
		case 1:
		{
			file_buffer << "LT\n";
			break;
		}
		case 2:
		{
			file_buffer << "GT\n";
			break;
		}
		case 3:
		{
			file_buffer << "GE\n";
			break;
		}
		case 4:
		{
			file_buffer << "NE\n";
			break;
		}
		case 5:
		{
			file_buffer << "EQ\n";
			break;
		}
	}
	file_buffer << "\n" << AST_NODE_SPACE << "   LHS (";
	lhs->print(file_buffer);
	file_buffer << ")";

	file_buffer << "\n" << AST_NODE_SPACE << "   RHS (";
	rhs->print(file_buffer);
	file_buffer << ")";
}

Eval_Result & Relational_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	Eval_Result & result = * new Eval_Result_Value_Int();

	int lhs_int = lhs->evaluate(eval_env,file_buffer).get_int_value();
	int rhs_int = rhs->evaluate(eval_env,file_buffer).get_int_value();
	int answer;
	switch(rel_oper)
	{
		case 0:
		{
			answer = (lhs_int <= rhs_int);
			break;
		}
		case 1:
		{
			answer = (lhs_int < rhs_int);
			break;
		}
		case 2:
		{
			answer = (lhs_int > rhs_int);
			break;
		}
		case 3:
		{
			answer = (lhs_int >= rhs_int);
			break;
		}
		case 4:
		{
			answer = (lhs_int != rhs_int);
			break;
		}
		case 5:
		{
			answer = (lhs_int == rhs_int);
			break;
		}
	}

	if(node_data_type == float_data_type)
	{

		Eval_Result & result = * new Eval_Result_Value_Float();
		result.set_value(answer);
		return result;
	}
	else if (node_data_type == int_data_type)
	{
		int k = (int) answer;
		Eval_Result & result  = * new Eval_Result_Value_Int();
		result.set_value(k);
		return result;
	}
	else
	{
		file_buffer<<" "<<node_data_type<<endl;
		Eval_Result & result = * new Eval_Result_Value_Int();
		result.set_value(answer);
		return result;
	}
}

Code_For_Ast & Relational_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");

	Code_For_Ast & stmt_lhs = lhs->compile();
	Code_For_Ast & stmt_rhs = rhs->compile();

	Register_Descriptor * register_lhs = stmt_lhs.get_reg();
	Register_Descriptor * register_rhs = stmt_rhs.get_reg();

	Ics_Opd * register_opd_l = new Register_Addr_Opd(register_lhs);
	Ics_Opd * register_opd_r = new Register_Addr_Opd(register_rhs);

	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register = machine_dscr_object.get_new_register(float_num);
	else
		result_register = machine_dscr_object.get_new_register(int_num);

	register_lhs->clear_lra_symbol_list();
	register_rhs->clear_lra_symbol_list();

	Symbol_Table_Entry & res = *new Symbol_Table_Entry();
	// res.set_register(result_register);
	result_register->update_symbol_information(res);

	CHECK_INVARIANT((result_register != NULL), "Result register cannot be null");
	Ics_Opd * register_opd = new Register_Addr_Opd(result_register);
	
	Icode_Stmt * rel_stmt;

	if(rel_oper==GT)
	{
		rel_stmt = new Relational_IC_Stmt(sgt, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==LT)
	{
		rel_stmt = new Relational_IC_Stmt(slt, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==GE)
	{
		rel_stmt = new Relational_IC_Stmt(sge, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==LE)
	{
		rel_stmt = new Relational_IC_Stmt(sle, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==NE)
	{
		rel_stmt = new Relational_IC_Stmt(sne, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==EQ)
	{
		rel_stmt = new Relational_IC_Stmt(seq, register_opd_l , register_opd_r , register_opd);
	}
	else
	{
		CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, "Invalid operation");
	}

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (stmt_lhs.get_icode_list().empty() == false)
		ic_list = stmt_lhs.get_icode_list();

	if (stmt_rhs.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(), stmt_rhs.get_icode_list());

	ic_list.push_back(rel_stmt);

	Code_For_Ast * relational_expr;
	if (ic_list.empty() == false)
		relational_expr = new Code_For_Ast(ic_list, result_register);

	return *relational_expr;

}
Code_For_Ast & Relational_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");

	int f1 = (typeid(*lhs) == typeid(Name_Ast));
	int f2 = (typeid(*rhs) == typeid(Name_Ast));
	bool s1 = false;
	bool s2 = false;

	if(f1 || typeid(*lhs) == typeid(Number_Ast<int>))
	{
		lra.optimize_lra(mc_2r, NULL, lhs);
	}

	Code_For_Ast & stmt_lhs = lhs->compile_and_optimize_ast(lra);
	
	s1 = lra.is_source_register();

	if(f2 || typeid(*rhs) == typeid(Number_Ast<int>))
	{
		lra.optimize_lra(mc_2r, NULL, rhs);
	}

	Code_For_Ast & stmt_rhs = rhs->compile_and_optimize_ast(lra);

	s2 = lra.is_source_register();

	Register_Descriptor * register_lhs = stmt_lhs.get_reg();
	Register_Descriptor * register_rhs = stmt_rhs.get_reg();

	Ics_Opd * register_opd_l = new Register_Addr_Opd(register_lhs);
	Ics_Opd * register_opd_r = new Register_Addr_Opd(register_rhs);

	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register = machine_dscr_object.get_new_register(float_num);
	else
		result_register = machine_dscr_object.get_new_register(int_num);

	Symbol_Table_Entry & res = *new Symbol_Table_Entry();
	// res.set_register(result_register);
	result_register->update_symbol_information(res);

	CHECK_INVARIANT((result_register != NULL), "Result register cannot be null");
	Ics_Opd * register_opd = new Register_Addr_Opd(result_register);
	
	Icode_Stmt * rel_stmt;

	if(rel_oper==GT)
	{
		rel_stmt = new Relational_IC_Stmt(sgt, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==LT)
	{
		rel_stmt = new Relational_IC_Stmt(slt, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==GE)
	{
		rel_stmt = new Relational_IC_Stmt(sge, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==LE)
	{
		rel_stmt = new Relational_IC_Stmt(sle, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==NE)
	{
		rel_stmt = new Relational_IC_Stmt(sne, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==EQ)
	{
		rel_stmt = new Relational_IC_Stmt(seq, register_opd_l , register_opd_r , register_opd);
	}
	else
	{
		CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, "Invalid operation");
	}

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (stmt_lhs.get_icode_list().empty() == false)
		ic_list = stmt_lhs.get_icode_list();

	if (stmt_rhs.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(), stmt_rhs.get_icode_list());

	ic_list.push_back(rel_stmt);

	Code_For_Ast * relational_expr;
	if (ic_list.empty() == false)
		relational_expr = new Code_For_Ast(ic_list, result_register);

	if(f1 == 0 || !s1)
	{
		register_lhs->clear_lra_symbol_list();
	}
	if(f2 == 0 || !s2)
	{
		register_rhs->clear_lra_symbol_list();
	}

	return *relational_expr;
}
// /////////////////////////////////////////////////////////////////
Expression_Ast::Expression_Ast(Ast * temp_lhs, Ast * temp_rhs, rop oper, int l)
{
	lhs = temp_lhs;
	rhs = temp_rhs;
	rel_oper = oper;
	lineno = l;
}

Expression_Ast::~Expression_Ast()
{
	delete lhs;
	delete rhs;
}

Data_Type Expression_Ast::get_data_type()
{
	return node_data_type;
}

bool Expression_Ast::check_ast()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");
	if (lhs->get_data_type() == int_data_type || rhs->get_data_type() == int_data_type ||
		lhs->get_data_type() == float_data_type || rhs->get_data_type() == float_data_type)
	{
		node_data_type = lhs->get_data_type();
		return true;
	}
	
	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH,"Relational Expression lhs and rhs data types doesn't match");
}

void Expression_Ast::print(ostream & file_buffer)
{
	file_buffer << "\n" << AST_SPACE << "Condition: ";
	//LE, LT, GT, GE, NE, EE
	switch(rel_oper)
	{
		case 6:
		{
			file_buffer << "\n" << AST_SPACE << "Arith: ";
			file_buffer<<"PLUS\n";
			break;
		}
		case 7:
		{
			file_buffer << "\n" << AST_SPACE << "Arith: ";
			file_buffer<<"MINUS\n";
			break;
		}
		case 8:
		{
			file_buffer << "\n" << AST_SPACE << "Arith: ";
			file_buffer<<"MULT\n";
			break;
		}
		case 9:
		{
			file_buffer << "\n" << AST_SPACE << "Arith: ";
			file_buffer<<"DIV\n";
			break;
		}
	}
	file_buffer << "\n" << AST_NODE_SPACE << "   LHS (";
	lhs->print(file_buffer);
	file_buffer << ")";

	file_buffer << "\n" << AST_NODE_SPACE << "   RHS (";
	rhs->print(file_buffer);
	file_buffer << ")";
}

Eval_Result & Expression_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	Eval_Result & result = * new Eval_Result_Value_Int();

	int lhs_int = lhs->evaluate(eval_env,file_buffer).get_int_value();
	int rhs_int = rhs->evaluate(eval_env,file_buffer).get_int_value();
	int answer;
	switch(rel_oper)
	{
		case PLUS:
		{
			answer = (lhs_int + rhs_int);
			break;
		}
		case MINUS:
		{
			answer = (lhs_int - rhs_int);
			break;
		}
		case MULT:
		{
			answer = (lhs_int * rhs_int);
			break;
		}
		case DIV:
		{
			answer = (lhs_int / rhs_int);
			break;
		}
	}

	if(node_data_type == float_data_type)
	{

		Eval_Result & result = * new Eval_Result_Value_Float();
		result.set_value(answer);
		return result;
	}
	else if (node_data_type == int_data_type)
	{
		int k = (int) answer;
		Eval_Result & result  = * new Eval_Result_Value_Int();
		result.set_value(k);
		return result;
	}
	else
	{
		file_buffer<<" "<<node_data_type<<endl;
		Eval_Result & result = * new Eval_Result_Value_Float();
		result.set_value(answer);
		return result;
	}
}

Code_For_Ast & Expression_Ast::compile()
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");

	Code_For_Ast & stmt_lhs = lhs->compile();
	Code_For_Ast & stmt_rhs = rhs->compile();

	Register_Descriptor * register_lhs = stmt_lhs.get_reg();
	Register_Descriptor * register_rhs = stmt_rhs.get_reg();

	Ics_Opd * register_opd_l = new Register_Addr_Opd(register_lhs);
	Ics_Opd * register_opd_r = new Register_Addr_Opd(register_rhs);

	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register = machine_dscr_object.get_new_register(float_num);
	else
		result_register = machine_dscr_object.get_new_register(int_num);

	register_lhs->clear_lra_symbol_list();
	register_rhs->clear_lra_symbol_list();

	Symbol_Table_Entry & res = *new Symbol_Table_Entry();
	// res.set_register(result_register);
	result_register->update_symbol_information(res);

	CHECK_INVARIANT((result_register != NULL), "Result register cannot be null");
	Ics_Opd * register_opd = new Register_Addr_Opd(result_register);
	
	Icode_Stmt * rel_stmt;

	
	if(rel_oper==PLUS)
	{
		if(node_data_type == int_data_type)
			rel_stmt = new Relational_IC_Stmt(plus_, register_opd_l , register_opd_r , register_opd);
		else
			rel_stmt = new Relational_IC_Stmt(plus_d, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==MINUS)
	{
		if(node_data_type == int_data_type)
			rel_stmt = new Relational_IC_Stmt(minus_, register_opd_l , register_opd_r , register_opd);
		else
			rel_stmt = new Relational_IC_Stmt(minus_d, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==MULT)
	{
		if(node_data_type == int_data_type)
			rel_stmt = new Relational_IC_Stmt(mult_, register_opd_l , register_opd_r , register_opd);
		else
			rel_stmt = new Relational_IC_Stmt(mult_d, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==DIV)
	{
		if(node_data_type == int_data_type)
			rel_stmt = new Relational_IC_Stmt(div_, register_opd_l , register_opd_r , register_opd);
		else
			rel_stmt = new Relational_IC_Stmt(div_d, register_opd_l , register_opd_r , register_opd);
	}
	else
	{
		CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, "Invalid operation");
	}

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (stmt_lhs.get_icode_list().empty() == false)
		ic_list = stmt_lhs.get_icode_list();

	if (stmt_rhs.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(), stmt_rhs.get_icode_list());

	ic_list.push_back(rel_stmt);

	Code_For_Ast * relational_expr;
	if (ic_list.empty() == false)
		relational_expr = new Code_For_Ast(ic_list, result_register);

	return *relational_expr;

}
Code_For_Ast & Expression_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{
	CHECK_INVARIANT((lhs != NULL), "Lhs cannot be null");
	CHECK_INVARIANT((rhs != NULL), "Rhs cannot be null");

	int f1 = (typeid(*lhs) == typeid(Name_Ast));
	int f2 = (typeid(*rhs) == typeid(Name_Ast));
	bool s1 = false;
	bool s2 = false;

	if(f1 || typeid(*lhs) == typeid(Number_Ast<int>))
	{
		lra.optimize_lra(mc_2r, NULL, lhs);
	}

	Code_For_Ast & stmt_lhs = lhs->compile_and_optimize_ast(lra);
	
	s1 = lra.is_source_register();

	if(f2 || typeid(*rhs) == typeid(Number_Ast<int>))
	{
		lra.optimize_lra(mc_2r, NULL, rhs);
	}

	Code_For_Ast & stmt_rhs = rhs->compile_and_optimize_ast(lra);

	s2 = lra.is_source_register();

	Register_Descriptor * register_lhs = stmt_lhs.get_reg();
	Register_Descriptor * register_rhs = stmt_rhs.get_reg();

	Ics_Opd * register_opd_l = new Register_Addr_Opd(register_lhs);
	Ics_Opd * register_opd_r = new Register_Addr_Opd(register_rhs);

	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register = machine_dscr_object.get_new_register(float_num);
	else
		result_register = machine_dscr_object.get_new_register(int_num);

	Symbol_Table_Entry & res = *new Symbol_Table_Entry();
	// res.set_register(result_register);
	result_register->update_symbol_information(res);

	CHECK_INVARIANT((result_register != NULL), "Result register cannot be null");
	Ics_Opd * register_opd = new Register_Addr_Opd(result_register);
	
	Icode_Stmt * rel_stmt;

	if(rel_oper==GT)
	{
		rel_stmt = new Relational_IC_Stmt(sgt, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==LT)
	{
		rel_stmt = new Relational_IC_Stmt(slt, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==GE)
	{
		rel_stmt = new Relational_IC_Stmt(sge, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==LE)
	{
		rel_stmt = new Relational_IC_Stmt(sle, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==NE)
	{
		rel_stmt = new Relational_IC_Stmt(sne, register_opd_l , register_opd_r , register_opd);
	}
	else if(rel_oper==EQ)
	{
		rel_stmt = new Relational_IC_Stmt(seq, register_opd_l , register_opd_r , register_opd);
	}
	else
	{
		CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, "Invalid operation");
	}

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (stmt_lhs.get_icode_list().empty() == false)
		ic_list = stmt_lhs.get_icode_list();

	if (stmt_rhs.get_icode_list().empty() == false)
		ic_list.splice(ic_list.end(), stmt_rhs.get_icode_list());

	ic_list.push_back(rel_stmt);

	Code_For_Ast * relational_expr;
	if (ic_list.empty() == false)
		relational_expr = new Code_For_Ast(ic_list, result_register);

	if(f1 == 0 || !s1)
	{
		register_lhs->clear_lra_symbol_list();
	}
	if(f2 == 0 || !s2)
	{
		register_rhs->clear_lra_symbol_list();
	}

	return *relational_expr;
}

// 	Goto_Ast * lhs;
// 	Goto_Ast * rhs;
// 	Relational_Ast * cond;

IfCondition_Ast::IfCondition_Ast(Goto_Ast * temp_lhs, Goto_Ast * temp_rhs, Ast * relcond, int a)
{
	lhs = temp_lhs;
	rhs = temp_rhs;
	cond = relcond;
	lineno = a;
}

IfCondition_Ast::~IfCondition_Ast()
{
	delete lhs;
	delete rhs;
}

Data_Type IfCondition_Ast::get_data_type()
{
	return cond->get_data_type();
}

/*bool IfCondition_Ast::check_ast()
{
	if (lhs->get_data_type() == rhs->get_data_type())
	{
		node_data_type = lhs->get_data_type();
		return true;
	}

	CHECK_INVARIANT(CONTROL_SHOULD_NOT_REACH, "Conditional statement data type not compatible");
}
*/
void IfCondition_Ast::print(ostream & file_buffer)
{
	file_buffer << AST_SPACE << "If_Else statement:";
	cond->print(file_buffer);
	file_buffer << "\n" << AST_NODE_SPACE <<"True Successor: ";
	file_buffer << lhs->blocknum()<<"\n";
	file_buffer << AST_NODE_SPACE <<"False Successor: ";
	file_buffer << rhs->blocknum()<<"\n";
}

Eval_Result & IfCondition_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	print(file_buffer);
	Eval_Result& result = *new Eval_Result_Value_Int();
	int condition = cond->evaluate(eval_env,file_buffer).get_int_value();
	if(condition)
	{
		int bnum = lhs->blocknum();
		file_buffer<<"\n"<<AST_SPACE<<"Condition True : ";
		file_buffer <<"Goto (BB "<<bnum<<")\n";
		result.set_value(bnum);
		result.set_result_enum(void_result);
	}
	else
	{
		int bnum = rhs->blocknum();
		file_buffer<<"\n"<<AST_SPACE<<"Condition False : ";
		file_buffer <<"Goto (BB "<<bnum<<")\n";
		result.set_value(bnum);
		result.set_result_enum(void_result);

	}
	
	return result;
}

Code_For_Ast & IfCondition_Ast::compile()
{
	CHECK_INVARIANT((cond != NULL), "Condition cannot be null");
	
	Code_For_Ast & load_stmt = cond->compile();
	
	Register_Descriptor * load_register = load_stmt.get_reg();
	
	Ics_Opd * register_opd = new Register_Addr_Opd(load_register);
	Ics_Opd * zero_opd = new Register_Addr_Opd(machine_dscr_object.spim_register_table[zero]);

	Icode_Stmt * cond_stmt_branch = new Branch_IC_Stmt(bne, register_opd , zero_opd , lhs->blocknum());
	Icode_Stmt * cond_stmt_goto = new Goto_IC_Stmt(g, rhs->blocknum());
	
	load_register->clear_lra_symbol_list();
	
	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (load_stmt.get_icode_list().empty() == false)
		ic_list = load_stmt.get_icode_list();

	ic_list.push_back(cond_stmt_branch);
	ic_list.push_back(cond_stmt_goto);

	Code_For_Ast * if_else_stmt;
	if (ic_list.empty() == false)
		if_else_stmt = new Code_For_Ast(ic_list, load_register);

	return *if_else_stmt;
}	

Code_For_Ast & IfCondition_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{

	int f1 = (typeid(*cond) == typeid(Name_Ast));
	if(f1 || typeid(*cond) == typeid(Number_Ast<int>))
	{
		lra.optimize_lra(mc_2r, NULL, cond);
	}
	CHECK_INVARIANT((cond != NULL), "Condition cannot be null");
	
	bool src = lra.is_source_register();

	Code_For_Ast & load_stmt = cond->compile_and_optimize_ast(lra);
	
	Register_Descriptor * load_register = load_stmt.get_reg();
	
	Ics_Opd * register_opd = new Register_Addr_Opd(load_register);
	Ics_Opd * zero_opd = new Register_Addr_Opd(machine_dscr_object.spim_register_table[zero]);

	Icode_Stmt * cond_stmt_branch = new Branch_IC_Stmt(bne, register_opd , zero_opd , lhs->blocknum());
	Icode_Stmt * cond_stmt_goto = new Goto_IC_Stmt(g, rhs->blocknum());
	
	
	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (load_stmt.get_icode_list().empty() == false)
		ic_list = load_stmt.get_icode_list();

	ic_list.push_back(cond_stmt_branch);
	ic_list.push_back(cond_stmt_goto);

	Code_For_Ast * if_else_stmt;
	if (ic_list.empty() == false)
		if_else_stmt = new Code_For_Ast(ic_list, load_register);

	if(f1 == 0 || src==0)
	{
		load_register->clear_lra_symbol_list();
	}

	return *if_else_stmt;	
}
///////////////////////////////////////////
/////Typecast_Ast//////////////////////////
Typecast_Ast::Typecast_Ast(Ast * temp,Data_Type type,int line)
{
	typecast = temp;
	prev = node_data_type;
	node_data_type = type;
	lineno = line;
}
Typecast_Ast::~Typecast_Ast()
{

}
Data_Type Typecast_Ast::get_data_type()
{
	return node_data_type;
}
bool Typecast_Ast::check_ast()
{
	return true;
}
Eval_Result & Typecast_Ast::evaluate(Local_Environment & eval_env,ostream & file_buffer)
{
	float answer  = typecast->evaluate(eval_env, file_buffer).get_int_value();
	if(node_data_type == int_data_type){
		int k = answer;
		Eval_Result & result = *new Eval_Result_Value_Int();
		result.set_value(k);
		return result;
	}
	else if(node_data_type == float_data_type){
		Eval_Result & result = *new Eval_Result_Value_Float();
		result.set_value(answer);
		return result;
	}
}

void Typecast_Ast::print(ostream & file_buffer)
{
	typecast->print(file_buffer);
}

Code_For_Ast & Typecast_Ast::compile()
{
	CHECK_INVARIANT((typecast != NULL), "Condition cannot be null");
	
	Code_For_Ast & load_stmt = typecast->compile();
	
	Register_Descriptor * load_register = load_stmt.get_reg();

	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register= machine_dscr_object.get_new_register(float_num);
	else
		result_register= machine_dscr_object.get_new_register(int_num);

	Symbol_Table_Entry & res = *new Symbol_Table_Entry();
	// res.set_register(result_register);
	result_register->update_symbol_information(res);


	Ics_Opd * register_opd = new Register_Addr_Opd(load_register);
	Ics_Opd * register_opd_1 = new Register_Addr_Opd(result_register);

	Icode_Stmt * cast_stmt;

	Code_For_Ast * if_else_stmt;

	
	if(node_data_type == float_data_type)
	{
		if(typecast->get_data_type() == int_data_type)
		{
			load_register->clear_lra_symbol_list();
	
			list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

			if (load_stmt.get_icode_list().empty() == false)
				ic_list = load_stmt.get_icode_list();

			cast_stmt = new Cast_IC_Stmt(mtc1, register_opd, register_opd_1);
			ic_list.push_back(cast_stmt);

			if (ic_list.empty() == false)
				if_else_stmt = new Code_For_Ast(ic_list, result_register);
		}
		else
		{
			result_register->clear_lra_symbol_list();
	
			list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

			if (load_stmt.get_icode_list().empty() == false)
				ic_list = load_stmt.get_icode_list();

			if (ic_list.empty() == false)
				if_else_stmt = new Code_For_Ast(ic_list, load_register);

		}
	}
	else
	{
		if(typecast->get_data_type() == float_data_type)
		{
			load_register->clear_lra_symbol_list();
	
			list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

			if (load_stmt.get_icode_list().empty() == false)
				ic_list = load_stmt.get_icode_list();

			cast_stmt = new Cast_IC_Stmt(mfc1, register_opd, register_opd_1);
			ic_list.push_back(cast_stmt);

			if (ic_list.empty() == false)
				if_else_stmt = new Code_For_Ast(ic_list, result_register);
		}
		else
		{
			result_register->clear_lra_symbol_list();
	
			list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

			if (load_stmt.get_icode_list().empty() == false)
				ic_list = load_stmt.get_icode_list();

			if (ic_list.empty() == false)
				if_else_stmt = new Code_For_Ast(ic_list, load_register);

		}

	}

	

	return *if_else_stmt;



}
Code_For_Ast & Typecast_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{

}
////////////////////////////////////////////////////////////////////////////////////////

Uminus_Ast::Uminus_Ast(Ast * temp,int line)
{
	typecast = temp;
	node_data_type = temp->get_data_type() ;
}
Uminus_Ast::~Uminus_Ast()
{

}
Data_Type Uminus_Ast::get_data_type()
{
	return node_data_type;
}
bool Uminus_Ast::check_ast()
{
	return true;
}
Eval_Result & Uminus_Ast::evaluate(Local_Environment & eval_env,ostream & file_buffer)
{
	float answer  = typecast->evaluate(eval_env, file_buffer).get_int_value();
	if(node_data_type == int_data_type){
		int k = -1*answer;
		Eval_Result & result = *new Eval_Result_Value_Int();
		result.set_value(k);
		return result;
	}
	else if(node_data_type == float_data_type){
		Eval_Result & result = *new Eval_Result_Value_Float();
		result.set_value(-1*answer);
		return result;
	}
}

void Uminus_Ast::print(ostream & file_buffer)
{
	typecast->print(file_buffer);
}

Code_For_Ast & Uminus_Ast::compile()
{
	CHECK_INVARIANT((typecast != NULL), "Condition cannot be null");
	
	Code_For_Ast & load_stmt = typecast->compile();
	
	Register_Descriptor * load_register = load_stmt.get_reg();

	Register_Descriptor * result_register;
	if(node_data_type == float_data_type)
		result_register= machine_dscr_object.get_new_register(float_num);
	else
		result_register= machine_dscr_object.get_new_register(int_num);

	Ics_Opd * register_opd = new Register_Addr_Opd(load_register);
	Ics_Opd * register_opd_1 = new Register_Addr_Opd(result_register);

	Icode_Stmt * cast_stmt;

	Code_For_Ast * if_else_stmt;

	Symbol_Table_Entry & res = *new Symbol_Table_Entry();
	// res.set_register(result_register);
	result_register->update_symbol_information(res);

	load_register->clear_lra_symbol_list();

	list<Icode_Stmt *> & ic_list = *new list<Icode_Stmt *>;

	if (load_stmt.get_icode_list().empty() == false)
		ic_list = load_stmt.get_icode_list();

	if(node_data_type == int_data_type)
		cast_stmt = new Uminus_IC_Stmt(uminus, register_opd, register_opd_1);
	else
		cast_stmt = new Uminus_IC_Stmt(uminus_d, register_opd, register_opd_1);
	ic_list.push_back(cast_stmt);

	if (ic_list.empty() == false)
		if_else_stmt = new Code_For_Ast(ic_list, result_register);

	

	return *if_else_stmt;



}
Code_For_Ast & Uminus_Ast::compile_and_optimize_ast(Lra_Outcome & lra)
{

}

template class Number_Ast<float>;
