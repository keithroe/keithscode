
class TigerParser


  prechigh
    nonassoc OF
    nonassoc DO
    left     UMINUS
    left     TIMES DIVIDE 
    left     PLUS MINUS
    nonassoc EQ GT GE LT LE NEQ 
    left     AND OR 
    nonassoc ASSIGN 
    left     ELSE
    nonassoc THEN
  preclow


rule
  Program           : Exp 

  Exp               : LET Decs IN ExpSeq END
                    { result = RBTiger::LetExp.new val[0].lineno, val[1] , val[3] }
                    | FOR ID ASSIGN Exp TO Exp DO Exp 
                    { result = RBTiger::ForExp.new( val[0].lineno, RBTiger::Symbol.create(val[1].value),
                                                    true, val[3], val[5], val[7] )
                    }
                    | IF Exp THEN Exp 
                    { result = RBTiger::IfExp.new val[0].lineno, val[1], val[3], nil }
                    | IF Exp THEN Exp ELSE Exp
                    { result = RBTiger::IfExp.new val[0].lineno, val[1], val[3], val[5] }
                    | WHILE Exp DO Exp 
                    { result = RBTiger::WhileExp.new val[0].lineno, val[1], val[3] }
                    | LValue ASSIGN Exp 
                    { result = RBTiger::AssignExp.new val[0].lineno, val[0], val[2] }
                    | PrimaryExp
                    | AdditiveExp
                    | MultiplicativeExp
                    | LogicalExp 
                    | RelationalExp

  PrimaryExp        : RealPrimaryExp
                    | LPAREN ExpSeq RPAREN
                    | UnaryExp =UMINUS

  RealPrimaryExp    : NIL
                    { result = RBTiger::NilExp.new  }
                    | INT 
                    { result = RBTiger::IntExp.new val[0].lineno, val[0].value }
                    | STRING
                    { result = RBTiger::StringExp.new val[0].lineno, val[0].value }
                    | ID LBRACK Exp RBRACK OF PrimaryExp 
                    { result = RBTiger::ArrayExp.new( val[0].lineno, RBTiger::Symbol.create( val[0].value ),
                                                      val[2], val[5] ) }
                    | ID LBRACE FieldInits RBRACE
                    { result = RBTiger::RecordExp.new val[0].lineno, RBTiger::Symbol.create( val[0].value ), val[2] }
                    | ID LPAREN ParamList  RPAREN
                    { result = RBTiger::CallExp.new val[0].lineno, RBTiger::Symbol.create( val[0].value ), val[2] }
                    | LValue

  Decs              : 
                    { result = [] }
                    | Decs Dec
                    { result.push val[1] }

  Dec               : VarDec
                    | TypeDecs
                    { result = RBTiger::TypeDecs.new val[0] }
                    | FuncDecs
                    { result = RBTiger::FuncDecs.new val[0] } 

  TypeDecs          : TypeDec
                    { result = [ val[0] ] }
                    | TypeDecs Typedec
                    { result.push [ val[0] ] }

  TypeDec           : TYPE ID EQ Type
                    { result = RBTiger::TypeDec.new val[0].lineno, RBTiger::Symbol.create( val[1].value ), val[3]  }

  Type              : ID
                    { result = RBTiger::NameType.new val[0].lineno, RBTiger::Symbol.create( val[0].value ) }
                    | ARRAY OF ID
                    { result = RBTiger::ArrayType.new val[0].lineno, RBTiger::Symbol.create( val[2].value ) }
                    | LBRACE TypeFields RBRACE
                    { result = RBTiger::RecordType.new val[0].lineno, val[1] }

  TypeFields        : 
                    { result = [] }
                    | TypeField 
                    { result = [ val[0] ] }
                    | TypeFields COMMA TypeField
                    { result.push val[2] }

  TypeField         : ID COLON ID
                    { result = [ RBTiger::Symbol.create( val[0].value ), RBTiger::Symbol.create( val[2].value ) ] }

  
  FieldInits        :
                    { result = [] }
                    | FieldInit 
                    { result = [ val[0] ] }
                    | FieldInits COMMA FieldInit
                    { result.push val[2] }

  FieldInit         : ID EQ Exp
                    { result = [ RBTiger::Symbol.create( val[0].value ), val[2] ] }


  FuncDecs          : FuncDec
                    { result = [ val[0] ] }
                    | FuncDecs FuncDec
                    { result.push [ val[0] ] }

  FuncDec           : FUNCTION ID LPAREN TypeFields RPAREN EQ Exp 
                    { result =  RBTiger::FuncDec.new( val[0].lineno, RBTiger::Symbol.create( val[1].value ),
                                                      val[3], nil, val[6] ) }
                    | FUNCTION ID LPAREN TypeFields RPAREN COLON ID EQ Exp 
                    { result =  RBTiger::FuncDec.new( val[0].lineno, RBTiger::Symbol.create( val[1].value ),
                                                      val[3], RBTiger::Symbol.create( val[6].value ), val[8] ) }

  VarDec            : VAR ID ASSIGN Exp
                    { result = RBTiger::VarDec.new( val[0].lineno, RBTiger::Symbol.create( val[1].value ), nil, val[3],
                                                    true ) }
                    | VAR ID COLON ID ASSIGN Exp
                    { result = RBTiger::VarDec.new( val[0].lineno, RBTiger::Symbol.create( val[1].value ),
                                                    RBTiger::Symbol.create( val[3].value ), val[5], true ) }

  ExpSeq            : 
                    { result = [] }
                    | Exp  
                    { result = [ val[0] ] }
                    | ExpSeq SEMICOLON Exp
                    { result.push val[2] }
  
  ParamList         : 
                    { result = [] }
                    | Exp
                    { result = [ val[0] ] }
                    | ParamList COMMA Exp
                    { result.push val[2] }

  LValue            : ID
                    { result = RBTiger::SimpleVar.new val[0].lineno, RBTiger::Symbol.create( val[0].value ) }
                    | FieldLValue
                    | SubscriptLValue

  FieldLValue       : LValue DOT ID
                    { result = RBTiger::RecordVar.new val[0].lineno, val[0], RBTiger::Symbol.create( val[2].value ) }

  SubscriptLValue   : ID LBRACK Exp RBRACK
                    { result = RBTiger::SubscriptVar.new val[0].lineno, RBTiger::Symbol.create( val[0].value ), val[2] }
                    | FieldLValue LBRACK Exp RBRACK
                    { result = RBTiger::SubscriptVar.new val[0].lineno, val[0], val[2] }
                    | SubscriptVar LBRACK Exp RBRACK
                    { result = RBTiger::SubscriptVar.new val[0].lineno, val[0], val[2] }

  AdditiveExp       : Exp PLUS  PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::PlusOp.new, val[2] }
                    | Exp MINUS PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::MinusOp.new, val[2] }

  MultiplicativeExp : Exp TIMES  PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::TimesOp.new, val[2] }
                    | Exp DIVIDE PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::DivideOp.new, val[2] }

  LogicalExp        : Exp AND PrimaryExp 
                    { result = RBTiger::IfExp.new( val[1].lineno, val[0], val[2],
                                                   RBTiger::IntExp.new( val[1].lineno, 0 ) ) }
                    | Exp OR  PrimaryExp 
                    { result = RBTiger::IfExp.new( val[1].lineno, val[0],
                                                   RBTiger::IntExp.new( val[1].lineno, 1 ), val[2] ) }

  RelationalExp     : Exp EQ PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::EqOp.new, val[2] }
                    | Exp NEQ PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::NeqOp.new, val[2] }
                    | Exp GT PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::GtOp.new, val[2] }
                    | Exp GE PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::GeOp.new, val[2] }
                    | Exp LT PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::LtOp.new, val[2] }
                    | Exp LE PrimaryExp
                    { result = RBTiger::OpExp.new val[1].lineno, val[0], RBTiger::LeOp.new, val[2] }

  UnaryExp          : MINUS PrimaryExp 
                    { result = RBTiger::OpExp.new( val[0].lineno, RBTiger::IntExp.new( val[0].lineno, 0 ),
                                                   RBTiger::MinusOp.new, val[2] ) }


                      

---- header ----

require File.dirname( __FILE__ ) + '/tiger_lex.rb'

---- inner ----

def on_error( t, v, values )
  raise Racc::ParseError, "<<#{@lexfilename}:#{@lexlineno}: syntax error #{v}>>"
end

---- footer ----


