
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

#ifndef AST_HH
#define AST_HH

#include<string>

#define AST_SPACE "         "
#define AST_NODE_SPACE "            "

using namespace std;

enum rop { LE, LT, GT, GE, NE, EQ,PLUS,MINUS,MULT,DIV,UNARY};

class Ast;

class Ast
{
protected:
	Data_Type node_data_type;
public:
	Ast();
	~Ast();

	virtual Data_Type get_data_type();
	virtual bool check_ast(int line);
	virtual void set_data_type(Data_Type value);
	virtual void print_ast(ostream & file_buffer) = 0;
	virtual void print_value(Local_Environment & eval_env, ostream & file_buffer);

	virtual Eval_Result & get_value_of_evaluation(Local_Environment & eval_env);
	virtual void set_value_of_evaluation(Local_Environment & eval_env, Eval_Result & result);
	virtual Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer) = 0;
};

class Assignment_Ast:public Ast
{
	Ast * lhs;
	Ast * rhs;

public:
	Assignment_Ast(Ast * temp_lhs, Ast * temp_rhs);
	~Assignment_Ast();

	Data_Type get_data_type();
	bool check_ast(int line);
	void set_data_type(Data_Type value);
	
	void print_ast(ostream & file_buffer);

	Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer);
};

class Name_Ast:public Ast
{
	string variable_name;
	Symbol_Table_Entry variable_symbol_entry;

public:
	Name_Ast(string & name, Symbol_Table_Entry & var_entry);
	~Name_Ast();

	Data_Type get_data_type();
	void set_data_type(Data_Type value);
	
	void print_ast(ostream & file_buffer);

	void print_value(Local_Environment & eval_env, ostream & file_buffer);
	Eval_Result & get_value_of_evaluation(Local_Environment & eval_env);
	void set_value_of_evaluation(Local_Environment & eval_env, Eval_Result & result);
	Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer);
};

template <class T>
class Number_Ast:public Ast
{
	T constant;

public:
	Number_Ast(T number, Data_Type constant_data_type);
	~Number_Ast();
	void set_data_type(Data_Type value);
	
	Data_Type get_data_type();

	void print_ast(ostream & file_buffer);

	Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer);
};

class Return_Ast:public Ast
{
	Ast * return_value;
	bool is_return;
public:
	Return_Ast();
	Return_Ast(Ast * l);
	~Return_Ast();
	void set_data_type(Data_Type value);
	void print_ast(ostream & file_buffer);

	Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer);
};

class Goto_Ast:public Ast
{
	int block_num;

public:
	Goto_Ast(int a);

	void print_ast(ostream & file_buffer);
	void set_data_type(Data_Type value);
	
	int blocknum();

	Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer);
};

class Relational_Ast:public Ast
{
	Ast * lhs;
	Ast * rhs;
	rop rel_oper;
public:
	Relational_Ast(Ast * temp_lhs, Ast * temp_rhs, rop a);
	void set_data_type(Data_Type value);
	
	~Relational_Ast();

	Data_Type get_data_type();

	bool check_ast(int line);

	void print_ast(ostream & file_buffer);

	Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer);
	
};

class IfCondition_Ast:public Ast
{
	Goto_Ast * lhs;
	Goto_Ast * rhs;
	Ast * cond;
public:
	IfCondition_Ast(Goto_Ast * temp_lhs, Goto_Ast * temp_rhs, Ast * cond);

	~IfCondition_Ast();
	void set_data_type(Data_Type value);
	Data_Type get_data_type();

	bool check_ast(int line);

	void print_ast(ostream & file_buffer);

	Eval_Result & evaluate(Local_Environment & eval_env, ostream & file_buffer);

	
};

class Typecast_Ast:public Ast
{
	Ast * typecast;
	Data_Type prev;
public:
	Typecast_Ast(Ast * temp,Data_Type type);
	~Typecast_Ast();
	Data_Type get_data_type();
	void set_data_type(Data_Type d);
	bool check_ast(int line);
	void print_ast(ostream & file_buffer);
	Eval_Result & evaluate(Local_Environment & eval_env,ostream & file_buffer);
};
class Procedurecall_Ast:public Ast
{
	string name;
	list<Ast *> arguments;
public:
	Procedurecall_Ast(string & n,list<Ast *> & args);
	~Procedurecall_Ast();
	Data_Type get_data_type();
	void set_data_type(Data_Type d);
	bool check_ast(int line);
	void print_ast(ostream & file_buffer);
	Eval_Result & evaluate(Local_Environment & eval_env,ostream & file_buffer);
};


#endif
