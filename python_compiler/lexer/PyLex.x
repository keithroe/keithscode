
{
module Main (main) where

import PyToken
import PyLexUtil
}



%wrapper "monadUser"

-- character sets --------------------------------------------------------------
$lf                    = \n
$cr                    = \r
$eol_char              = [$lf $cr]
$not_eol_char          = ~$eol_char
$white_char            = [\ \n\r\f\v\t]
$white_no_nl           = $white_char # $eol_char

$shortstringcharsingle = [^\n\\']
$shortstringchardouble = [^\n\\"]
$longstringchar        = [^\n]

$lowercase             = [a-z]
$uppercase             = [A-Z]

$digit                 = [0-9]
$nonzerodigit          = [1-9]
$octdigit              = [0-7]
$bindigit              = [0-1]
$hexdigit              = [$digit a-f A-F] 

-- macros ----------------------------------------------------------------------

@eol_pattern           = $lf | $cr $lf | $cr $lf  

@stringprefix          = r | R
@stringescapeseq       = \\.
@shortstringitemsingle = $shortstringcharsingle | @stringescapeseq
@shortstringitemdouble = $shortstringchardouble | @stringescapeseq
@longstringitem        = $longstringchar | @stringescapeseq
@shortstring           = ' @shortstringitemsingle* ' | \" @shortstringitemdouble* \"
@longstring            = ''' @longstringitem* ''' | \"\"\" @longstringitem* \"\"\"
@shortstringliteral    = @stringprefix? @shortstring
@longstringliteral     = @stringprefix? @longstring

@letter                = $lowercase | $uppercase
@identifier            = (@letter|_) (@letter | $digit | _)*

@decimalinteger        = $nonzerodigit $digit* | 0+
@octinteger            = 0 (o | O) $octdigit+
@hexinteger            = 0 (x | X) $hexdigit+
@bininteger            = 0 (b | B) $bindigit+
@integer               = @decimalinteger | @octinteger | @hexinteger | @bininteger

@intpart               = $digit+
@fraction              = \. $digit+
@exponent              = (e | E) (\+ | \-)? $digit+
@pointfloat            = (@intpart? @fraction) | @intpart \. 
@exponentfloat         = (@intpart | @pointfloat) @exponent
@floatnumber           = @pointfloat | @exponentfloat
@imagnumber            = (@floatnumber | @intpart) (j | J)



-- tokens ----------------------------------------------------------------------

tokens :-

$white_no_nl+         ;
\# ($not_eol_char)*   ; 
\\ @eol_pattern       ;

<0> {
  ()                    { begin begin_logical_line }
}

<begin_logical_line > {
  @eol_pattern          ;
  ()                    { handleIndentation `andBegin` logical_line }
}

<logical_line> {
  ^ $white_no_nl* @eol_pattern ;
  
  @eol_pattern          { mkL NEWLINE `andBegin` begin_logical_line }

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
  "("                   { mkL LPAREN      } 
  ")"                   { mkL RPAREN      } 
  "["                   { mkL LBRACK      } 
  "]"                   { mkL RBRACK      } 
  "{"                   { mkL LCURLY      } 
  "}"                   { mkL RCURLY      } 
  ","                   { mkL COMMA       } 
  ":"                   { mkL COLON       } 
  "."                   { mkL DOT         } 
  ";"                   { mkL SEMICOLON   } 
  "@"                   { mkL AT          } 
  "="                   { mkL PEQ         } 
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
  @identifier           { mkId            }
  @integer              { mkInt           }
  @floatnumber          { mkFloat         }
  @imagnumber           { mkImag          }
  @longstringliteral    { mkLongString    }
  @shortstringliteral   { mkShortString   }
  .                     { mkError         }
  
}

{
--------------------------------------------------------------------------------
--
-- User types
--
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
-- Lexeme

data Lexeme = Lexeme AlexPosn PyToken.Token String

mkL :: PyToken.Token -> AlexInput -> Int -> Alex Lexeme
mkL tok (posn,_,str) len = return (Lexeme posn tok (take len str))

mkId :: AlexInput -> Int -> Alex Lexeme
mkId input@(posn,_,str) len = return ( Lexeme posn (ID stringval) (stringval) ) 
                              where stringval = take len str

mkInt :: AlexInput -> Int -> Alex Lexeme
mkInt input@(posn,_,str) len = return ( Lexeme posn (INT $ stringToInt stringval) (stringval) ) 
                              where stringval = take len str

mkFloat :: AlexInput -> Int -> Alex Lexeme
mkFloat input@(posn,_,str) len = return ( Lexeme posn (FLOAT $ stringToDouble stringval) (stringval) ) 
                                 where stringval = take len str

mkImag :: AlexInput -> Int -> Alex Lexeme
mkImag input@(posn,_,str) len = return ( Lexeme posn (IMAG $ stringToDouble (init stringval ) ) (stringval) ) 
                                where stringval = take len str

mkShortString :: AlexInput -> Int -> Alex Lexeme
mkShortString input@(posn,_,str) len = return ( Lexeme posn (STRING ( init $ tail $ stringval) ) (stringval) ) 
                                       where stringval = take len str

mkLongString :: AlexInput -> Int -> Alex Lexeme
mkLongString input@(posn,_,str) len = return ( Lexeme posn (STRING ( drop 3 ( take (len-3) stringval ) ) ) (stringval) )
                                      where stringval = take len str

mkError :: AlexInput -> Int -> Alex Lexeme
mkError input@(posn,_,str) len =
    let
        posnString (AlexPn _ line col) = show line ++ ':': show col
        errorString = "Line: " ++ posnString posn ++ " before '" ++ (take len str )  ++ "'"
    in
        return (Lexeme posn (ERROR $ errorString ) (take len str) )

mkXError :: AlexInput -> Int -> Alex Lexeme
mkXError input@(posn,_,str) len =
    let
        posnString (AlexPn _ line col) = show line ++ ':': show col
        errorString = "X Line: " ++ posnString posn ++ " before '" ++ (take len str )  ++ "'"
    in
        return (Lexeme posn (ERROR $ errorString ) (take len str) )


--------------------------------------------------------------------------------
-- UserState

data UserState = UserState {
    delimDepth    :: Int,      -- Current depth of opening delims (eg, '(', '[' ) 
    indentStack   :: [Int]     -- Stack of indentation levels (in spaces)
} deriving( Show )


userStartState :: UserState  
userStartState = UserState 0 [1]


currentIndent :: Alex Int
currentIndent = 
    do userData <- alexGetUserData
       return $ head $ indentStack userData


pushIndent :: Int -> Alex ()
pushIndent x = do currentDD <- currentDelimDepth
                  currentIS <- currentIndentStack
                  alexSetUserData ( UserState currentDD (x:currentIS) )


dedentWhile :: Int -> [Int] -> Int 
dedentWhile col dedent_stack = dedentWhile' col dedent_stack 
    where dedentWhile' col (x:xs)
              | col < x   = 1 + dedentWhile' col xs 
              | otherwise = 0  


popIndent :: Int -> Alex PyToken.Token 
popIndent column = do currentDD <- currentDelimDepth
                      currentIS <- currentIndentStack
                      let num_dedents = dedentWhile column currentIS
                      let newDD = drop num_dedents currentIS 
                      alexSetUserData ( UserState currentDD (newDD) )
                      return (DEDENT num_dedents)


currentIndentStack :: Alex [ Int ]
currentIndentStack = 
    do userData <- alexGetUserData
       return ( indentStack userData )


currentDelimDepth :: Alex Int
currentDelimDepth = 
    do userData <- alexGetUserData
       return ( delimDepth userData )


decDelimDepth :: PyToken.Token -> AlexInput -> Int -> Alex Lexeme
decDelimDepth token input len= 
    do userData  <- alexGetUserData
       currentIS <- currentIndentStack
       currentD  <- currentDelimDepth
       alexSetUserData ( UserState (currentD - 1) currentIS )
       mkL token input len
       

incDelimDepth :: PyToken.Token -> AlexInput -> Int -> Alex Lexeme
incDelimDepth token input len= 
    do userData  <- alexGetUserData
       currentIS <- currentIndentStack
       currentD  <- currentDelimDepth
       alexSetUserData ( UserState (currentD + 1) currentIS )
       mkL token input len


currentColumn :: AlexPosn -> Int
currentColumn (AlexPn _ _ col) = col 


handleIndentation :: AlexInput -> Int -> Alex Lexeme
handleIndentation  input@(posn,_,str) len =
    do state <- alexGetUserData 
       currentInd <- currentIndent
       let currentCol = currentColumn posn
       case compare currentInd currentCol of
           EQ -> skip input len
           GT -> do token <- popIndent currentCol
                    mkL token input len
           LT -> do pushIndent currentCol 
                    currentIS <- currentIndentStack 
                    let size = length  currentIS
                    mkL (INDENT currentCol) input len


tokens :: String -> Either String [ PyToken.Token ]
tokens str = runAlex str $ do
  let loop = do tok@( Lexeme _ cl _) <- alexMonadScan;
                if cl == PEOF
                   then return [cl]
                   else do toks <- loop
                           return (cl:toks)
  loop


alexEOF :: (Monad m) => m Lexeme 
alexEOF = return (Lexeme undefined PEOF "")


printTokens :: Either String [ PyToken.Token ] -> IO () 
printTokens (Left  x ) = print x
printTokens (Right x ) = sequence_ ( map print x )

printTokensForClass :: Either String [ PyToken.Token ] -> IO () 
printTokensForClass (Left  x ) = print x
printTokensForClass (Right x ) = sequence_ ( map (putStrLn. renderClass) x )


main :: IO ()
main = do
  s <- getContents
  --print (tokens s)
  --map print ( uneither ( tokens s ) )
  --map print ( uneither ( tokens s ) )
  --printTokens ( tokens s )
  printTokensForClass ( tokens s )
}

