
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
#include <iomanip>
using namespace std;

#include"user-options.hh"
#include"error-display.hh"
#include"local-environment.hh"
#include"symbol-table.hh"
#include"ast.hh"
#include"basic-block.hh"
#include"procedure.hh"
#include"program.hh"
Ast::Ast()
{}

Ast::~Ast()
{}

bool Ast::check_ast(int line)
{
	report_internal_error("Should not reach, Ast : check_ast");
}

Data_Type Ast::get_data_type()
{
	report_internal_error("Should not reach, Ast : get_data_type");
}

void Ast::print_value(Local_Environment & eval_env, ostream & file_buffer)
{
	report_internal_error("Should not reach, Ast : print_value");
}

Eval_Result & Ast::get_value_of_evaluation(Local_Environment & eval_env)
{
	report_internal_error("Should not reach, Ast : get_value_of_evaluation");
}

void Ast::set_value_of_evaluation(Local_Environment & eval_env, Eval_Result & result)
{
	report_internal_error("Should not reach, Ast : set_value_of_evaluation");
}

void Ast::set_data_type(Data_Type value)
{
	report_internal_error("Should not reach, Ast : set_data_type");
}
////////////////////////////////////////////////////////////////

Assignment_Ast::Assignment_Ast(Ast * temp_lhs, Ast * temp_rhs)
{
	lhs = temp_lhs;
	rhs = temp_rhs;
}

Assignment_Ast::~Assignment_Ast()
{
	delete lhs;
	delete rhs;
}

Data_Type Assignment_Ast::get_data_type()
{
	return node_data_type;
}

bool Assignment_Ast::check_ast(int line)
{
	if (lhs->get_data_type() == rhs->get_data_type())
	{
		node_data_type = lhs->get_data_type();
		return true;
	}

	report_error("Assignment statement data type not compatible", line);
}
void Assignment_Ast::set_data_type(Data_Type value)
{
	node_data_type=value;
}

void Assignment_Ast::print_ast(ostream & file_buffer)
{
	file_buffer <<"\n"<< AST_SPACE << "Asgn:\n";

	file_buffer << AST_NODE_SPACE"LHS (";
	lhs->print_ast(file_buffer);
	file_buffer << ")\n";

	file_buffer << AST_NODE_SPACE << "RHS (";
	rhs->print_ast(file_buffer);
	file_buffer << ")\n";
}

Eval_Result & Assignment_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	Eval_Result & result = rhs->evaluate(eval_env, file_buffer);

	if (result.is_variable_defined() == false)
		report_error("Variable should be defined to be on rhs", NOLINE);

	lhs->set_value_of_evaluation(eval_env, result);

	// Print the result

	print_ast(file_buffer);

	lhs->print_value(eval_env, file_buffer);

	return result;
}
/////////////////////////////////////////////////////////////////

Name_Ast::Name_Ast(string & name, Symbol_Table_Entry & var_entry)
{
	variable_name = name;
	variable_symbol_entry = var_entry;
}

Name_Ast::~Name_Ast()
{}

Data_Type Name_Ast::get_data_type()
{
	return variable_symbol_entry.get_data_type();
}

void Name_Ast::print_ast(ostream & file_buffer)
{
	file_buffer << "Name : " << variable_name;
}
void Name_Ast::set_data_type(Data_Type value)
{
	node_data_type=value;
}

void Name_Ast::print_value(Local_Environment & eval_env, ostream & file_buffer)
{
	Eval_Result_Value * loc_var_val = eval_env.get_variable_value(variable_name);
	Eval_Result_Value * glob_var_val = interpreter_global_table.get_variable_value(variable_name);

	file_buffer << "\n" << AST_SPACE << variable_name << " : ";

	if (!eval_env.is_variable_defined(variable_name) && !interpreter_global_table.is_variable_defined(variable_name))
		file_buffer << "undefined";

	else if (eval_env.is_variable_defined(variable_name) && loc_var_val != NULL)
	{
		if (loc_var_val->get_result_enum() == int_result)
			file_buffer << (int)loc_var_val->get_value() << "\n";
		else if(loc_var_val->get_result_enum() == float_result)
		{
			file_buffer<< setiosflags(ios::fixed) << setprecision(2)<<loc_var_val->get_value() << "\n";
		}
		else
			report_internal_error("Result type can only be int and float");
	}

	else
	{
		if (glob_var_val->get_result_enum() == int_result)
		{
			if (glob_var_val == NULL)
				file_buffer << "0\n";
			else
				file_buffer << (int)glob_var_val->get_value() << "\n";
		}
		else if(glob_var_val->get_result_enum() == float_result)
		{
			if(glob_var_val == NULL)
				file_buffer << "0\n";
			else
				file_buffer<< setiosflags(ios::fixed) << setprecision(2)<< glob_var_val->get_value() << "\n";
		}
		else
			report_internal_error("Result type can only be int and float");
	}
	file_buffer << "\n";
}

Eval_Result & Name_Ast::get_value_of_evaluation(Local_Environment & eval_env)
{
	if (eval_env.does_variable_exist(variable_name))
	{
		Eval_Result * result = eval_env.get_variable_value(variable_name);
		return *result;
	}

	Eval_Result * result = interpreter_global_table.get_variable_value(variable_name);
	return *result;
}

void Name_Ast::set_value_of_evaluation(Local_Environment & eval_env, Eval_Result & result)
{
	Eval_Result_Value * i;
	if (result.get_result_enum() == int_result)
	{
		i = new Eval_Result_Value_Int();
	 	i->set_value(result.get_value());
	}
	else if(result.get_result_enum() == float_result)
	{
		i = new Eval_Result_Value_Float();
		i->set_value(result.get_value());
	}
	if (eval_env.does_variable_exist(variable_name))
		eval_env.put_variable_value(*i, variable_name);
	else
		interpreter_global_table.put_variable_value(*i, variable_name);
}

Eval_Result & Name_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	return get_value_of_evaluation(eval_env);
}

///////////////////////////////////////////////////////////////////////////////

template <class DATA_TYPE>
Number_Ast<DATA_TYPE>::Number_Ast(DATA_TYPE number, Data_Type constant_data_type)
{
	constant = number;
	node_data_type = constant_data_type;
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
void Number_Ast<DATA_TYPE>::set_data_type(Data_Type value)
{
	node_data_type=value;
}
template <class DATA_TYPE>
void Number_Ast<DATA_TYPE>::print_ast(ostream & file_buffer)
{
	if(node_data_type == float_data_type)
		file_buffer << "Num : "<< setiosflags(ios::fixed) << setprecision(2)<< constant;
	else
		file_buffer << "Num : "<< (int)constant;
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
	else if(node_data_type == float_data_type)
	{
		Eval_Result & result = *new Eval_Result_Value_Float();
		result.set_value(constant);
		return result;
	}
}

///////////////////////////////////////////////////////////////////////////////

Return_Ast::Return_Ast()
{
	is_return = false;
}
Return_Ast::Return_Ast(Ast * l)
{
	return_value = l;
	is_return = true;
}
Return_Ast::~Return_Ast()
{}

void Return_Ast::print_ast(ostream & file_buffer)
{
	if(!is_return)
		file_buffer<<"\n\n"<< AST_SPACE << "RETURN <NOTHING>\n";
	else
	{
		file_buffer<<"\n\n"<< AST_SPACE << "RETURN ";
		 return_value->print_ast(file_buffer);
		 file_buffer<<"\n\n";
	}
}
void Return_Ast::set_data_type(Data_Type value)
{
	node_data_type=value;
}
Eval_Result & Return_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	if(!is_return)
	{	
		Eval_Result & result = *new Eval_Result_Value_Int();
		file_buffer<<AST_SPACE<<"Return <NOTHING>\n";
		result.set_result_enum(void_result);
		return result;
	}
	else
	{
		file_buffer<<"\n\n"<< AST_SPACE << "RETURN ";
		Eval_Result & result = *new Eval_Result_Value_Int();
		return_value->print_ast(file_buffer);
		file_buffer<<"\n\n";
		result.set_result_enum(return_result);
		return result;
	}
}

template class Number_Ast<int>;
template class Number_Ast<float>; 

////////////////////////////////////////////////////////////////
//int block_num;

Goto_Ast::Goto_Ast(int a)
{
	block_num = a;
}

void Goto_Ast::print_ast(ostream & file_buffer)
{
	file_buffer << AST_SPACE << "Goto statement:\n";
	file_buffer	<< AST_NODE_SPACE"Successor: ";
	file_buffer << block_num << "\n";
}

int Goto_Ast::blocknum()
{
	return block_num;
}
void Goto_Ast::set_data_type(Data_Type value)
{
	node_data_type=value;
}

Eval_Result & Goto_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	Eval_Result & result = * new Eval_Result_Value_Int();
	print_ast(file_buffer);
	file_buffer << AST_SPACE << "GOTO (BB "<<this->block_num<<")\n";
	result.set_value(this->block_num);
	result.set_result_enum(void_result);
	return result;
}


/////////////////////////////////////////////////////////////////
Relational_Ast::Relational_Ast(Ast * temp_lhs, Ast * temp_rhs, rop oper)
{
	lhs = temp_lhs;
	rhs = temp_rhs;
	rel_oper = oper;
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

void Relational_Ast::set_data_type(Data_Type value)
{
	node_data_type=value;
}

bool Relational_Ast::check_ast(int line)
{
	if (lhs->get_data_type() == rhs->get_data_type())
	{
		node_data_type = lhs->get_data_type();
		return true;
	}
	else if(lhs->get_data_type() == float_data_type || rhs->get_data_type() == float_data_type)
	{
		node_data_type = float_data_type;
		return true;
	}
	report_error("Relational Expression lhs and rhs data types doesn't match", line);
}

void Relational_Ast::print_ast(ostream & file_buffer)
{
	//LE, LT, GT, GE, NE, EE
	switch(rel_oper)
	{
		case 0:
		{
			file_buffer << "\n" << AST_SPACE << "Condition: ";
			file_buffer << "LE\n";
			break;
		}
		case 1:
		{
			file_buffer << "\n" << AST_SPACE << "Condition: ";
			file_buffer << "LT\n";
			break;
		}
		case 2:
		{
			file_buffer << "\n" << AST_SPACE << "Condition: ";
			file_buffer << "GT\n";
			break;
		}
		case 3:
		{
			file_buffer << "\n" << AST_SPACE << "Condition: ";
			file_buffer << "GE\n";
			break;
		}
		case 4:
		{
			file_buffer << "\n" << AST_SPACE << "Condition: ";
			file_buffer << "NE\n";
			break;
		}
		case 5:
		{
			file_buffer << "\n" << AST_SPACE << "Condition: ";
			file_buffer << "EQ\n";
			break;
		}
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
		case 10:
		{
			file_buffer << "\n" << AST_SPACE << "Arith: ";
			file_buffer<<"UMINUS\n";
			file_buffer << "\n" << AST_NODE_SPACE << "   LHS (";
			rhs->print_ast(file_buffer);
			file_buffer << ")";
			return ;
		}
	}
	file_buffer << "\n" << AST_NODE_SPACE << "   LHS (";
	lhs->print_ast(file_buffer);
	file_buffer << ")";

	file_buffer << "\n" << AST_NODE_SPACE << "   RHS (";
	rhs->print_ast(file_buffer);
	file_buffer << ")";
}

Eval_Result & Relational_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{

	float lhs_int = lhs->evaluate(eval_env,file_buffer).get_value();
	float rhs_int = rhs->evaluate(eval_env,file_buffer).get_value();
	float answer;

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
		case 6:
		{
			answer = (lhs_int + rhs_int);
			break;
		}
		case 7:
		{
			answer = (lhs_int - rhs_int);
			break;
		}
		case 8:
		{
			answer = (lhs_int * rhs_int);
			break;
		}
		case 9:
		{
			answer = (lhs_int/rhs_int);
			break;
		}
		case 10:
		{
			answer = -1*(lhs_int);
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
// /////////////////////////////////////////////////////////////////
// 	Goto_Ast * lhs;
// 	Goto_Ast * rhs;
// 	Relational_Ast * cond;

IfCondition_Ast::IfCondition_Ast(Goto_Ast * temp_lhs, Goto_Ast * temp_rhs, Ast * relcond)
{
	lhs = temp_lhs;
	rhs = temp_rhs;
	cond = relcond;
}

IfCondition_Ast::~IfCondition_Ast()
{
	delete lhs;
	delete rhs;
}
void IfCondition_Ast::set_data_type(Data_Type value)
{
	node_data_type=value;
}

Data_Type IfCondition_Ast::get_data_type()
{
	return cond->get_data_type();
}

bool IfCondition_Ast::check_ast(int line)
{
	if (lhs->get_data_type() == rhs->get_data_type())
	{
		node_data_type = lhs->get_data_type();
		return true;
	}

	report_error("Conditional statement data type not compatible", line);
}

void IfCondition_Ast::print_ast(ostream & file_buffer)
{
	file_buffer << AST_SPACE << "If_Else statement:";
	cond->print_ast(file_buffer);
	file_buffer << "\n" << AST_NODE_SPACE <<"True Successor: ";
	file_buffer << lhs->blocknum()<<"\n";
	file_buffer << AST_NODE_SPACE <<"False Successor: ";
	file_buffer << rhs->blocknum()<<"\n";
}

Eval_Result & IfCondition_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	print_ast(file_buffer);
	Eval_Result& result = *new Eval_Result_Value_Int();
	int condition = cond->evaluate(eval_env,file_buffer).get_value();
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

///////////////////////////////////////////
Typecast_Ast::Typecast_Ast(Ast * temp_lhs,Data_Type data_type)
{
	typecast = temp_lhs;
	prev = node_data_type;
	node_data_type = data_type;
}

Typecast_Ast::~Typecast_Ast()
{

}

bool Typecast_Ast::check_ast(int line)
{
	return true;

}
void Typecast_Ast::set_data_type(Data_Type d)
{
	node_data_type = d;
}
Data_Type Typecast_Ast::get_data_type()
{
	return node_data_type;
}
void Typecast_Ast::print_ast(ostream & file_buffer)
{
	typecast->print_ast(file_buffer);
}
Eval_Result & Typecast_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	float answer  = typecast->evaluate(eval_env, file_buffer).get_value();
	if(node_data_type == int_data_type)
	{
		int k = answer;
		Eval_Result & result = *new Eval_Result_Value_Int();
		result.set_value(k);
		return result;
	}
	else if(node_data_type == float_data_type)
	{
		Eval_Result & result = *new Eval_Result_Value_Float();
		result.set_value(answer);
		return result;
	}
}

/////////////////////////////////////////////
Procedurecall_Ast::Procedurecall_Ast(string& n,list<Ast *> & args)
{
	name = n;
	arguments = args;
}

Procedurecall_Ast::~Procedurecall_Ast()
{

}

bool Procedurecall_Ast::check_ast(int line)
{
	return true;

}
void Procedurecall_Ast::set_data_type(Data_Type d)
{
	 node_data_type = d;
}
Data_Type Procedurecall_Ast::get_data_type()
{
	return node_data_type;
}
void Procedurecall_Ast::print_ast(ostream & file_buffer)
{
	Procedure * p;
	p = program_object.get_procedure(name);
	if (arguments.size() == 0)
		file_buffer<<"\n"<<AST_SPACE<<"FN CALL: "<<name<<"()";
	else
	{		
		file_buffer<<"\n"<<AST_SPACE<<"FN CALL: "<<name<<"(";
		list<Ast *> :: iterator it;
		for(it = arguments.begin();it!=arguments.end();it++)
		{
			file_buffer<<"\n"<<AST_NODE_SPACE;
			(*it)->print_ast(file_buffer);
		}	
		file_buffer<<")";
	}
}
Eval_Result & Procedurecall_Ast::evaluate(Local_Environment & eval_env, ostream & file_buffer)
{
	Procedure * prod = program_object.get_procedure(name);
	list<Eval_Result * > eval_result;
	list<Ast *> :: iterator it;
	for(it = arguments.begin(); it != arguments.end();it++)
	{
		
		eval_result.push_back(&((*it)->evaluate(eval_env,file_buffer)));
	}
	prod->evaluate(file_buffer); 
}

/////////////////////////////////////////////
