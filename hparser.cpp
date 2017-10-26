
#include "hparser.h"

using namespace std;

int HParser::parse()
{
    set_AST( program() );
    return 0;
}

ProgramNode*
HParser::program() {
    match( decaf::token_type::kwClass );
    string name = token_.lexeme;
    match( decaf::token_type::Identifier );
    match( decaf::token_type::ptLBrace );
    auto list_vdn = variable_declarations();
    auto list_mdn = method_declarations();
//    match( decaf::token_type::ptRBrace );
 //   match( decaf::token_type::EOI );
    return new ProgramNode(name, list_vdn, list_mdn);
}

list<VariableDeclarationNode*>*
HParser::variable_declarations()
{
    auto list_vdn = new list<VariableDeclarationNode*>();
    while ( token_.type == decaf::token_type::kwInt ||
            token_.type == decaf::token_type::kwReal ) {
        ValueType type = this->type();
        auto list_v = variable_list();
        list_vdn->push_back( new VariableDeclarationNode( type, list_v ) );
    }
    return list_vdn;
}


std::list<MethodNode*>*
HParser::method_declarations()
{
    auto list_mdn = new list<MethodNode*>();
    while ( token_.type == decaf::token_type::kwStatic ) {
        match( decaf::token_type::kwStatic );
        ValueType type = this->method_type();
        std::string method_name = this->method_name();
        match( decaf::token_type::ptLParen );
        auto list_params = parameters();
        match( decaf::token_type::ptRParen );
        match( decaf::token_type::ptLBrace );
        auto list_vdn = variable_declarations();
        auto list_sdn = statement_list();
        //match( decaf::token_type::ptRBrace );
        list_mdn->push_back( new MethodNode( type, method_name, list_params, list_vdn, list_sdn ) );
    }
    return list_mdn;
}

std::list<StmNode*>*
HParser::statement_list() {
    auto list_statements = new std::list<StmNode*>();
    do {
        StmNode* node = nullptr;
        if(token_.type == decaf::token_type::kwIf ) {
            match( decaf::token_type::kwIf );
            match( decaf::token_type::ptLParen );
            ExprNode* exr = expr();
            match( decaf::token_type::ptRParen );
            BlockStmNode* if_ = statement_block();
            BlockStmNode* else_ = optional_else();
            node = new IfStmNode( exr, if_, else_ );
        }
        else if(token_.type == decaf::token_type::kwFor ) {
            match( decaf::token_type::kwFor );
            match( decaf::token_type::ptLParen );
            VariableExprNode* start_var = variable();
            match( decaf::token_type::OpAssign );
            ExprNode* start_exr = expr();
            match( decaf::token_type::ptSemicolon );
            ExprNode* iter_exr = expr();
            match( decaf::token_type::ptSemicolon );
            VariableExprNode* iter_var = variable();
            IncrDecrStmNode* dir = op_incr_decr(iter_var);
            match( decaf::token_type::ptRParen );
            BlockStmNode* block = statement_block();
            node = new ForStmNode( new AssignStmNode(start_var, start_exr),
                                   iter_exr,
                                   dir, block);
        }
        else if(token_.type == decaf::token_type::kwReturn ) {
            match( decaf::token_type::kwReturn );
            node = new ReturnStmNode( optiona_expr() );
        }
        else if(token_.type == decaf::token_type::kwBreak ) {
            match( decaf::token_type::kwBreak );
            node = new BreakStmNode();
        }
        else if(token_.type == decaf::token_type::kwContinue ) {
            match( decaf::token_type::kwContinue );
            node = new ContinueStmNode();
        }
        else if(token_.type == decaf::token_type::ptLParen ) {
            node = statement_block();
        }
        else if(token_.type == decaf::token_type::Identifier ) {
            id_start_stm();
        }
        else {
            error( decaf::token_type::Identifier );
        }

        if(node != nullptr) {
            list_statements->push_back(node);
        } else {
            return list_statements;
        }
    } while (1);
    return list_statements;
}

BlockStmNode*
HParser::statement_block() {
    match( decaf::token_type::ptLParen );
    std::list<StmNode*>* slist = statement_list();
    match( decaf::token_type::ptRParen );
    return new BlockStmNode( slist );
}

BlockStmNode*
HParser::optional_else() {
    if(token_.type != decaf::token_type::kwElse)
        return nullptr;
    return statement_block();
};

StmNode*
HParser::id_start_stm() {
    StmNode* node;
    VariableExprNode* var = variable();
    if(token_.type == decaf::token_type::ptLParen) {
        match( decaf::token_type::ptLParen );
        node = new MethodCallExprStmNode( var->str(), expr_list());
        match( decaf::token_type::ptRParen );
    }
    else if(token_.type == decaf::token_type::OpAssign) {
        match( decaf::token_type::OpAssign );
        node = new AssignStmNode( var, expr() );
    }
    else if(token_.type == decaf::token_type::OpArtInc ||
            token_.type == decaf::token_type::OpArtDec) {
        node = op_incr_decr( var );
    }
    else {
        error( decaf::token_type::OpAssign );
    }
    match( decaf::token_type::ptSemicolon );
    return node;
}

IncrDecrStmNode*
HParser::op_incr_decr( VariableExprNode *var ) {
    IncrDecrStmNode* node = nullptr;
    if(token_.type == decaf::token_type::OpArtInc) {
        match( decaf::token_type::OpArtInc );
        node = new IncrStmNode(var);
    } else if(token_.type == decaf::token_type::OpArtDec) {
        match( decaf::token_type::OpArtDec );
        node = new DecrStmNode(var);
    } else {
        error( decaf::token_type::OpArtInc );
    }
    return node;
}

ExprNode*
HParser::expr() {
    // Unfinished
    match(token_.type);
    match(token_.type);
    match(token_.type);
    ExprNode* node = new MethodCallExprStmNode(token_.lexeme, new std::list<ExprNode*>());
    match(token_.type);
    return node;
}

ExprNode*
HParser::optiona_expr() {
    // Unfinished
    return nullptr;
}

std::list<ExprNode*>*
HParser::expr_list() {
    // Unfinished
    std::list<ExprNode*>* node = new std::list<ExprNode*>();
    return node;
}

std::list<ParameterNode*>*
HParser::parameters() {
    auto list_params = new std::list<ParameterNode*>();
    while ( token_.type == decaf::token_type::kwInt ||
            token_.type == decaf::token_type::kwReal ) {

        ValueType type = this->type();
        auto var = variable();
        list_params->push_back( new ParameterNode( type, var ) );
        if( token_.type == decaf::token_type::ptComma ) {
            match(  decaf::token_type::ptComma );
        }
    }
    return list_params;
}

ValueType HParser::method_type()
{
    ValueType valuetype = ValueType::VoidVal;
    if ( token_.type == decaf::token_type::kwInt ) {
        match( decaf::token_type::kwInt );
        valuetype = ValueType::IntVal;
    }
    else if ( token_.type == decaf::token_type::kwReal ) {
        match( decaf::token_type::kwReal );
        valuetype = ValueType::RealVal;
    }
    else if ( token_.type == decaf::token_type::kwVoid ) {
        match( decaf::token_type::kwVoid );
        valuetype = ValueType::VoidVal;
    } else {
        error( decaf::token_type::kwInt );
    }
    return valuetype;
}

ValueType HParser::type()
{
    ValueType valuetype = ValueType::VoidVal;
    if ( token_.type == decaf::token_type::kwInt ) {
        match( decaf::token_type::kwInt );
        valuetype = ValueType::IntVal;
    }
    else if ( token_.type == decaf::token_type::kwReal ) {
        match( decaf::token_type::kwReal );
        valuetype = ValueType::RealVal;
    }
    else {
       error( decaf::token_type::kwInt );
    }
    return valuetype;
}


list<VariableExprNode*>*
HParser::variable_list()
{
    auto list_v = new list<VariableExprNode*>();
    list_v->push_back( variable() );
    while ( token_.type == decaf::token_type::ptComma ) {
        match( decaf::token_type::ptComma );
        list_v->push_back( variable() );
    }
    match( decaf::token_type::ptSemicolon );
    return list_v;
}


VariableExprNode*
HParser::variable()
{
    auto node = new VariableExprNode(token_.lexeme);
    match( decaf::token_type::Identifier );
    return node;
}

std::string
HParser::method_name()
{
    std::string name = token_.lexeme;
    match( decaf::token_type::Identifier );
    return name;
}
