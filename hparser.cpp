
#include "hparser.h"

using namespace std;

//#define DEBUG_EXPR 1

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
    match( decaf::token_type::ptRBrace );
    match( decaf::token_type::EOI );
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
        match( decaf::token_type::ptRBrace );
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
            node = new ReturnStmNode( optional_expr() );
            match( decaf::token_type::ptSemicolon );
        }
        else if(token_.type == decaf::token_type::kwBreak ) {
            match( decaf::token_type::kwBreak );
            node = new BreakStmNode();
            match( decaf::token_type::ptSemicolon );
        }
        else if(token_.type == decaf::token_type::kwContinue ) {
            match( decaf::token_type::kwContinue );
            node = new ContinueStmNode();
            match( decaf::token_type::ptSemicolon );
        }
        else if(token_.type == decaf::token_type::ptLParen ) {
            node = statement_block();
        }
        else if(token_.type == decaf::token_type::Identifier ) {
            node = id_start_stm();
        }

        if(node != nullptr) {
            list_statements->push_back(node);
        } else {
            return list_statements;
        }
    } while (1);
}

BlockStmNode*
HParser::statement_block() {
    match( decaf::token_type::ptLBrace );
    std::list<StmNode*>* slist = statement_list();
    match( decaf::token_type::ptRBrace );
    return new BlockStmNode( slist );
}

BlockStmNode*
HParser::optional_else() {
    if(token_.type != decaf::token_type::kwElse)
        return nullptr;
    match( decaf::token_type::kwElse );
    return statement_block();
};

StmNode*
HParser::id_start_stm() {
    StmNode* node;
    std::string lex = token_.lexeme;
    match( decaf::token_type::Identifier );
    if(token_.type == decaf::token_type::ptLParen) {
        match( decaf::token_type::ptLParen );
        node = new MethodCallExprStmNode( lex, expr_list());
        match( decaf::token_type::ptRParen );
    }
    else if(token_.type == decaf::token_type::OpAssign) {
        match( decaf::token_type::OpAssign );
        node = new AssignStmNode( new VariableExprNode(lex), expr() );
    }
    else if(token_.type == decaf::token_type::OpArtInc ||
            token_.type == decaf::token_type::OpArtDec) {
        node = op_incr_decr( new VariableExprNode(lex) );
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
    }
    else if(token_.type == decaf::token_type::OpArtDec) {
        match( decaf::token_type::OpArtDec );
        node = new DecrStmNode(var);
    }
    else {
        error( decaf::token_type::OpArtInc );
    }
    return node;
}

ExprNode*
HParser::expr() {
#ifdef DEBUG_EXPR
    std::cout << "expr" << std::endl;
#endif
    return expr_delta( expr_and() );
}

ExprNode*
HParser::expr_delta(ExprNode* other) {
    if(token_.type != decaf::token_type::OpLogOr)
        return other;
#ifdef DEBUG_EXPR
    std::cout << "expr delta" << std::endl;
#endif
    match( decaf::token_type::OpLogOr );
    return expr_delta( new OrExprNode(other, expr_and()) );
}

ExprNode*
HParser::expr_and() {
#ifdef DEBUG_EXPR
    std::cout << "and" << std::endl;
#endif
    return expr_and_delta( expr_eq() );
}

ExprNode*
HParser::expr_and_delta(ExprNode* other) {
    if(token_.type != decaf::token_type::OpLogAnd)
        return other;
#ifdef DEBUG_EXPR
    std::cout << "and delta" << std::endl;
#endif
    match( decaf::token_type::OpLogAnd );
    return expr_and_delta( new AndExprNode(other, expr_eq()) );
}

ExprNode*
HParser::expr_eq() {
#ifdef DEBUG_EXPR
    std::cout << "eq" << std::endl;
#endif
    return expr_eq_delta( expr_rel() );
}

ExprNode*
HParser::op_eq(ExprNode *lhs) {
    ExprNode* node = nullptr;
    if(token_.type == decaf::token_type::OpRelEQ) {
        match( decaf::token_type::OpRelEQ );
        ExprNode *rhs = expr_rel();
        node = new EqExprNode(lhs, rhs);
    }
    else if(token_.type == decaf::token_type::OpRelNEQ) {
        match( decaf::token_type::OpRelNEQ );
        ExprNode *rhs = expr_rel();
        node = new NeqExprNode(lhs, rhs);
    }
    else {
        error(decaf::token_type::OpRelEQ);
    }
    return node;
}

ExprNode*
HParser::expr_eq_delta(ExprNode* other) {
    if(token_.type != decaf::token_type::OpRelEQ &&
       token_.type != decaf::token_type::OpRelNEQ) {
        return other;
    }
#ifdef DEBUG_EXPR
    std::cout << "eq delta" << std::endl;
#endif
    return expr_eq_delta( op_eq(other) );
}

ExprNode*
HParser::expr_rel() {
#ifdef DEBUG_EXPR
    std::cout << "rel" << std::endl;
#endif
    return expr_rel_delta( expr_add() );
}

ExprNode*
HParser::op_rel(ExprNode *lhs) {
    ExprNode* node = nullptr;
    if(token_.type == decaf::token_type::OpRelGT) {
        match( decaf::token_type::OpRelGT );
        ExprNode *rhs = expr_add();
        node = new GtExprNode(lhs, rhs);
    }
    else if(token_.type == decaf::token_type::OpRelGTE) {
        match( decaf::token_type::OpRelGTE );
        ExprNode *rhs = expr_add();
        node = new GteExprNode(lhs, rhs);
    }
    else if(token_.type == decaf::token_type::OpRelLT) {
        match( decaf::token_type::OpRelLT );
        ExprNode *rhs = expr_add();
        node = new LtExprNode(lhs, rhs);
    }
    else if(token_.type == decaf::token_type::OpRelLTE) {
        match( decaf::token_type::OpRelLTE );
        ExprNode *rhs = expr_add();
        node = new LteExprNode(lhs, rhs);
    }
    else {
        error(decaf::token_type::OpRelGT);
    }
    return node;
}

ExprNode*
HParser::expr_rel_delta(ExprNode* other) {
    if(token_.type != decaf::token_type::OpRelGT &&
       token_.type != decaf::token_type::OpRelGTE &&
       token_.type != decaf::token_type::OpRelLT &&
       token_.type != decaf::token_type::OpRelLTE) {
        return other;
    }
#ifdef DEBUG_EXPR
    std::cout << "rel delta" << std::endl;
#endif
    return expr_rel_delta( op_rel(other) );
}

ExprNode*
HParser::expr_add() {
#ifdef DEBUG_EXPR
    std::cout << "add" << std::endl;
#endif
    return expr_add_delta( expr_mult() );
}

ExprNode*
HParser::op_add(ExprNode *lhs) {
    ExprNode* node = nullptr;
    if(token_.type == decaf::token_type::OpArtPlus) {
        match( decaf::token_type::OpArtPlus );
        ExprNode *rhs = expr_mult();
        node = new PlusExprNode(lhs, rhs);
    }
    else if(token_.type == decaf::token_type::OpArtMinus) {
        match( decaf::token_type::OpArtMinus );
        ExprNode *rhs = expr_mult();
        node = new MinusExprNode(lhs, rhs);
    }
    else {
        error(decaf::token_type::OpArtPlus);
    }
    return node;
}

ExprNode*
HParser::expr_add_delta(ExprNode* other) {
    if(token_.type != decaf::token_type::OpArtPlus &&
       token_.type != decaf::token_type::OpArtMinus) {
        return other;
    }
#ifdef DEBUG_EXPR
    std::cout << "add delta" << std::endl;
#endif
    return expr_add_delta( op_add(other) );
}

ExprNode*
HParser::expr_mult() {
#ifdef DEBUG_EXPR
    std::cout << "mult" << std::endl;
#endif
    return expr_mult_delta( expr_unary() );
}

ExprNode*
HParser::op_mul(ExprNode *lhs) {
    ExprNode* node = nullptr;
    if(token_.type == decaf::token_type::OpArtMult) {
        match( decaf::token_type::OpArtMult );
        ExprNode *rhs = expr_unary();
        node = new MultiplyExprNode(lhs, rhs);
    }
    else if(token_.type == decaf::token_type::OpArtDiv) {
        match( decaf::token_type::OpArtDiv );
        ExprNode *rhs = expr_unary();
        node = new DivideExprNode(lhs, rhs);
    }
    else if(token_.type == decaf::token_type::OpArtModulus) {
        match( decaf::token_type::OpArtModulus );
        ExprNode *rhs = expr_unary();
        node = new ModulusExprNode(lhs, rhs);
    }
    else {
        error(decaf::token_type::OpArtMult);
    }
    return node;
}

ExprNode*
HParser::expr_mult_delta(ExprNode* other) {
    if(token_.type != decaf::token_type::OpArtMult &&
       token_.type != decaf::token_type::OpArtDiv &&
       token_.type != decaf::token_type::OpArtModulus) {
        return other;
    }
#ifdef DEBUG_EXPR
    std::cout << "mult delta" << std::endl;
#endif
    return expr_mult_delta( op_mul(other) );
}

ExprNode*
HParser::op_unary() {
    ExprNode* node = nullptr;
    if(token_.type == decaf::token_type::OpArtPlus) {
        match( decaf::token_type::OpArtPlus );
        ExprNode *rhs = expr_unary();
        node = new PlusExprNode(nullptr, rhs);
    }
    else if(token_.type == decaf::token_type::OpArtMinus) {
        match( decaf::token_type::OpArtMinus );
        ExprNode *rhs = expr_unary();
        node = new MinusExprNode(nullptr, rhs);
    }
    else if(token_.type == decaf::token_type::OpLogNot) {
        match( decaf::token_type::OpLogNot );
        ExprNode *rhs = expr_unary();
        node = new NotExprNode(rhs);
    }
    else {
        error(decaf::token_type::OpArtPlus);
    }
    return node;
}

ExprNode*
HParser::expr_unary() {
    if(token_.type == decaf::token_type::OpArtPlus ||
       token_.type == decaf::token_type::OpArtMinus ||
       token_.type == decaf::token_type::OpLogNot) {
        return op_unary();
    }
#ifdef DEBUG_EXPR
    std::cout << "unary" << std::endl;
#endif
    return factor();
}

ExprNode*
HParser::optional_expr() {
    if(token_.type == decaf::token_type::ptSemicolon)
        return nullptr;
    return expr();
}

void
HParser::more_expr(std::list<ExprNode*>* list) {
    if(token_.type != decaf::token_type::ptComma)
        return;
    match(decaf::token_type::ptComma);
    list->push_back( expr() );
    more_expr(list);
}

std::list<ExprNode*>*
HParser::expr_list() {
    if(token_.type == decaf::token_type::ptRParen)
        return nullptr;
    auto exr_list = new std::list<ExprNode*>();
    exr_list->push_back( expr() );
    more_expr(exr_list);
    return exr_list;
}

ExprNode*
HParser::factor() {
#ifdef DEBUG_EXPR
    std::cout << "factor" << std::endl;
#endif
    ExprNode* node = nullptr;
    if(token_.type == decaf::token_type::Number) {
        node = new NumberExprNode(token_.lexeme);
        match( decaf::token_type::Number );
    }
    else if(token_.type == decaf::token_type::ptLParen) {
        match( decaf::token_type::ptLParen );
        node = expr();
        match( decaf::token_type::ptRParen );
    }
    else if(token_.type == decaf::token_type::Identifier) {
        std::string func_name = token_.lexeme;
        match( decaf::token_type::Identifier );
        if(token_.type == decaf::token_type::ptLParen) {
            match( decaf::token_type::ptLParen );
            node = new MethodCallExprStmNode( func_name, expr_list() );
            match( decaf::token_type::ptRParen );
        } else {
            node = new VariableExprNode(func_name);
        }
    } else {
        error( decaf::token_type::Identifier );
    }
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
