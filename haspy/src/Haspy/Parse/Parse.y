
{

module Haspy.Parse.Parse 
  (
   parse
  )
  where

import Haspy.Parse.Token
import Haspy.Parse.Lex

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

input :: { String }
    : tokens               { $1 }
    | PEOF                 { "eof" }

tokens :: { String }
    : token                { show $1 }
    | tokens tokens        { show $2 ++ $1 }

token :: { Token }
    : DEF        { DEF }
    | PEOF       { PEOF $1 }



--expr :: { Expr }
--  : plusExpr          { $1 }
--  | listExpr          { $1 }


{

parseError :: [Token] -> a
parseError t = error $ "Parse error on: " ++ show t

}

