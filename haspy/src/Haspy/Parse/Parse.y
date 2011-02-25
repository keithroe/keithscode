
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
decorated: decorators either( classdef, funcdef )      { makeDecorated $1 $2 }

funcdef :: { FuncDef } 
funcdef 
    : DEF ID parameters opt( right( '->', test ) ) ':' suite     { makeFuncDef $2 $3 $4 $6 }



parameters :: { Args }
    : '(' opt( typedargslist ) ')'           { makeArgs $1 }


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
    : ELSE ':' suite        { $4 }

while_stmt :: { Stmt }
    : WHILE test ':' suite opt( else )      { While $2 $4 $5 }

for_stmt :: { Stmt }
    : FOR exprlist 'in' testlist ':' suite opt( else )  { For $2 $4 $5 $6 }

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

