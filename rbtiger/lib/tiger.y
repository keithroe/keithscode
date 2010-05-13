
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
                    { result = LetExp.new val[1] , val[3] }
                    | FOR ID ASSIGN Exp TO Exp DO Exp 
                    { result = ForExp.new ASSymbol.new( val[1].value ), true, val[3], val[5], val[7] }
                    | IF Exp THEN Exp 
                    { result = IfExp.new val[1], val[3], nil }
                    | IF Exp THEN Exp ELSE Exp
                    { result = IfExp.new val[1], val[3], val[5] }
                    | WHILE Exp DO Exp 
                    { result = WhileExp.new val[1], val[3] }
                    | LValue ASSIGN Exp 
                    { result = ArrayExp.new val[0], val[2], NilExp.new }
                    | PrimaryExp
                    | AdditiveExp
                    | MultiplicativeExp
                    | LogicalExp 
                    | RelationalExp

  PrimaryExp        : RealPrimaryExp
                    | LPAREN ExpSeq RPAREN
                    | UnaryExp =UMINUS

  RealPrimaryExp    : NIL
                    { result = NilExp.new }
                    | INT 
                    { result = IntExp.new val[0].value }
                    | STRING
                    { result = StringExp.new val[0].value }
                    | ID LBRACK Exp RBRACK OF PrimaryExp 
                    { result = ArrayExp.new ASSymbol.new( val[0].value ), val[2], val[5] }
                    | ID LBRACE FieldInits RBRACE
                    { result = RecordExp.new ASSymbol.new( val[0].value ), val[2] }
                    | ID LPAREN ParamList  RPAREN
                    { result = CallExp.new ASSymbol.new( val[0].value ), val[2] }
                    | LValue


  Decs              : 
                    { result = [] }
                    | Decs Dec
                    { result.push val[1] }

  Dec               : TypeDecs
                    { result = TypeDecs.new val[0] }
                    | VarDec
                    | FuncDec

  TypeDecs          : TypeDec
                    { result = [ val[0] ] }
                    | TypeDecs Typedec
                    { result.push [ val[0] ] }

  TypeDec           : TYPE ID EQ Type
                    { result =  TypeDec.new ASSymbol.new( val[1].value ), val[3]  }

  Type              : ID
                    { result = NameType.new ASSymbol.new( val[0].value ) }
                    | ARRAY OF ID
                    { result = ArrayType.new ASSymbol.new( val[2].value ) }
                    | LBRACE TypeFields RBRACE
                    { result = RecordType.new val[1] }

  TypeFields        : 
                    { result = [] }
                    | TypeField 
                    { result = [ val[0] ] }
                    | TypeFields COMMA TypeField
                    { result.push val[2] }

  TypeField         : ID COLON ID
                    { result = [ ASSymbol.new( val[0].value ), ASSymbol.new( val[2].value ) ] }

  
  FieldInits        :
                    { result = [] }
                    | FieldInit 
                    { result = [ val[0] ] }
                    | FieldInits COMMA FieldInit
                    { result.push val[2] }

  FieldInit         : ID EQ Exp
                    { result = [ ASSymbol.new( val[0].value ), val[2] ] }


  FuncDec           : FUNCTION ID LPAREN TypeFields RPAREN EQ Exp 
                    { result = FuncDecs.new [ FuncDec.new( ASSymbol.new( val[1].value ), val[3], nil, val[6] ) ] }
                    | FUNCTION ID LPAREN TypeFields RPAREN COLON ID EQ Exp 
                    { result = FuncDecs.new [ FuncDec.new( ASSymbol.new( val[1].value ), val[3], ASSymbol.new( val[6].value ), val[8] ) ] }

  VarDec            : VAR ID ASSIGN Exp
                    { result = VarDec.new ASSymbol.new( val[1].value ), nil, val[3], true }
                    | VAR ID COLON ID ASSIGN Exp
                    { result = VarDec.new ASSymbol.new( val[1].value ), ASSymbol.new( val[3].value ), val[5], true }

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
                    { result = SimpleVar.new ASSymbol.new( val[0].value ) }
                    | FieldLValue
                    | SubscriptLValue

  FieldLValue       : LValue DOT ID
                    { result = RecordVar.new val[0], ASSymbol.new( val[2].value ) }

  SubscriptLValue   : ID LBRACK Exp RBRACK
                    { result = SubscriptVar.new ASSymbol.new( val[0].value ), val[2] }
                    | FieldLValue LBRACK Exp RBRACK
                    { result = SubscriptVar.new val[0], val[2] }
                    | SubscriptVar LBRACK Exp RBRACK
                    { result = SubscriptVar.new val[0], val[2] }

  AdditiveExp       : Exp PLUS  PrimaryExp
                    { result = OpExp.new val[0], PlusOp.new, val[2] }
                    | Exp MINUS PrimaryExp
                    { result = OpExp.new val[0], MinusOp.new, val[2] }

  MultiplicativeExp : Exp TIMES  PrimaryExp
                    { result = OpExp.new val[0], TimesOp.new, val[2] }
                    | Exp DIVIDE PrimaryExp
                    { result = OpExp.new val[0], DivideOp.new, val[2] }

  LogicalExp        : Exp AND PrimaryExp 
                    { result = IfExp.new( val[0], val[2], IntExp.new( 0 ) ) }
                    | Exp OR  PrimaryExp 
                    { result = IfExp.new( val[0], IntExp.new( 1 ), val[2] ) }

  RelationalExp     : Exp EQ PrimaryExp
                    { result = OpExp.new val[0], EqOp.new, val[2] }
                    | Exp NEQ PrimaryExp
                    { result = OpExp.new val[0], NeqOp.new, val[2] }
                    | Exp GT PrimaryExp
                    { result = OpExp.new val[0], GtOp.new, val[2] }
                    | Exp GE PrimaryExp
                    { result = OpExp.new val[0], GeOp.new, val[2] }
                    | Exp LT PrimaryExp
                    { result = OpExp.new val[0], LtOp.new, val[2] }
                    | Exp LE PrimaryExp
                    { result = OpExp.new val[0], LeOp.new, val[2] }

  UnaryExp          : MINUS PrimaryExp 
                    { result = OpExp.new( IntExp.new( 0 ), MinusOp.new, val[2] ) }


                      

---- header ----

---- inner ----

def on_error( t, v, values )
  raise Racc::ParseError, "<<#{@lexfilename}:#{@lexlineno}: syntax error #{v}>>"
end

---- footer ----


