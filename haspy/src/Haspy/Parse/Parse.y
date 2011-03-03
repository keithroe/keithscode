
{
module Haspy.Parse.Parse 
  (
  parse
  )
  where

import Haspy.Parse.Token
import Haspy.Parse.Lex
import Haspy.Parse.AST

}

%name parse
%error     { parseError }
%tokentype { Token }

%token

  -- Whitespace
  NEWLINE       { NEWLINE      }
  INDENT        { INDENT    $$ }
  DEDENT        { DEDENT    $$ }
  PEOF          { PEOF      $$ }
  ERROR         { ERROR     $$ }

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

delimListTrailing(p,q )    : delimList( p, q ) q              { $1 } 
delimListTrailingOpt(p,q ) : delimList( p, q ) opt( q )       { $1 } 

delimListLeading(p,q )     : q delimList( p, q )              { $2 } 

fst(p,q)                   : p q                              { $1 }
snd(p,q)                   : p q                              { $2 }
or(p,q)                    : p                                { $1 }                
                           | q                                { $1 }                
either(p,q)                : p                                { Left  $1 }
                           | q                                { Right $1 }

both(p,q)                  : p q                              { ($1,$2) }

oneOrMOre(p)               : reverseList(p)                   { reverse $1 }

zeroOrMOre(p)              : oneOrMOre(p)                     { $1 }
                           |                                  { [] }


--------------------------------------------------------------------------------
--
-- Grammar 
--
--------------------------------------------------------------------------------


file_input :: { Module }
file_input 
    : zeroOrMOre( either( stmt, NEWLINE ) ) PEOF {  makeModule ( concat ( lefts $1 ) ) }




decorator :: { Expr }
    : '@' dotted_name opt( parenarglist ) NEWLINE      { makeCall $1, $2, [], Nothing, Nothing  }

decorators :: { [Expr] } 
    : oneOrMOre( decorator )                           { makeDecorator $1 }

decorated :: { Expr } 
    : decorators classdef       { DecoratedClassDef $1 $2 }
    | decorators funcdef        { DecoratedFuncDef  $1 $2 }

funcdef :: { FuncDef } 
funcdef 
    : DEF ID parameters opt( right( '->', test ) ) ':' suite     { FuncDef $2 $3 $4 $6 }



parameters :: { Params }
    : '(' opt( typedargslist ) ')'           { makeParams $1 }


{--
typedargslist: 
    : ( 
        tfpdef ['=' test] (',' tfpdef ['=' test])* 
          [',' ['*' [tfpdef] (',' tfpdef ['=' test])* [',' '**' tfpdef] | '**' tfpdef] ]
             |  '*' [tfpdef] (',' tfpdef ['=' test])* [',' '**' tfpdef] | '**' tfpdef 
      )
--}
typedargslist :: {}
typedargslist 
    : positional_args non_positional_args 
    | non_positional_args

non_positional_args :: {}
non_positional_args
    : opt( snd( ',' vararg ) zeroOrMore( snd( ',' annot_arg_default) ) ) opt( snd( ',', kw_vararg )

vararg :: {}
vararg
    : snd( '*', opt( annot_arg ) )      { $1 }

kw_vararg :: {}
kw_vararg
    : snd( '**', annot_arg )     { $1 }

positional_args :: { }
positional_args
     : delimList( annot_arg_default, ',' )   { $1 }

kw_only_args :: {}
kw_only_args
     : delimList( annot_arg_default, ',' )   { $1 }

-- tfpdef
annot_arg :: { ( Ident, Maybe Expr ) }
    : ID opt( snd( ':', test ) )          { ($1, $2) }

annot_arg_default :: { Ident, Maybe Expr, Maybe Expr }
    : annot_arg opt( snd( '=', test ) )   { ( fst $1 snd $1 $2 ) }



{--  
varargslist: (vfpdef ['=' test] (',' vfpdef ['=' test])* [','
       ['*' [vfpdef] (',' vfpdef ['=' test])* [',' '**' vfpdef] | '**' vfpdef]]
     |  '*' [vfpdef] (',' vfpdef ['=' test])* [',' '**' vfpdef] | '**' vfpdef)

Should be able to refactor and share between varargslist and typedarglist
using parameterized productions ... for now, just allow annotations
--}

varargslist :: {}
varargslist 
    : typedarglist : { $1 }


stmt :: { [Stmt] }
stmt
    : simple_stmt         { $1 }
    | compound_stmt       { $1 }

simple_stmt :: { [Stmt] }
simple_stmt 
    : delimList( small_stmt, ';' ) NEWLINE  { $1 }

small_stmt :: { Stmt }
small_stmt
    : expr_stmt              { $1 }
    | del_stmt               { $1 }
    | pass_stmt              { $1 }
    | flow_stmt              { $1 }
    | import_stmt            { $1 }
    | global_stmt            { $1 }
    | nonlocal_stmt          { $1 }
    | assert_stmt            { $1 }


expr_stmt :: { Stmt }
expr_stmt
    -- testlist_star_expr (augassign (yield_expr|testlist) | ('=' (yield_expr|testlist_star_expr))*)
    : testlist_star_expr augassign either( yield_expr, testlist ) 
      { makeAugAssign $1 $2 $3 }
    | testlist_star_expr zeroOrMore( snd( '=',  either( yield_expr, testlist_star_expr ) ) )
      { makeAssign $1 $2 }

testlist_star_expr :: { Expr }
    : delimListTrailingOpt( or(test|star_expr), ',' )

augassign :: { Op }
    : '+='    { Add    }
    | '-='    { Sub    }
    | '*='    { Mult   }
    | '/='    { Div    }
    | '%='    { Mod    }
    | '&='    { Pow    }
    | '|='    { BitOr  }
    | '^='    { BitXor }
    | '<<='   { LShift }
    | '>>='   { RShift }
    | '**='   { Pow    }
    | '//='   { Floor  }

del_stmt :: { Stmt }  
del_stmt
    : DEL exprlist    { Delete $2 }


pass_stmt :: { Stmt }
pass_stmt
    : PASS   { Pass } 



flow_stmt :: { Stmt }
    : break_stmt        { $1 }
    | continue_stmt     { $1 }
    | return_stmt       { $1 }
    | raise_stmt        { $1 }
    | yield_stmt        { $1 }

break_stmt :: { Stmt }
    : BREAK { Break } 

continue_stmt :: { Stmt }
    : CONTINUE { Continute } 

return_stmt :: { Stmt }
    : RETURN opt( testlist )   { Return $2 }

yield_stmt :: { Stmt }
    : yield_expr  { $1 }

raise_stmt :: { Stmt }
    -- raise_stmt: 'raise' [test ['from' test]]
    : RAISE test_from_test  { Raise $2 }    

test_from_test :: { ( Expr, Expr ) }
    : opt( test )          { ( $1, Nothing) } 
    | test FROM test       { ( $1, $3 ) }

import_stmt :: { Stmt }
    : import_name
    --| import_from

import_name :: { Stmt }
    : IMPORT dotted_as_names     { Import $2 } 

-- # note below: the ('.' | '...') is necessary because '...' is tokenized as ELLIPSIS
--import_from: 'from' (('.' | '...')* dotted_name | ('.' | '...')+)
--              'import' ('*' | '(' import_as_names ')' | import_as_names)
--import_as_name: NAME ['as' NAME]
--import_as_names: import_as_name (',' import_as_name)* [',']

dotted_as_name :: { Alias }
    : dotted_name opt( snd( 'as', NAME ) )          { Alias $1 $2 }      

dotted_as_names: { [Alias] }
    : delimList( dotted_as_name, ',' )              { $1 }

dotted_name :: { Ident }
    : NAME                             { $1 }
    | dotted_name '.' NAME             { $1 ++ $2 ++ $3 }   -- string concat

global_stmt :: { Stmt }
    : 'global' delimList( NAME, ',' )        { Global $2 }

nonlocal_stmt :: { Stmt }
    : 'nonlocal' delimList( NAME, ',' )        { Nonlocal $2 }

assert_stmt :: { Stmt }
    : 'assert' test opt( snd( ',', test ) )    { Assert $2  $3 }

compound_stmt ::: { Stmt }
    : if_stmt        { $1 }
    | while_stmt     { $1 }
    | for_stmt       { $1 }
    | try_stmt       { $1 }
    | with_stmt      { $1 }
    | funcdef        { $1 }
    | classdef       { $1 }
    | decorated      { $1 }

if_stmt :: { Stmt }
    -- if' test ':' suite ('elif' test ':' suite)* ['else' ':' suite]
    : IF test ':' suite zeroOrMore( elif ) opt( else )   { makeIfStmt( $2, $4, $5, $6 ) }  

elif :: { ( Expr, [Stmt ] ) }
    : ELIF test ':' suite   { ( $2, $4 ) }
    
else ::  { [Statement] }
    : ELSE ':' suite        { $3 }

finally ::  { [Statement] }
    : FINALLY ':' suite        { $3 }

while_stmt :: { Stmt }
    : WHILE test ':' suite opt( else )      { While $2 $4 $5 }

for_stmt :: { Stmt }
    : FOR exprlist 'in' testlist ':' suite opt( else )  { For $2 $4 $5 $6 }

try_stmt :: { Stmt }
    -- try_stmt: ('try' ':' suite ((except_clause ':' suite)+ ['else' ':' suite] ['finally' ':' suite] | 'finally' ':' suite))
    : TRY ':' suite oneOrMOre( except_handler ) opt( else ) opt( finally )  { makeTry $2 $3 $4 $5 } 
    | TRY ':' suite finally                                                 { makeTry $2 [] Nothing $3 }
    
except_handler :: { ExceptHandler }
    : except_clause ':' suite                  { ExceptHandler (fst $1) (snd $1) $3 }
     
except_clause :: { ( Maybe Expr, Maybe Ident ) }
    | EXCEPT                                             { ( Nothing, Nothing ) }
    : EXCEPT both( test,  opt( snd( AS, NAME ) ) )     { $2 }

with_stmt :: { Stmt }
    : WITH oneOrMOre( with_item ) ':' suite       { makeWith $2 $4 }

with_item :: { ( Expr, Maybe Expr ) }
    : test opt( snd( 'as', expr ) )     { ( $1 $2 ) }

suite :: { [ Stmt ] }
    : simple_stmt                             { $1 }
    | NEWLINE INDENT oneOrMOre( stmt ) DEDENT { $3 }

-- test: or_test ['if' or_test 'else' test] | lambdef
test :: { Expr }
    : or_test opt( test_if )      { makeTest $1 $2 } 
    | lambdef                     { $1 }

test_if :: { (Expr, Expr) }
    : IF or_test ELSE test   { ( $2 $4 ) }

test_nocond :: {Expr}
    : or_test            { $1 }
    | lambdef_nocond     { $2 }

lambdef ::  { Expr }
    : LAMBDA opt(varargslist) ':' test           { Lambda $2 $4 }

lambdef_nocond :: { Expr }
    : LAMBDA opt(varargslist) ':' test_nocond    { Lambda $2 $4 }

or_test :: { Expr }
    -- and_test ('or' and_test)*
    : and_test                    { $1 }
    | or_test or_op and_test      { BinOp $1 $2 $3 }

or_op :: { Op }
   : OR        { Or }


and_test :: { Expr }
   : not_test                   { $1 }
   : and_test and_op not_test   { BinOp $1 $2 $3 }

not_test :: { Expr }
   : not_op not_test          { UnaryOp $1 $2 }
   | comparison               { $1 }

not_op { UnaryOp }
   : NOT  { Not }

comparison :: { Expr }
   : expr zeroOrMore( both( comparison, comp_op expr ) )     { makeCompare $1 $2 }

comp_op :: { CmpOp } : '<'         { Lt    } | '>'         { Gt    }
   | '=='        { Eq    }
   | '>='        { GtE   }
   | '<='        { LtE   }
   | '<>'        { NotEq }
   | '!='        { NotEq }
   | 'in'        { In    }
   | 'not' 'in'  { NotIn }
   | 'is'        { Is    }
   | 'is' 'not'  { IsNot }

star_expr :: { Expr }
   | '*' expr    { Starred $2 }

expr :: { Expr }
   : xor_expr                    { $1 } 
   | expr bit_or_op  xor_expr    { BinOp $1 $2 $3 }

bit_or_op { Op }
   : '|'       { BitOr }


xor_expr :: { Expr }
   : and_expr                          { $1 }
   : xor_expr bit_xor_op  and_expr     { BinOp $1 $2 $3 }

bit_xor_op { Op }
   : '^'       { BitXor }

and_expr :: { Expr }
   : shift_expr                        { $1 }
   | and_expr bit_and_op shift_expr    { BinOp $1 $2 $3 } 

bit_and_op { Op }
   : '$'       { BitAnd }

shift_expr :: { Expr }
   : arith_expr                        { $1 }
   | shift_expr shift_op arith_expr    { BinOp $1 $2 $3 }   

shift_op :: { Op }
   : '<<'       { LShift }
   | '>>'       { RShift }

arith_expr :: { Expr }
   : term                       { $1 }
   | arith_exp arith_op term    { BinOp $1 $2 $3 }   

arith_op :: { Op }
   : '+'       { Add }
   | '-'       { Sub }


term :: { Expr } 
   : factor                     { $1 }
   | term term_op factor        { BinOp $1 $2 $3 }     

term_op { Op }
   : '*'   { Mult     }
   | '/'   { Div      }
   | '%'   { Mod      }
   | '//'  { FloorDiv }

factor: 
   : factor_op factor  { UnaryOp $1 $2 }
   | power             { $1 }

factor_op :: { UnaryOp }
   : '+' { UAdd }
   | '-' { USub }
   | '~' { Invert }

power :: { Expr }
   : atom_trailer                         { $1 }
   | atom trailer power_op factor )       { BinOp $1 $2 }

atom_trailer :: { Expr }
   : atom zeroOrMore( trailer )           { makeAtomTrailer $1, $2 }


power_op :: { Op }
   : '**' { Pow }

atom :: { Expr }
   : '(' yield_expr_or_testlist_comp ')'   { $1 }
   | '[' [testlist_comp] ']'               { $1 }
   | '{' [dictorsetmaker] '}'              { $1 } 
   | ID                                    { Ident $1 }
   | INT                                   { Int $1 }
   | FLOAT                                 { Float $1 }
   | oneOrMOre( STRING )                   { String (concat $1 } 
   | '...'                                 { Ellipsis }
   | 'None'                                { None }
   | 'True'                                { True }
   | 'False'                               { False }

yield_expr_or_testlist_comp :: { Expr }
   :                           { Tuple [] }
   | yield_expr                { $1 } 
   | testlist_comp             { $1 }

testlist_comp :: { Expr } 
   : Either( test, star_expr ) oneOrMore( comp_for )     {  ListComp $1 $2 }
   | delimListTrailingOpt( either( test, star_expr ) )   {  List $1        }

trailer :: { Trailer }
   : '(' arglist ')'        { TrailerCall $2      }
   | '[' subscriptlist ']'    { TrailerSlice $2     }
   | '.' ID                   { TrailerAttribute $2 }

subscriptlist :: { Slice }
   : subscript                                        { $1 }
   | subscript ',' delimListTrailingOpt( subscript )  { Index ( Tuple [$1] ++ $2 ) }

subscript ::  {}
   : test                                       { Index $1 }
   | opt(test) ':' opt(test) opt( sliceop )     { Slice $1 $3 $4 } 

sliceop :: { Maybe Expr }
   : ':' opt( test )     { $2 }

exprlist :: { [ Expr ] }   
    : delimListTrailingOpt( either( expr, star_expr ), ',' )    { $1 }

testlist :: { Expr } -- Either normal expr or tuple
    :: delimListTrailingOpt( test, ',' )      { makeTestList $1 }

dictorsetmaker :: { Expr }
    : dict_item comp_for                      { DictComp (fst $1) (snd $2) $3 }  
    | delimListTrailingOpt( dict_item, ',' )  { Dict ( fst ( unzip $1 ) snd ( unzip $1 ) ) }
    | test comp_for                           { SetComp $1 $2 }
    | delimListTrailingOpt( test, ',' )       { Set $1 }

dict_item { (Expr, Expr) }
    : test ':' test           { ($1, $2) }

classdef :: { Expr }
    : 'class' ID opt( classbases )  ':' suite  { ClassDef $2 $3 $5 }

classbases :: { Maybe Args }
    : '(' opt( arglist )  ')'      { $2 }


argument_list  :: { Args }
    : posargs opt_keywords opt( snd( ",", starargs ) ) opt_keywords opt( snd( ",", kwargs ) ) 
    { Args $1 ($2 ++ $4) $3 $5 }
    |    keyword_arguments opt( snd( ",", starargs ) ) opt_keywords opt( snd( ",", kwargs ) )
    { Args [] ($1 ++ $3) $2 $4 }
    |                                         starargs opt_keywords opt( snd( ",", kwargs ) )
    { Args [] $2 $1 $3 }
    |                                                                              kwargs  
    { Args [] [] Nothing $4 }

posargs { [Expr] }
    : delimList( test, "," )        { $1 }

keywords { [Keyword] }
    : delimList( keyword, "," )     { $1 }

keyword { Keyword }
    : test '=' test                 { Keyword $1 $3 }

opt_keywords :: { [Keywords] }
    : {- empty -}                   { [] }
    | ',' keywords                  { $2 }

starargs :: { Expr }
    : '*' test                      { $2 }

kwargs :: { Expr }
    : '**' test  { $2 }  


-- The reason that keywords are test nodes instead of NAME is that using NAME
-- results in an ambiguity. ast.c makes sure it's a NAME.
-- TODO: disallowing generators for now
argument :: {  }
    : test
    | test '=' test                        -- Really [keyword '='] test

comp_for :: { Comprehension }
    : 'for' exprlist 'in' or_test zeroOrMore( comp_if ) { Comprehension $2 $4 $5 }

comp_if :: { Expr }
    :: 'if' test_nocond    { $2 }

yield_expr :: { Expr }
    : YIELD opt( testlist )   { Yield $1 }





{--
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

