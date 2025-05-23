// GRAMMAR:
// #=================================================================================================================#
// get_syntax_analysis: get_statement_list_untill_eof <EOF>

// get_expression: get_logical_expression

// get_statement:
//     | get_function_initialization
//     | get_while
//     | get_cont_ret_break
//     | get_function_call
//     | get_assignment
//     | get_variable_initialization_with_assignment
//     | get_variable_initialization
//     | get_expression
//     | get_selection_statement

// get_return: <return> get_expression

// get_cont_ret_break:
//     | get_return
//     | <break>
//     | <continue>

// get_selection_statement: <if> <(> get_expression <)> get_scope {<else> get_scope}*

// get_logical_expression: get_additive_expression {<logical_op> get_additive_expression}*

// get_additive_expression: get_multiplicative_expression {<additive_op> get_multiplicative_expression}*

// get_multiplicative_expression: get_direct_declarator {<multiplicative_op> get_direct_declarator}*

// get_direct_declarator:
//     | get_additive_expression
//     | get_function_call
//     | get_primary_expression

// get_type: <type_word>

// get_variable_initialization: get_type get_variable

// get_variable_initialization_with_assignment: get_type get_assignment

// get_func_separated_init_args: {get_variable_initialization <,>}*

// get_statement_list_untill_eof: {get_statement <;>}*

// get_scope: <{> {get_statement <;>}* <}>

// get_function_initialization: get_type get_func_identificator <(> get_func_separated_init_args <)> get_scope

// get_function_call: get_func_identificator <(> {get_expression(data) <,>}* <)>

// get_assignment: get_variable <=> get_expression

// get_primary_expression:
//     | get_variable
//     | get_constant
//     | get_string_literal

// get_while: <while> <(> get_expression <)> get_scope

// get_constant: <NUM>

// get_string_literal: <STR_LIT>

// get_variable: <ID>

// get_func_identificator: <ID>
// #=================================================================================================================#
