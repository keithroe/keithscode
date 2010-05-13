
#
# TODO: 
#  Add counter for comment nesting
#  Add FOR and WHILE states for ensuring break is within loop
# 
class TigerParser


macro
  COMMENTBEGIN  \/\*
  COMMENTEND    \*\/

  STRINGBEGIN   \"
  STRING        [^\"\\\n]+
  STRINGNEWLINE \\n 
  STRINGQUOTE   \\\" 
  STRINGEND     \"
  
  BLANK         [\ \t]+
  NEWLINE       \n 

  ID            [A-Za-z_][A-Za-z_0-9]*
  INT           [0-9]+
  COMMA         \,
  COLON         \:
  SEMICOLON     \;
  LPAREN        \(
  RPAREN        \)
  LBRACK        \[
  RBRACK        \]
  LBRACE        \{
  RBRACE        \}
  DOT           \.
  PLUS          \+
  MINUS         \-
  TIMES         \*
  DIVIDE        \/
  EQ            \=
  NEQ           \<\>
  LT            \<
  LE            \<\=
  GT            \>
  GE            \>\=
  AND           \&
  OR            \|
  ASSIGN        \:\=
  ARRAY         array 
  IF            if
  THEN          then
  ELSE          else
  WHILE         while
  FOR           for
  TO            to
  DO            do
  LET           let
  IN            in
  END           end
  OF            of
  BREAK         break
  NIL           nil
  FUNCTION      function
  VAR           var
  TYPE          type



rule
          {COMMENTBEGIN}             { state = :COMMENT;   nil;  }
 :COMMENT {COMMENTEND}               { state = nil;        nil;  }
 :COMMENT  [^\*]+                    {                     nil;  } 
 :COMMENT  \*[^\/]                   {                     nil;  } 
          
          {STRINGBEGIN}              { state = :STRING; @stringval =  "";     nil;  }
 :STRING  {STRINGEND}                { state = nil;     [ :STRING, Token.new( "STRING", @stringval, @lineno ) ]  }
 :STRING  {STRING}                   {                  @stringval += text;   nil;  } 
 :STRING  {STRINGNEWLINE}            {                  @stringval += "\n";   nil;  } 
 :STRING  {STRINGQUOTE}              {                  @stringval += '"';    nil;  } 

          
          
          {BLANK}                    # skip
          {NEWLINE}                  

          {INT}                      { [ :INT,         Token.new( "INT",       text.to_i, @lineno ) ] }
          {COMMA}                    { [ :COMMA,       Token.new( "COMMA",     text, @lineno ) ] }
          {COLON}                    { [ :COLON,       Token.new( "COLON",     text, @lineno ) ] }
          {SEMICOLON}                { [ :SEMICOLON,   Token.new( "SEMICOLON", text, @lineno ) ] }
          {LPAREN}                   { [ :LPAREN,      Token.new( "LPAREN",    text, @lineno ) ] }
          {RPAREN}                   { [ :RPAREN,      Token.new( "RPAREN",    text, @lineno ) ] }
          {LBRACK}                   { [ :LBRACK,      Token.new( "LBRACK",    text, @lineno ) ] }
          {RBRACK}                   { [ :RBRACK,      Token.new( "RBRACK",    text, @lineno ) ] }
          {LBRACE}                   { [ :LBRACE,      Token.new( "LBRACE",    text, @lineno ) ] }
          {RBRACE}                   { [ :RBRACE,      Token.new( "RBRACE",    text, @lineno ) ] }
          {DOT}                      { [ :DOT,         Token.new( "DOT",       text, @lineno ) ] }
          {PLUS}                     { [ :PLUS,        Token.new( "PLUS",      text, @lineno ) ] }
          {MINUS}                    { [ :MINUS,       Token.new( "MINUX",     text, @lineno ) ] }
          {TIMES}                    { [ :TIMES,       Token.new( "TIMES",     text, @lineno ) ] }
          {DIVIDE}                   { [ :DIVIDE,      Token.new( "DIVIDE",    text, @lineno ) ] }
          {EQ}                       { [ :EQ,          Token.new( "EQ",        text, @lineno ) ] }
          {NEQ}                      { [ :NEQ,         Token.new( "NEQ",       text, @lineno ) ] }
          {LT}                       { [ :LT,          Token.new( "LT",        text, @lineno ) ] }
          {LE}                       { [ :LE,          Token.new( "LE",        text, @lineno ) ] }
          {GT}                       { [ :GT,          Token.new( "GT",        text, @lineno ) ] }
          {GE}                       { [ :GE,          Token.new( "GE",        text, @lineno ) ] }
          {AND}                      { [ :AND,         Token.new( "AND",       text, @lineno ) ] }
          {OR}                       { [ :OR,          Token.new( "OR",        text, @lineno ) ] }
          {ASSIGN}                   { [ :ASSIGN,      Token.new( "ASSIGN",    text, @lineno ) ] }
          {ARRAY}                    { [ :ARRAY,       Token.new( "ARRAY",     text, @lineno ) ] }
          {IF}                       { [ :IF,          Token.new( "IF",        text, @lineno ) ] }
          {THEN}                     { [ :THEN,        Token.new( "THEN",      text, @lineno ) ] }
          {ELSE}                     { [ :ELSE,        Token.new( "ELSE",      text, @lineno ) ] }
          {WHILE}                    { [ :WHILE,       Token.new( "WHILE",     text, @lineno ) ] }
          {FOR}                      { [ :FOR,         Token.new( "FOR",       text, @lineno ) ] }
          {TO}                       { [ :TO,          Token.new( "TO",        text, @lineno ) ] }
          {DO}                       { [ :DO,          Token.new( "DO",        text, @lineno ) ] }
          {LET}                      { [ :LET,         Token.new( "LET",       text, @lineno ) ] }
          {IN}                       { [ :IN,          Token.new( "IN",        text, @lineno ) ] }
          {END}                      { [ :END,         Token.new( "END",       text, @lineno ) ] }
          {OF}                       { [ :OF,          Token.new( "OF",        text, @lineno ) ] }
          {BREAK}                    { [ :BREAK,       Token.new( "BREAD",     text, @lineno ) ] }
          {NIL}                      { [ :NIL,         Token.new( "NIL",       text, @lineno ) ] }
          {FUNCTION}                 { [ :FUNCTION,    Token.new( "FUNCTION",  text, @lineno ) ] }
          {VAR}                      { [ :VAR,         Token.new( "VAR",       text, @lineno ) ] }
          {TYPE}                     { [ :TYPE,        Token.new( "TYPE",      text, @lineno ) ] }

          
          {ID}                       { [ :ID,          Token.new( "ID",        text, @lineno ) ] }

     
          .                         { puts "Could not match #{text}; }
 


inner
  
  class Token 
    attr_reader :lineno
    attr_reader :value
    attr_reader :type
    def initialize( type, value, lineno )
      @type   = type
      @value  = value
      @lineno = lineno
    end
    def to_s
      "<Token: #{@type}: '#{@value}' at line #{@lineno}>"
    end
  end
end

