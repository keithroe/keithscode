
{
module Main (main) where

import PyToken
import PyLexUtil
import Data.List 
import Control.Exception
}



%wrapper "monadUser"

-- character sets --------------------------------------------------------------
$lf                    = \n
$cr                    = \r
$eol_char              = [$lf $cr]
$not_eol_char          = ~$eol_char
$white_char            = [\ \n\r\f\v\t]
$white_no_nl           = $white_char # $eol_char

$not_single_quote      = [. \n] # '
$not_double_quote      = [. \n] # \"
$shortstringcharsingle = [^ \n \r ' \\ ]
$shortstringchardouble = [^ \n \r \" \\ ]
$longstringchar        = [. \n] # [\" ' \\]

$lowercase             = [a-z]
$uppercase             = [A-Z]

$digit                 = [0-9]
$nonzerodigit          = [1-9]
$octdigit              = [0-7]
$bindigit              = [0-1]
$hexdigit              = [$digit a-f A-F] 

-- macros ----------------------------------------------------------------------

@eol_pattern           = $lf | $cr $lf | $cr $lf  

@stringprefix          = r | R | b | B | br | Br | bR | BR -- Not handling unicode for now

@one_single_quote      = ' $not_single_quote
@two_single_quotes     = '' $not_single_quote
@one_double_quote      = \" $not_double_quote
@two_double_quotes     = \"\" $not_double_quote
@stringescapeseq       = \\ (.|@eol_pattern)
@shortstringitemsingle = $shortstringcharsingle | @stringescapeseq
@shortstringitemdouble = $shortstringchardouble | @stringescapeseq
@longstringitemdouble  = $longstringchar | '  | @stringescapeseq | @one_double_quote | @two_double_quotes
@longstringitemsingle  = $longstringchar | \" | @stringescapeseq | @one_single_quote | @two_single_quotes
@shortstring           = ' @shortstringitemsingle* ' | \" @shortstringitemdouble* \"
@longstring            = ''' @longstringitemsingle* ''' | \"\"\" @longstringitemdouble* \"\"\"
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
\\ @eol_pattern       ;
\# ($not_eol_char)*   ;

<0> {
  ()                    { begin begin_logical_line }
}

<begin_logical_line > {
  @eol_pattern          ;
  ()                    { handleIndentation `andBegin` logical_line }
}

<logical_line> {
  -- ^ $white_no_nl* @eol_pattern            ;

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
  "("                   { incDelimDepth LPAREN      } 
  ")"                   { decDelimDepth RPAREN      } 
  "["                   { incDelimDepth LBRACK      } 
  "]"                   { decDelimDepth RBRACK      } 
  "{"                   { incDelimDepth LCURLY      } 
  "}"                   { decDelimDepth RCURLY      } 
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

  @eol_pattern          { handleNewline   } 
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
mkShortString input@(posn,_,str) len = return ( Lexeme posn (STRING $ processShortString stringval ) (stringval) ) 
                                       where stringval = take len str

mkLongString :: AlexInput -> Int -> Alex Lexeme
mkLongString input@(posn,_,str) len = return ( Lexeme posn (STRING $ processLongString stringval ) (stringval) )
                                      where stringval = take len str

mkError :: AlexInput -> Int -> Alex Lexeme
mkError input@(posn,_,str) len =
    let
        posnString (AlexPn _ line col) = show line ++ ':': show col
        errorString = "Line: " ++ posnString posn ++ " before '" ++ (take len str )  ++ "'"
    in
        return (Lexeme posn (ERROR $ errorString ) (take len str) )



--------------------------------------------------------------------------------
-- UserState -- TODO: Move UserState datatype and all pure code into separate
--                    file, leaving Alex IO code here

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


dedentWhile :: Int -> [Int] -> Maybe Int 
dedentWhile col (x:xs)
    | col < x   = case (dedentWhile col xs ) of 
                      Nothing -> Nothing
                      Just i  -> Just( 1 + i )
    | col > x   = Nothing 
    | otherwise = Just 0  


popIndent :: AlexPosn -> Int -> Alex PyToken.Token 
popIndent (AlexPn _ l c) column = do currentDD <- currentDelimDepth
                                     currentIS <- currentIndentStack
                                     let num_dedents = dedentWhile column currentIS
                                     case dedentWhile column currentIS of 
                                         Nothing -> return (ERROR $ "Bad indentation at " ++ show l ++ ":" ++ show c )
                                         Just num_dedents -> do let newDD = drop num_dedents currentIS 
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
handleIndentation  input@(posn,_,[]) len = skip input len  -- EOF without newline
handleIndentation  input@(posn,_,str) len =
    do state <- alexGetUserData 
       currentInd <- currentIndent
       let currentCol = currentColumn posn
       case compare currentInd currentCol of
           EQ -> skip input len
           GT -> do token <- popIndent posn currentCol
                    mkL token input len
           LT -> do pushIndent currentCol 
                    currentIS <- currentIndentStack 
                    let size = length  currentIS
                    mkL (INDENT currentCol) input len


handleNewline :: AlexInput -> Int -> Alex Lexeme
handleNewline input len = 
    do userData  <- alexGetUserData
       currentD  <- currentDelimDepth
       if currentD > 0
           then skip input len
           else (mkL NEWLINE `andBegin` begin_logical_line) input len

handleComment :: AlexInput -> Int -> Alex Lexeme
handleComment input len = 
    do userData  <- alexGetUserData
       currentD  <- currentDelimDepth
       if currentD > 0
           then skip input len
           else (mkL NEWLINE `andBegin` begin_logical_line) input len


tokens :: String -> [ PyToken.Token ]
tokens str = toTokens $ runAlex str $ do
    let loop = do ( Lexeme _ cl _) <- alexMonadScan;
                  processToken cl
                  where
                      processToken (PEOF i) = return [(PEOF i)]
                      processToken cl = do toks <- loop
                                           return (cl:toks)
    loop
    where
        toTokens :: Either String [ PyToken.Token ] -> [ PyToken.Token ]
        toTokens (Left  x) = [ (ERROR x) ]
        toTokens (Right x) = x 


alexEOF :: Alex Lexeme 
alexEOF = do currentIS <- currentIndentStack
             return (Lexeme undefined ( PEOF $ length currentIS - 1 ) "")


printTokens :: [ PyToken.Token ] -> IO () 
printTokens x = sequence_ ( map print x )

printTokensForClass :: [ PyToken.Token ] -> IO () 
printTokensForClass xs = sequence_ ( map putStrLn (map renderClass ( take  (findError xs +1 ) xs ) ) )
                         where
                             findError []             = 0 
                             findError ((ERROR x):xs) = 0 
                             findError (_:xs)         = 1 + findError( xs ) 


main :: IO ()
main = do
  s <- getContents
  --print (tokens s)
  --map print ( uneither ( tokens s ) )
  --map print ( uneither ( tokens s ) )
  --printTokens ( tokens s )
  printTokensForClass ( tokens s )
}

