
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

        // Code here
        auto bla = new std::list<StmNode*>();

        //match( decaf::token_type::ptRBrace );
        list_mdn->push_back( new MethodNode( type, method_name, list_params, list_vdn, bla ) );
    }
    return list_mdn;
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
