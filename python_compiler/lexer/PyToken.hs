-------------------------------------------------------------------------------- 
--
-- Filename:  PyToken.hs
-- Author  :  R. Keith Morley
-- Created :  1/31/11
-- 
-- License MIT
--
-------------------------------------------------------------------------------- 

module PyToken
    (
    Token( .. ),
    renderClass 
    )
    where



data Token

    -- Whitespace
    = NEWLINE
    | INDENT    Int
    | DEDENT    Int
    | PEOF
    | ERROR     String

    -- Identifier
    | ID        String

    -- literals
    | STRING    String
    | INT       Int
    | FLOAT     Float
    | Imaginary Float

    -- KeywordS
    | FALSE
    | CLASS
    | FINALLY
    | IS
    | RETURN
    | NONE
    | CONTINUE
    | FOR
    | LAMBDA
    | TRY
    | TRUE
    | DEF
    | FROM
    | NONLOCAL
    | WHILE
    | AND
    | DEL
    | GLOBAL
    | NOT
    | WITH
    | AS
    | ELIF
    | IF
    | OR
    | YIELD
    | ASSERT
    | ELSE
    | IMPORT
    | PASS
    | BREAK
    | EXCEPT
    | IN
    | RAISE

    -- Operators
    | PLUS       -- +
    | MINUS      -- -
    | STAR       -- *
    | STARSTAR   -- **
    | DIV        -- /
    | DIVDIV     -- //
    | MOD        -- %
    | LSHIFT     -- <<
    | RSHIFT     -- >>
    | AMP        -- &
    | PIPE       -- |
    | CARET      -- ^
    | TILDE      -- ~
    | LANGLE     -- <
    | RANGLE     -- >
    | LANGLEEQ   -- <=
    | RANGLEEQ   -- >=
    | EQEQ       -- ==
    | NOTEQ      -- !=
    
    -- Delimiters
    | LPAREN     -- (
    | RPAREN     -- )
    | LBRACK     -- [
    | RBRACK     -- ]
    | LCURLY     -- {
    | RCURLY     -- }
    | COMMA      -- ,
    | COLON      -- :
    | DOT        -- .
    | SEMICOLON  -- ;
    | AT         -- @
    | PEQ         -- =
    | PLUSEQ     -- +=
    | MINUSEQ    -- -=
    | STAREQ     -- *=
    | DIVEQ      -- /=
    | DIVDIVEQ   -- //=
    | MODEQ      -- %=
    | AMPEQ      -- &=
    | PIPEEQ     -- |=
    | CARETEQ    -- ^=
    | RSHIFTEQ   -- >>=
    | LSHIFTEQ   -- <<=
    | STARSTAREQ -- **=
    deriving (Eq, Show)


renderClass :: Token -> String
renderClass  NEWLINE       = "(NEWLINE)"
renderClass  (INDENT    x) = "(INDENT)"
renderClass  (DEDENT    x) = init $ unlines (replicate x "(DEDENT)")
renderClass  PEOF          = "(ENDMARKER)"
renderClass  (ERROR     x) = "(ERROR " ++ x ++ "\")"
renderClass  (ID        x) = "(ID \""  ++ x ++ "\")"
renderClass  (STRING    x) = "(LIT \"" ++ x ++ "\")"
renderClass  (INT       x) = "(LIT " ++ show x ++ ")"
renderClass  (FLOAT     x) = "(LIT " ++ show x ++ ")"
renderClass  (Imaginary x) = "(LIT " ++ show x ++ ")"

renderClass  FALSE         = "(KEYWORD False)" 
renderClass  CLASS         = "(KEYWORD class)" 
renderClass  FINALLY       = "(KEYWORD finally)"
renderClass  IS            = "(KEYWORD is)"
renderClass  RETURN        = "(KEYWORD return)"
renderClass  NONE          = "(KEYWORD none)"
renderClass  CONTINUE      = "(KEYWORD continue)"
renderClass  FOR           = "(KEYWORD for)"
renderClass  LAMBDA        = "(KEYWORD lambda)"
renderClass  TRY           = "(KEYWORD try)"
renderClass  TRUE          = "(KEYWORD True)"
renderClass  DEF           = "(KEYWORD def)"
renderClass  FROM          = "(KEYWORD from)"
renderClass  NONLOCAL      = "(KEYWORD nonlocal)"
renderClass  WHILE         = "(KEYWORD while)"
renderClass  AND           = "(KEYWORD and)"
renderClass  DEL           = "(KEYWORD del)"
renderClass  GLOBAL        = "(KEYWORD global)"
renderClass  NOT           = "(KEYWORD not)"
renderClass  WITH          = "(KEYWORD with)"
renderClass  AS            = "(KEYWORD as)"
renderClass  ELIF          = "(KEYWORD elif)"
renderClass  IF            = "(KEYWORD if)"
renderClass  OR            = "(KEYWORD or)"
renderClass  YIELD         = "(KEYWORD yield)"
renderClass  ASSERT        = "(KEYWORD assert)"
renderClass  ELSE          = "(KEYWORD else)"
renderClass  IMPORT        = "(KEYWORD import)"
renderClass  PASS          = "(KEYWORD pass)"
renderClass  BREAK         = "(KEYWORD break)"
renderClass  EXCEPT        = "(KEYWORD except)"
renderClass  IN            = "(KEYWORD in)"
renderClass  RAISE         = "(KEYWORD raise)"

renderClass  PLUS          = "(PUNCT \"+\")"
renderClass  MINUS         = "(PUNCT \"-\")"
renderClass  STAR          = "(PUNCT \"*\")"
renderClass  STARSTAR      = "(PUNCT \"**\")"
renderClass  DIV           = "(PUNCT \"/\")"
renderClass  DIVDIV        = "(PUNCT \"//\")"
renderClass  MOD           = "(PUNCT \"%\")"
renderClass  LSHIFT        = "(PUNCT \"<<\")"
renderClass  RSHIFT        = "(PUNCT \">>\")"
renderClass  AMP           = "(PUNCT \"&\")"
renderClass  PIPE          = "(PUNCT \"|\")"
renderClass  CARET         = "(PUNCT \"^\")"
renderClass  TILDE         = "(PUNCT \"~\")"
renderClass  LANGLE        = "(PUNCT \"<\")"
renderClass  RANGLE        = "(PUNCT \">\")"
renderClass  LANGLEEQ      = "(PUNCT \"<=\")"
renderClass  RANGLEEQ      = "(PUNCT \">=\")"
renderClass  EQEQ          = "(PUNCT \"==\")"
renderClass  NOTEQ         = "(PUNCT \"!=\")"
 
renderClass  RPAREN        = "(PUNCT \")\")"
renderClass  LPAREN        = "(PUNCT \"(\")"
renderClass  LBRACK        = "(PUNCT \"[\")"
renderClass  RBRACK        = "(PUNCT \"]\")"
renderClass  LCURLY        = "(PUNCT \"{\")"
renderClass  RCURLY        = "(PUNCT \"}\")"
renderClass  COMMA         = "(PUNCT \",\")"
renderClass  COLON         = "(PUNCT \":\")"
renderClass  DOT           = "(PUNCT \".\")"
renderClass  SEMICOLON     = "(PUNCT \";\")"
renderClass  AT            = "(PUNCT \"@\")"
renderClass  PEQ           = "(PUNCT \"=\")"
renderClass  PLUSEQ        = "(PUNCT \"+=\")"
renderClass  MINUSEQ       = "(PUNCT \"-=\")"
renderClass  STAREQ        = "(PUNCT \"*=\")"
renderClass  DIVEQ         = "(PUNCT \"/=\")"
renderClass  DIVDIVEQ      = "(PUNCT \"//=\")"
renderClass  MODEQ         = "(PUNCT \"%=\")"
renderClass  AMPEQ         = "(PUNCT \"&=\")"
renderClass  PIPEEQ        = "(PUNCT \"|=\")"
renderClass  CARETEQ       = "(PUNCT \"^=\")"
renderClass  RSHIFTEQ      = "(PUNCT \">>=\")"
renderClass  LSHIFTEQ      = "(PUNCT \"<<=\")"
renderClass  STARSTAREQ    = "(PUNCT \"**=\")"
