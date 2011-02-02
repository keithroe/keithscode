
{
module Main (main) where

import PyToken
}



%wrapper "monad"

-- character sets --------------------------------------------------------------
$shortstringcharsingle =  [^\n\\']
$shortstringchardouble =  [^\n\\"]
$longstringchar        =  [^\n]


-- macros ----------------------------------------------------------------------
@stringprefix          =  r | R
@stringescapeseq       =  "\".
@shortstringitemsingle =  $shortstringcharsingle | @stringescapeseq
@shortstringitemdouble =  $shortstringchardouble | @stringescapeseq
@longstringitem        =  $longstringchar | @stringescapeseq
@shortstring           =  "'" @shortstringitemsingle* "'" |
                          "\"" @shortstringitemdouble* "\""
@longstring            =  "'''" @longstringitem* "'''" | 
                          "\"\"\""@longstringitem* "\"\"\""
@stringliteral         =  @stringprefix? (@shortstring | @longstring)



-- tokens ----------------------------------------------------------------------

tokens :-
  $white+               ;
  "False"               { mkL FALSE       }
  "None"                { mkL NONE        }
  "True"                { mkL TRUE        }
  "and"                 { mkL AND         }
  "as"                  { mkL AS          }
  "assert"              { mkL ASSERT      }
  "break"               { mkL BREAK       }
  "class"               { mkL CLASS       }
  "continue"            { mkL CONTINUE    }
  "def"                 { mkL DEF         }
  "del"                 { mkL DEL         }
  "elif"                { mkL ELIF        }
  "else"                { mkL ELSE        }
  "except"              { mkL EXCEPT      }
  "finally"             { mkL FINALLY     }
  "for"                 { mkL FOR         }
  "from"                { mkL FROM        }
  "global"              { mkL GLOBAL      }
  "if"                  { mkL IF          }
  "import"              { mkL IMPORT      }
  "in"                  { mkL IN          }
  "is"                  { mkL IS          }
  "lambda"              { mkL LAMBDA      }
  "nonlocal"            { mkL NONLOCAL    }
  "not"                 { mkL NOT         }
  "or"                  { mkL OR          }
  "pass"                { mkL PASS        }
  "raise"               { mkL RAISE       }
  "return"              { mkL RETURN      }
  "try"                 { mkL TRY         }
  "while"               { mkL WHILE       }
  "with"                { mkL WITH        }
  "yield"               { mkL YIELD       }
  "+"                   { mkL PLUS        } 
  "-"                   { mkL MINUS       } 
  "*"                   { mkL STAR        } 
  "**"                  { mkL STARSTAR    } 
  "/"                   { mkL DIV         } 
  "//"                  { mkL DIVDIV      } 
  "%"                   { mkL MOD         } 
  "<<"                  { mkL LSHIFT      } 
  ">>"                  { mkL RSHIFT      } 
  "&"                   { mkL AMP         } 
  "|"                   { mkL PIPE        } 
  "^"                   { mkL CARET       } 
  "~"                   { mkL TILDE       } 
  "<"                   { mkL LANGLE      } 
  ">"                   { mkL RANGLE      } 
  "<="                  { mkL LANGLEEQ    } 
  ">="                  { mkL RANGLEEQ    } 
  "=="                  { mkL EQEQ        } 
  "!="                  { mkL NOTEQ       } 
  "("                   { mkL RPAREN      } 
  ")"                   { mkL LPAREN      } 
  "["                   { mkL LBRACK      } 
  "]"                   { mkL RBRACK      } 
  "{"                   { mkL LCURLY      } 
  "}"                   { mkL RCURLY      } 
  ","                   { mkL COMMA       } 
  ":"                   { mkL COLON       } 
  "."                   { mkL DOT         } 
  ";"                   { mkL SEMICOLON   } 
  "@"                   { mkL AT          } 
  "="                   { mkL PyToken.EQ  } 
  "+="                  { mkL PLUSEQ      } 
  "-="                  { mkL MINUSEQ     } 
  "*="                  { mkL STAREQ      }
  "/="                  { mkL DIVEQ       }
  "//="                 { mkL DIVDIVEQ    }
  "%="                  { mkL MODEQ       }
  "&="                  { mkL AMPEQ       }
  "|="                  { mkL PIPEEQ      }
  "^="                  { mkL CARETEQ     }
  ">>="                 { mkL RSHIFTEQ    }
  "<<="                 { mkL LSHIFTEQ    }
  "**="                 { mkL STARSTAREQ  }

{

data Lexeme = Lexeme AlexPosn PyToken.Token String


mkL :: PyToken.Token -> AlexInput -> Int -> Alex Lexeme
mkL tok (p,_,str) len = return (Lexeme p tok (take len str))


tokens :: String -> Either String [ PyToken.Token ]
tokens str = runAlex str $ do
  let loop = do tok@( Lexeme _ cl _) <- alexMonadScan;
                if cl == PEOF
                   then return [cl]
                   else do toks <- loop
                           return (cl:toks)
  loop



alexEOF :: Lexeme
alexEOF = return (Lexeme undefined PEOF "")



main = do
  s <- getContents
  print (tokens s)

}
