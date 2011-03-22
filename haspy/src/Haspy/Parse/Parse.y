
{
module Haspy.Parse.Parse 
  (
  parse
  )
  where

import Haspy.Parse.Token
import Haspy.Parse.Lex
import Haspy.Parse.AST as AST

import Data.Either 

}

%name parse file_input
%error     { parseError }
%tokentype { Token }

%token

  -- Whitespace
  NEWLINE       { NEWLINE      }
  INDENT        { INDENT    $$ }
  DEDENT        { DEDENT    $$ }
  PEOF          { PEOF      $$ }

  -- Identifier
  ID            { ID        $$ }

  -- literals
  STRING        { STRING    $$ }
  INT           { INT       $$ }
  FLOAT         { FLOAT     $$ }
  IMAG          { IMAG      $$ }

  -- KeywordS
  FALSE         { FALSE        }
  CLASS         { CLASS        }
  FINALLY       { FINALLY      }
  IS            { IS           }
  RETURN        { RETURN       }
  NONE          { NONE         }
  CONTINUE      { CONTINUE     }
  FOR           { FOR          }
  LAMBDA        { LAMBDA       }
  TRY           { TRY          }
  TRUE          { TRUE         }
  DEF           { DEF          }
  FROM          { FROM         }
  NONLOCAL      { NONLOCAL     }
  WHILE         { WHILE        }
  AND           { AND          }
  DEL           { DEL          }
  GLOBAL        { GLOBAL       }
  NOT           { NOT          }
  WITH          { WITH         }
  AS            { AS           }
  ELIF          { ELIF         }
  IF            { IF           }
  OR            { OR           }
  YIELD         { YIELD        }
  ASSERT        { ASSERT       }
  ELSE          { ELSE         }
  IMPORT        { IMPORT       }
  PASS          { PASS         }
  BREAK         { BREAK        }
  EXCEPT        { EXCEPT       }
  IN            { IN           }
  RAISE         { RAISE        }

  -- Operators
  '+'           { PLUS         }
  '-'           { MINUS        }
  '*'           { STAR         }
  '**'          { STARSTAR     }
  '/'           { DIV          }
  '//'          { DIVDIV       }
  '%'           { MOD          }
  '<<'          { LSHIFT       }
  '>>'          { RSHIFT       }
  '&'           { AMP          }
  '|'           { PIPE         }
  '^'           { CARET        }
  '~'           { TILDE        }
  '<'           { LANGLE       }
  '>'           { RANGLE       }
  '<='          { LANGLEEQ     }
  '>='          { RANGLEEQ     }
  '=='          { EQEQ         }
  '!='          { NOTEQ        }
  
  --     Delimiters
  '('           { LPAREN       }
  ')'           { RPAREN       }
  '['           { LBRACK       }
  ']'           { RBRACK       }
  '{'           { LCURLY       }
  '}'           { RCURLY       }
  ','           { COMMA        }
  ':'           { COLON        }
  '.'           { DOT          }
  '...'         { ELLIPSIS     }
  '->'          { RARROW       }
  ';'           { SEMICOLON    }
  '@'           { AT           }
  '='           { PEQ          }
  '+='          { PLUSEQ       }
  '-='          { MINUSEQ      }
  '*='          { STAREQ       }
  '/='          { DIVEQ        }
  '//='         { DIVDIVEQ     }
  '%='          { MODEQ        }
  '&='          { AMPEQ        }
  '|='          { PIPEEQ       }
  '^='          { CARETEQ      }
  '>>='         { RSHIFTEQ     }
  '<<='         { LSHIFTEQ     }
  '**='         { STARSTAREQ   }

%%

--------------------------------------------------------------------------------
--
-- Parameterized productions
--
--------------------------------------------------------------------------------
opt(p)                     : p                                { Just $1 }
                           |                                  { Nothing }


reverseList(p)             : p                                { [$1] }
                           | reverseList(p) p                 { $2 : $1 }

reverseDelimList(p,q )     : p                                { [$1] }
                           | reverseDelimList( p, q ) q p     { $3 : $1 }

delimList(p,q )            : reverseDelimList( p, q )         { reverse $1 } 

delimListTrailingOpt(p,q ) : reverseDelimList( p, q )         { reverse $1 } 
                           | reverseDelimList( p, q ) q       { reverse $1 } 

fst(p,q)                   : p q                              { $1 }
snd(p,q)                   : p q                              { $2 }
or(p,q)                    : p                                { $1 }                
                           | q                                { $1 }                
either(p,q)                : p                                { Left  $1 }
                           | q                                { Right $1 }

both(p,q)                  : p q                              { ($1,$2) }

oneOrMore(p)               : reverseList(p)                   { reverse $1 }

zeroOrMore(p)              : oneOrMore(p)                     { $1 }
                           |                                  { [] }


--------------------------------------------------------------------------------
--
-- Grammar 
--
--------------------------------------------------------------------------------



file_input :: { Module }
file_input 
    : zeroOrMore( either( stmt, NEWLINE ) ) PEOF
      {  Module ( concat (lefts $1) ) }

NAME :: { Ident }
NAME 
   : ID                                          { Ident $1 }

decorator :: { Decorator }
decorator
    : '@' dotted_name              NEWLINE      { Decorator $2 [] }
    | '@' dotted_name parenarglist NEWLINE      { Decorator $2 $3 }

parenarglist :: { [Argument] }
parenarglist
    : '('  argument_list ')'                     { $2 }

decorators :: { [Decorator] } 
decorators
    : oneOrMore( decorator )                     { $1 }

decorated :: { Stmt } 
decorated
    : decorators classdef                        { DecoratedClassDef $1 $2 }
    | decorators funcdef                         { DecoratedFuncDef  $1 $2 }

funcdef :: { Stmt } 
funcdef 
    : DEF NAME parameters opt( snd( '->', test ) ) ':' suite
      { FuncDef $2 $3 $4 $6 }

parameters :: { [Param] }
parameters
    : '(' typedargslist ')'           { $2 }

-- Used for FuncDefs and Lambdas
-- typedargslist: ( 
--   tfpdef ['=' test] (',' tfpdef ['=' test])* 
--     [',' ['*' [tfpdef] (',' tfpdef ['=' test])* [',' '**' tfpdef] | '**' tfpdef] ]
--        |  '*' [tfpdef] (',' tfpdef ['=' test])* [',' '**' tfpdef] | '**' tfpdef 
--   )

typedargslist :: { [ Param ] }
typedargslist 
     : delimListTrailingOpt( param, ',' )        { $1 }

param :: { Param }
    : arg_default                                { $1 }
    | vararg                                     { $1 }
    | kw_vararg                                  { $1 }

arg_default :: { Param }
arg_default
    : annot_arg opt( snd( '=', test ) )          { Param (fst $1) (snd $1) $2 } 

vararg :: { Param }
vararg
    : '*'
      { VarParam Nothing Nothing   }
    | '*' annot_arg
      { VarParam ( Just (fst $2) ) (snd $2) }

kw_vararg :: { Param }
kw_vararg
    : '**'                                       
    { KeywordVarParam Nothing Nothing   }

    | '**' annot_arg                             
    { KeywordVarParam ( Just (fst $2) ) (snd $2) }

-- tfpdef
annot_arg :: { ( Ident, Maybe Expr ) }
annot_arg
    : NAME opt( snd( ':', test ) )        { ($1, $2) }


-- varargslist: (vfpdef ['=' test] (',' vfpdef ['=' test])* [','
--      ['*' [vfpdef] (',' vfpdef ['=' test])* [',' '**' vfpdef] | '**' vfpdef]]
--    |  '*' [vfpdef] (',' vfpdef ['=' test])* [',' '**' vfpdef] | '**' vfpdef)
--
-- Should be able to refactor and share between varargslist and typedargslist
-- using parameterized productions ... for now, Just allow annotations
varargslist :: { [Param] }
varargslist 
    : typedargslist                              { $1 }


stmt :: { [Stmt] }
stmt
    : simple_stmt                                { $1   }
    | compound_stmt                              { [$1] }

simple_stmt :: { [Stmt] }
simple_stmt 
    : delimList( small_stmt, ';' ) NEWLINE       { $1 }

small_stmt :: { Stmt }
small_stmt
    : expr_stmt                                  { $1 }
    | del_stmt                                   { $1 }
    | pass_stmt                                  { $1 }
    | flow_stmt                                  { $1 }
    | import_stmt                                { $1 }
    | global_stmt                                { $1 }
    | nonlocal_stmt                              { $1 }
    | assert_stmt                                { $1 }

expr_stmt :: { Stmt }
expr_stmt
    -- testlist (augassign (yield_expr|testlist) | ('=' (yield_expr|testlist))*)
    : testlist
      { Expression $1 }

    | testlist augassign_op yield_expr_or_testlist
      { AugAssign $1 $2 $3 }

    | testlist '=' delimList( testlist, '=' )
      { Assign  ($1:(init $3)) (last $3)  }

yield_expr_or_testlist :: { Expr }
    : yield_expr                                 { $1 }
    | testlist                                   { $1 }

augassign_op :: { Op }
augassign_op
    : '+='                                       { Add    }
    | '-='                                       { Sub    }
    | '*='                                       { Mult   }
    | '/='                                       { Div    }
    | '%='                                       { Mod    }
    | '&='                                       { BitAnd }
    | '|='                                       { BitOr  }
    | '^='                                       { BitXor }
    | '<<='                                      { LShift }
    | '>>='                                      { RShift }
    | '**='                                      { Pow    }
    | '//='                                      { FloorDiv  }

del_stmt :: { Stmt }  
del_stmt
    : DEL exprlist                               { Delete $2 }

pass_stmt :: { Stmt }
pass_stmt
    : PASS   { Pass } 

flow_stmt :: { Stmt }
flow_stmt
    : break_stmt                                 { $1 }
    | continue_stmt                              { $1 }
    | return_stmt                                { $1 }
    | raise_stmt                                 { $1 }
    | yield_stmt                                 { $1 }

break_stmt :: { Stmt }
break_stmt
    : BREAK                                      { Break } 

continue_stmt :: { Stmt }
continue_stmt
    : CONTINUE                                   { Continue } 

return_stmt :: { Stmt }
return_stmt
    : RETURN opt( testlist )                     { Return $2 }

yield_stmt :: { Stmt }
yield_stmt
    : yield_expr                                 { Expression $1 }

raise_stmt :: { Stmt }
raise_stmt
    -- raise_stmt: 'raise' [test ['from' test]]
    : RAISE test_from_test                       { Raise (fst $2) (snd $2) }    

test_from_test :: { ( Maybe Expr, Maybe Expr ) }
test_from_test
    : opt( test )                                { ( $1, Nothing ) } 
    | test FROM test                             { ( Just $1, Just $3 ) }

import_stmt :: { Stmt }
import_stmt
    : import_name  { $1 } 
    --| import_from    -- TODO: add this later

import_name :: { Stmt }
import_name
    : IMPORT dotted_as_names                     { Import $2 } 

-- # note: the ('.' | '...') is necessary because '...' is tokenized as ELLIPSIS
-- import_from: 'from' (('.' | '...')* dotted_name | ('.' | '...')+)
--              'import' ('*' | '(' import_as_names ')' | import_as_names)
-- import_as_name: ID ['as' ID]
-- import_as_names: import_as_name (',' import_as_name)* [',']

dotted_as_name :: { Alias }
dotted_as_name
    : dotted_name opt( snd( AS, NAME ) )         { Alias $1 $2 }      

dotted_as_names :: { [Alias] }
dotted_as_names
    : delimList( dotted_as_name, ',' )           { $1 }

dotted_name :: { Ident }
dotted_name
    : NAME                              
      { $1 }

    | dotted_name '.' NAME           
      { Ident ( (identName $1)  ++ "." ++ (identName $3) ) }

global_stmt :: { Stmt }
global_stmt
    : GLOBAL delimList( NAME, ',' )              { Global $2 }

nonlocal_stmt :: { Stmt }
nonlocal_stmt
    : NONLOCAL delimList( NAME, ',' )            { Nonlocal $2 }

assert_stmt :: { Stmt }
assert_stmt
    : ASSERT test opt( snd( ',', test ) )        { Assert $2  $3 }

compound_stmt :: { Stmt }
compound_stmt
    : if_stmt                                    { $1 }
    | while_stmt                                 { $1 }
    | for_stmt                                   { $1 }
    | try_stmt                                   { $1 }
    | with_stmt                                  { $1 }
    | funcdef                                    { $1 }
    | classdef                                   { $1 }
    | decorated                                  { $1 }

if_stmt :: { Stmt }
    -- if' test ':' suite ('elif' test ':' suite)* ['else' ':' suite]
    : IF test ':' suite elif_else                { If $2 $4 $5 }  
   
elif_else :: { [Stmt] }
elif_else
   :                                             { [] }
   | elif elif_else                              { [ If (fst $1) (snd $1) $2 ] }
   | else                                        { $1 }

elif :: { ( Expr, [Stmt ] ) }
elif
    : ELIF test ':' suite                        { ( $2, $4 ) }
    
else :: { [Stmt] }
else
    : ELSE ':' suite                             { $3 }

while_stmt :: { Stmt }
while_stmt
    : WHILE test ':' suite opt_else              { While $2 $4 $5 }

for_stmt :: { Stmt }
for_stmt
    : FOR exprlist IN testlist ':' suite opt_else
      { For $2 $4 $6 $7 }

opt_else :: { [Stmt] }
opt_else 
    :                                            { [] }
    | else                                       { $1 }

try_stmt :: { Stmt }
try_stmt
    -- try_stmt:   ('try' ':' suite
    --           ( (except_clause ':' suite)+ ['else' ':' suite] 
    --             ['finally' ':' suite] | 'finally' ':' suite ) )
    : TRY ':' suite zeroOrMore( except_handler ) try_else try_finally
      { Try $3 $4 $5 $6 } 

try_else :: { [Stmt] }
try_else
    :                                            { [] }
    | ELSE ':' suite                             { $3 }

try_finally ::  { [Stmt] }
try_finally
    :                                            { [] }
    | FINALLY ':' suite                          { $3 }

except_handler :: { ExceptHandler }
except_handler
    : except_clause ':' suite
      { ExceptHandler (fst $1) (snd $1) $3 }
     
except_clause :: { ( Maybe Expr, Maybe Ident ) }
except_clause
    : EXCEPT                                     { ( Nothing, Nothing ) }
    | EXCEPT both( test, opt( snd(AS, NAME) ) )  { (Just (fst $2), snd $2) }

with_stmt :: { Stmt }
with_stmt
    : WITH delimList(with_item, ',') ':' suite   { makeWith $2 $4 }

with_item :: { ( Expr, Maybe Expr ) }
with_item
    : test opt( snd( AS, expr ) )                { ( $1, $2 ) }

suite :: { [ Stmt ] }
suite
    : simple_stmt                                { $1 }
    | NEWLINE INDENT oneOrMore( stmt ) DEDENT    { concat $3 }

test :: { Expr }
test
    -- test: or_test ['if' or_test 'else' test] | lambdef
    : or_test opt( test_if )                     { makeTest $1 $2 } 
    | lambdef                                    { $1 }

test_if :: { (Expr, Expr) }
test_if
    : IF or_test ELSE test                       { ( $2, $4 ) }

test_nocond :: {Expr}
test_nocond
    : or_test                                    { $1 }
    | lambdef_nocond                             { $1 }

lambdef :: { Expr }
lambdef
    : LAMBDA varargslist ':' test                { Lambda $2 $4 }

lambdef_nocond :: { Expr }
lambdef_nocond
    : LAMBDA varargslist ':' test_nocond         { Lambda $2 $4 }

or_test :: { Expr }
or_test
    -- and_test ('or' and_test)*
    : and_test                                   { $1 }
    | or_test or_op and_test                     { BinOp $1 $2 $3 }

or_op :: { Op }
or_op
   : OR                                          { Or }

and_test :: { Expr }
and_test
   : not_test                                    { $1 }
   | and_test and_op not_test                    { BinOp $1 $2 $3 }

and_op :: { Op }
and_op
   : AND { And }

not_test :: { Expr }
not_test
   : not_op not_test                             { UnaryOp $1 $2 }
   | comparison                                  { $1 }

not_op :: { UnaryOp }
not_op
   : NOT  { Not }

comparison :: { Expr }
comparison
   -- comparison: expr (comp_op expr)*
   : expr zeroOrMore( both( comp_op, expr ) )    { makeCompare $1 $2 }

comp_op :: { CmpOp } 
comp_op
   : '<'                                         { Lt    }
   | '>'                                         { Gt    }
   | '=='                                        { Eq    }
   | '>='                                        { GtE   }
   | '<='                                        { LtE   }
   | '!='                                        { NotEq }
   | IN                                          { In    }
   | NOT IN                                      { NotIn }
   | IS                                          { Is    }
   | IS NOT                                      { IsNot }

star_expr :: { Expr }
star_expr
   : '*' expr                                    { Starred $2 }

expr :: { Expr }
expr
   : xor_expr                                    { $1 } 
   | expr bit_or_op  xor_expr                    { BinOp $1 $2 $3 }

bit_or_op :: { Op }
bit_or_op
   : '|'                                         { BitOr }

xor_expr :: { Expr }
xor_expr
   : and_expr                                    { $1 }
   | xor_expr bit_xor_op  and_expr               { BinOp $1 $2 $3 }

bit_xor_op :: { Op }
bit_xor_op
   : '^'                                         { BitXor }

and_expr :: { Expr }
and_expr
   : shift_expr                                  { $1 }
   | and_expr bit_and_op shift_expr              { BinOp $1 $2 $3 } 

bit_and_op :: { Op }
bit_and_op
   : '&'       { BitAnd }

shift_expr :: { Expr }
shift_expr
   : arith_expr                                  { $1 }
   | shift_expr shift_op arith_expr              { BinOp $1 $2 $3 }   

shift_op :: { Op }
shift_op
   : '<<'                                        { LShift }
   | '>>'                                        { RShift }

arith_expr :: { Expr }
arith_expr
   : term                                        { $1 }
   | arith_expr arith_op term                    { BinOp $1 $2 $3 }   

arith_op :: { Op }
arith_op
    : '+'                                        { Add }
    | '-'                                        { Sub }

term :: { Expr } 
term
    : factor                                     { $1 }
    | term term_op factor                        { BinOp $1 $2 $3 }     

term_op :: { Op }
term_op
    : '*'                                        { Mult     }
    | '/'                                        { Div      }
    | '%'                                        { Mod      }
    | '//'                                       { FloorDiv }

factor :: { Expr } 
factor
    : factor_op factor                           { UnaryOp $1 $2 }
    | power                                      { $1 }

factor_op :: { UnaryOp }
factor_op
    : '+'                                        { UAdd }
    | '-'                                        { USub }
    | '~'                                        { Invert }

power :: { Expr }
power
    : atom_trailer                               { $1 }
    | atom_trailer power_op factor               { BinOp $1 $2 $3 }

atom_trailer :: { Expr }
    : atom zeroOrMore( trailer )                 { makeAtomTrailer $1 $2 }

power_op :: { Op }
power_op
    : '**'                                       { Pow }

atom :: { Expr }
atom
    : '(' tuplemaker ')'                         { $2           }
    | '[' testlist_comp ']'                      { $2           }
    | '{' dictorsetmaker '}'                     { $2           } 
    | NAME                                       { Name      $1 }
    | INT                                        { AST.Int   $1 }
    | FLOAT                                      { AST.Float $1 }
    | IMAG                                       { AST.Imag  $1 }
    | oneOrMore( STRING )                        { AST.Str   (concat $1) } 
    | '...'                                      { Ellipsis     }
    | NONE                                       { AST.NoneVal  }
    | TRUE                                       { AST.TrueVal  }
    | FALSE                                      { AST.FalseVal }

tuplemaker :: { Expr }
tuplemaker
    : {- empty -}                                { Tuple [] }
    | delimListTrailingOpt( test, ',' )          { Tuple $1 }

testlist_comp :: { Expr } 
testlist_comp
    : {- empty -}
      {  List [] }
    | test_or_star_expr comp_for
      {  ListComp $1 $2 }
    | delimListTrailingOpt(test_or_star_expr, ',')
      {  List $1        }

test_or_star_expr :: { Expr }
test_or_star_expr 
    : test                                       { $1 }
    | star_expr                                  { $1 }


trailer :: { Trailer }
trailer
    : '(' argument_list ')'                      { TrailerCall      $2 }
    | '[' subscriptlist ']'                      { TrailerSubscript $2 }
    | '.' NAME                                   { TrailerAttribute $2 }

subscriptlist :: { Slice }
subscriptlist
    : subscript                                  { $1 }
    | testlist                                   { Index $1 }

subscript ::  { Slice }
subscript
    : opt(test) ':' opt(test) sliceop            { Slice $1 $3 $4 } 

sliceop :: { Maybe Expr }
sliceop
    : {- empty -}                                { Nothing } 
    | ':' opt( test )                            { $2 }

exprlist :: { [ Expr ] }   
exprlist
    : delimListTrailingOpt( expr_or_star_expr, ',' )
      { $1 }

expr_or_star_expr :: { Expr }
expr_or_star_expr
    : expr                                       { $1 }
    | star_expr                                  { $1 }


testlist :: { Expr } -- Either normal expr or tuple
testlist
    : delimListTrailingOpt( test, ',' )          { makeTestList $1 }

dictorsetmaker :: { Expr }
dictorsetmaker
    : -- empty                                   
      { Dict [] []                                     }

    | delimListTrailingOpt( dict_item, ',' )
      { Dict ( fst ( unzip $1 ) ) ( snd ( unzip $1 ) ) }

    | dict_item comp_for
      { DictComp (fst $1) (snd $1) $2                  }

    | delimListTrailingOpt( test, ',' )
      { Set $1                                         }

    | test comp_for
      { SetComp $1 $2                                  }

dict_item :: { (Expr, Expr) }
dict_item
    : test ':' test                              { ($1, $3) }

classdef :: { Stmt }
classdef
  : CLASS NAME classbases ':' suite              { ClassDef $2 $3 $5 }

classbases :: { [Argument] }
classbases
    :                                            { [] }
    | '(' argument_list ')'                      { $2 }


-- arglist: ( argument  ',')* 
--          ( argument [',']                            |
--            '*'  test (',' argument)* [',' '**' test] |
--            '**' test)

argument_list :: { [ Argument ] }
argument_list 
    :                                            { [] }
    | delimListTrailingOpt( argument, ',' )      { $1 }

argument :: { Argument }
    : positional                                 { $1 }
    | keyword                                    { $1 }
    | starargs                                   { $1 }
    | kwargs                                     { $1 }
 
positional :: { Argument }
    : test                                       { Positional $1 }

keyword :: { Argument }
keyword
    : NAME '=' test                              { Keyword $1 $3 }

starargs :: { Argument }
starargs
    : '*' test                                   { StarArgs $2 }

kwargs :: { Argument }
kwargs
    : '**' test                                  { KWArgs $2 }  


-- The reason that keywords are test nodes instead of ID is that using ID
-- results in an ambiguity. ast.c makes sure it's a ID.
-- TODO: disallowing generators for now

comp_for :: { Comprehension }
comp_for
    : FOR exprlist IN or_test zeroOrMore( comp_if ) 
      { Comprehension (makeTestList $2) $4 $5 }

comp_if :: { Expr }
comp_if
    : IF test_nocond                             { $2 }

yield_expr :: { Expr }
yield_expr
    : YIELD opt( testlist )                      { Yield $2 }


{-
    single_input: NEWLINE | simple_stmt | compound_stmt NEWLINE
    file_input: (NEWLINE | stmt)* ENDMARKER
    eval_input: testlist NEWLINE* ENDMARKER

    decorator: '@' dotted_name [ '(' [arglist] ')' ] NEWLINE
    decorators: decorator+
    decorated: decorators (classdef | funcdef)
    funcdef: 'def' NAME parameters ['->' test] ':' suite
    parameters: '(' [typedargslist] ')'
    typedargslist: (tfpdef ['=' test] (',' tfpdef ['=' test])* [','
           ['*' [tfpdef] (',' tfpdef ['=' test])* [',' '**' tfpdef] | '**' tfpdef]]
         |  '*' [tfpdef] (',' tfpdef ['=' test])* [',' '**' tfpdef] | '**' tfpdef)
    tfpdef: NAME [':' test]
    varargslist: (vfpdef ['=' test] (',' vfpdef ['=' test])* [','
           ['*' [vfpdef] (',' vfpdef ['=' test])* [',' '**' vfpdef] | '**' vfpdef]]
         |  '*' [vfpdef] (',' vfpdef ['=' test])* [',' '**' vfpdef] | '**' vfpdef)
    vfpdef: NAME

    stmt: simple_stmt | compound_stmt
    simple_stmt: small_stmt (';' small_stmt)* [';'] NEWLINE
    small_stmt: (expr_stmt | del_stmt | pass_stmt | flow_stmt |
                 import_stmt | global_stmt | nonlocal_stmt | assert_stmt)
    expr_stmt: testlist_star_expr (augassign (yield_expr|testlist) |
                         ('=' (yield_expr|testlist_star_expr))*)
    testlist_star_expr: (test|star_expr) (',' (test|star_expr))* [',']
    augassign: ('+=' | '-=' | '*=' | '/=' | '%=' | '&=' | '|=' | '^=' |
                '<<=' | '>>=' | '**=' | '//=')
    # For normal assignments, additional restrictions enforced by the interpreter
    del_stmt: 'del' exprlist
    pass_stmt: 'pass'
    flow_stmt: break_stmt | continue_stmt | return_stmt | raise_stmt | yield_stmt
    break_stmt: 'break'
    continue_stmt: 'continue'
    return_stmt: 'return' [testlist]
    yield_stmt: yield_expr
    raise_stmt: 'raise' [test ['from' test]]
    import_stmt: import_name | import_from
    import_name: 'import' dotted_as_names
    # note below: the ('.' | '...') is necessary because '...' is tokenized as ELLIPSIS
    import_from: ('from' (('.' | '...')* dotted_name | ('.' | '...')+)
                  'import' ('*' | '(' import_as_names ')' | import_as_names))
    import_as_name: NAME ['as' NAME]
    dotted_as_name: dotted_name ['as' NAME]
    import_as_names: import_as_name (',' import_as_name)* [',']
    dotted_as_names: dotted_as_name (',' dotted_as_name)*
    dotted_name: NAME ('.' NAME)*
    global_stmt: 'global' NAME (',' NAME)*
    nonlocal_stmt: 'nonlocal' NAME (',' NAME)*
    assert_stmt: 'assert' test [',' test]

    compound_stmt: if_stmt | while_stmt | for_stmt | try_stmt | with_stmt | funcdef | classdef | decorated
    if_stmt: 'if' test ':' suite ('elif' test ':' suite)* ['else' ':' suite]
    while_stmt: 'while' test ':' suite ['else' ':' suite]
    for_stmt: 'for' exprlist 'in' testlist ':' suite ['else' ':' suite]
    try_stmt: ('try' ':' suite
               ((except_clause ':' suite)+
                ['else' ':' suite]
                ['finally' ':' suite] |
               'finally' ':' suite))
    with_stmt: 'with' with_item (',' with_item)*  ':' suite
    with_item: test ['as' expr]
    # NB compile.c makes sure that the default except clause is last
    except_clause: 'except' [test ['as' NAME]]
    suite: simple_stmt | NEWLINE INDENT stmt+ DEDENT

    test: or_test ['if' or_test 'else' test] | lambdef
    test_nocond: or_test | lambdef_nocond
    lambdef: 'lambda' [varargslist] ':' test
    lambdef_nocond: 'lambda' [varargslist] ':' test_nocond
    or_test: and_test ('or' and_test)*
    and_test: not_test ('and' not_test)*
    not_test: 'not' not_test | comparison
    comparison: expr (comp_op expr)*
    comp_op: '<'|'>'|'=='|'>='|'<='|'<>'|'!='|'in'|'not' 'in'|'is'|'is' 'not'
    star_expr: '*' expr
    expr: xor_expr ('|' xor_expr)*
    xor_expr: and_expr ('^' and_expr)*
    and_expr: shift_expr ('&' shift_expr)*
    shift_expr: arith_expr (('<<'|'>>') arith_expr)*
    arith_expr: term (('+'|'-') term)*
    term: factor (('*'|'/'|'%'|'//') factor)*
    factor: ('+'|'-'|'~') factor | power
    power: atom trailer* ['**' factor]
    atom: ('(' [yield_expr|testlist_comp] ')' |
           '[' [testlist_comp] ']' |
           '{' [dictorsetmaker] '}' |
           NAME | NUMBER | STRING+ | '...' | 'None' | 'True' | 'False')
    testlist_comp: (test|star_expr) ( comp_for | (',' (test|star_expr))* [','] )
    trailer: '(' [arglist] ')' | '[' subscriptlist ']' | '.' NAME
    subscriptlist: subscript (',' subscript)* [',']
    subscript: test | [test] ':' [test] [sliceop]
    sliceop: ':' [test]
    exprlist: (expr|star_expr) (',' (expr|star_expr))* [',']
    testlist: test (',' test)* [',']
    dictorsetmaker: ( (test ':' test (comp_for | (',' test ':' test)* [','])) |
                      (test (comp_for | (',' test)* [','])) )

    classdef: 'class' NAME ['(' [arglist] ')'] ':' suite

    arglist: (argument ',')* (argument [',']
                             |'*' test (',' argument)* [',' '**' test] 
                             |'**' test)
    # The reason that keywords are test nodes instead of NAME is that using NAME
    # results in an ambiguity. ast.c makes sure it's a NAME.
    argument: test [comp_for] | test '=' test  # Really [keyword '='] test
    comp_iter: comp_for | comp_if
    comp_for: 'for' exprlist 'in' or_test [comp_iter]
    comp_if: 'if' test_nocond [comp_iter]

    # not used in grammar, but may appear in "node" passed from Parser to Compiler
    encoding_decl: NAME

    yield_expr: 'yield' [testlist]

--}


{

parseError :: [Token] -> a
parseError t = error $ "Parse error on: " ++ show t

}

