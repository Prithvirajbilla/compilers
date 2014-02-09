
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

%scanner ../scanner.h
%scanner-token-function d_scanner.lex()
%filenames parser
%parsefun-source parser.cc

%union 
{
	int integer_value;
	double float_value;
	std::string * string_value;
	list<Ast *> * ast_list;
	Ast * ast;
	rop rel_op;
	Goto_Ast * goto_ast;
	IfCondition_Ast * ifelse_ast;
	Relational_Ast * rel_ast;
	Symbol_Table * symbol_table;
	Symbol_Table_Entry * symbol_entry;
	Basic_Block * basic_block;
	list<Basic_Block *> * basic_block_list;
	Procedure * procedure;
};

%token <float_value> FLOAT_NUMBER
%token <integer_value> INTEGER_NUMBER
%token <integer_value> basicblock_number
%token <string_value> NAME
%token RETURN INTEGER FLOAT
%token IF 
%token ELSE
%token GOTO
%token ASSIGN_OP
%left OP2 OP3
%left OP7 OP5 OP6 OP4  
%left MINUS ADD
%left DIV MULT

%type <symbol_table> declaration_statement_list
%type <symbol_entry> declaration_statement
%type <basic_block_list> basic_block_list
%type <basic_block> basic_block
%type <ast_list> executable_statement_list
%type <ast_list> assignment_statement_list
%type <ast> assignment_statement
%type <ast> variable
%type <ast> constant
%type <ast> IFELSE
%type <goto_ast> GOTO_exp
%type <ast> conditional_exp
%start program

%%

program:
	declaration_statement_list procedure_name
	{
		
		program_object.set_global_table(*$1);
		
	}
	procedure_body
	{
		
		program_object.set_procedure_map(current_procedure, get_line_number());

		if ($1)
			$1->global_list_in_proc_map_check(get_line_number());

		delete $1;
		
	}
|
	procedure_name
	{
		
		// return_statement_used_flag = false;				 No return statement in the current procedure till now
		
	}
	procedure_body
	{	
		
		program_object.set_procedure_map(current_procedure, get_line_number());
		
	}
;

procedure_name:
	NAME '(' ')'
	{
		
		current_procedure = new Procedure(void_data_type, *$1);
		
	}
;

procedure_body:
	'{' declaration_statement_list
	{
		
		current_procedure->set_local_list(*$2);
		delete $2;
		
	}
	basic_block_list '}'
	{
		
		current_procedure->set_basic_block_list(*$4);

		delete $4;
		
	}
|
	'{' basic_block_list '}'
	{

		current_procedure->set_basic_block_list(*$2);

		delete $2;
		
	}
;

declaration_statement_list:
	declaration_statement
	{
		
		int line = get_line_number();
		program_object.variable_in_proc_map_check($1->get_variable_name(), line);

		string var_name = $1->get_variable_name();
		if (current_procedure && current_procedure->get_proc_name() == var_name)
		{
			int line = get_line_number();
			report_error("Variable name cannot be same as procedure name", line);
		}

		$$ = new Symbol_Table();
		$$->push_symbol($1);
		
	}
|
	declaration_statement_list declaration_statement
	{
		
		// if declaration is local then no need to check in global list
		// if declaration is global then this list is global list

		int line = get_line_number();
		program_object.variable_in_proc_map_check($2->get_variable_name(), line);

		string var_name = $2->get_variable_name();
		if (current_procedure && current_procedure->get_proc_name() == var_name)
		{
			int line = get_line_number();
			report_error("Variable name cannot be same as procedure name", line);
		}

		if ($1 != NULL)
		{
			if($1->variable_in_symbol_list_check(var_name))
			{
				int line = get_line_number();
				report_error("Variable is declared twice", line);
			}

			$$ = $1;
		}

		else
			$$ = new Symbol_Table();

		$$->push_symbol($2);
		
	}
;

declaration_statement:
	INTEGER NAME ';'
	{
		
		$$ = new Symbol_Table_Entry(*$2, int_data_type);

		delete $2;
		
	}
;

basic_block_list:
	basic_block_list basic_block
	{
		
		if (!$2)
		{
			int line = get_line_number();
			report_error("Basic block doesn't exist", line);
		}

		bb_strictly_increasing_order_check($$, $2->get_bb_number());

		$$ = $1;
		$$->push_back($2);
		
	}
|
	basic_block
	{
		
		if (!$1)
		{
			int line = get_line_number();
			report_error("Basic block doesn't exist", line);
		}

		$$ = new list<Basic_Block *>;
		$$->push_back($1);
		
	}
	
;

basic_block:
	
	basicblock_number ':' executable_statement_list
	{
		if ($1 < 2)
		{
			int line = get_line_number();
			report_error("Illegal basic block lable", line);
		}

		if ($3 != NULL)
			$$ = new Basic_Block($1, *$3);
		else
		{
			list<Ast *> * ast_list = new list<Ast *>;
			$$ = new Basic_Block($1, *ast_list);
		}

		delete $3;
	}
;

executable_statement_list:
	assignment_statement_list
	{
		
		$$ = $1;
		
	}
|
	assignment_statement_list RETURN ';'
	{
		
		Ast * ret = new Return_Ast();


		if ($1 != NULL)
			$$ = $1;

		else
			$$ = new list<Ast *>;

		$$->push_back(ret);
		
	}
|
	assignment_statement_list IFELSE
	{

		if($1 != NULL)
			$$ = $1;
		else
			$$ = new list<Ast *>;
		$$->push_back($2);
	}
|
	assignment_statement_list GOTO_exp
	{

		if($1 != NULL)
			$$ = $1;
		else
			$$ = new list<Ast *>;
		$$->push_back($2); 
	}
;

assignment_statement_list:
	{
		
		$$ = NULL;
		
	}
|
	assignment_statement_list assignment_statement
	{
		
		if ($1 == NULL)
			$$ = new list<Ast *>;

		else
			$$ = $1;

		$$->push_back($2);
		
	}
;

assignment_statement:
	variable ASSIGN_OP conditional_exp ';'
	{
		$$ = new Assignment_Ast($1, $3);
	}
;

IFELSE:
	IF '(' conditional_exp ')' GOTO_exp ELSE GOTO_exp
	{
		$$ = new IfCondition_Ast($5, $7, $3);
	}
;

GOTO_exp:
	GOTO basicblock_number ';'
	{
		$$ = new Goto_Ast($2);
	}
;

conditional_exp:
	conditional_exp OP2 conditional_exp
	{
		$$ = new Relational_Ast($1, $3, NE);
	}
|
	conditional_exp OP3 conditional_exp
	{
		$$ = new Relational_Ast($1, $3, EQ);
	}
|	
	conditional_exp OP4 conditional_exp
	{
		$$ = new Relational_Ast($1, $3, GE);
	}
|
	conditional_exp OP5 conditional_exp
	{
		$$ = new Relational_Ast($1, $3, LE);
	}
|
	conditional_exp OP6 conditional_exp
	{
		$$ = new Relational_Ast($1, $3, GT);
	}
|
	conditional_exp OP7 conditional_exp
	{
		$$ = new Relational_Ast($1, $3, LT);
	}
|
	variable
	{
		$$ = $1;
	}
|
	constant
	{
		$$ = $1;
	}
;


variable:
	NAME
	{
		
		Symbol_Table_Entry var_table_entry;

		if (current_procedure->variable_in_symbol_list_check(*$1))
			 var_table_entry = current_procedure->get_symbol_table_entry(*$1);

		else if (program_object.variable_in_symbol_list_check(*$1))
			var_table_entry = program_object.get_symbol_table_entry(*$1);

		else
		{
			int line = get_line_number();
			report_error("Variable has not been declared", line);
		}

		$$ = new Name_Ast(*$1, var_table_entry);

		delete $1;
		
	}
;

constant:
	INTEGER_NUMBER
	{
		
		$$ = new Number_Ast<int>($1, int_data_type);
		
	}
;
