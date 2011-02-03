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
    Token( .. )
    )
    where

data Token

    -- Whitespace
    = NEWLINE
    | INDENT
    | DEDENT
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
    | RPAREN     -- (
    | LPAREN     -- )
    | LBRACK     -- [
    | RBRACK     -- ]
    | LCURLY     -- {
    | RCURLY     -- }
    | COMMA      -- ,
    | COLON      -- :
    | DOT        -- .
    | SEMICOLON  -- ;
    | AT         -- @
    | EQ         -- =
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

