
#
# TODO: 
#  Add counter for comment nesting
#  Add FOR and WHILE states for ensuring break is within loop
# 
class TigerParser


macro
  COMMENTBEGIN  \/\*
  COMMENTEND    \*\/
  
  BLANK         [\ \t]+
  NEWLINE       \n 

  ID            [A-Za-z_][A-Za-z_0-9]*
  STRING        \"[^\"\n]*\" 
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
          
          {BLANK}                    # skip
          {NEWLINE}                  

          {INT}                      { [ :INT,         ASSymbol.new( "INT",       text.to_i, @lineno ) ] }
          {COMMA}                    { [ :COMMA,       ASSymbol.new( "COMMA",     text, @lineno ) ] }
          {COLON}                    { [ :COLON,       ASSymbol.new( "COLON",     text, @lineno ) ] }
          {SEMICOLON}                { [ :SEMICOLON,   ASSymbol.new( "SEMICOLON", text, @lineno ) ] }
          {LPAREN}                   { [ :LPAREN,      ASSymbol.new( "LPAREN",    text, @lineno ) ] }
          {RPAREN}                   { [ :RPAREN,      ASSymbol.new( "RPAREN",    text, @lineno ) ] }
          {LBRACK}                   { [ :LBRACK,      ASSymbol.new( "LBRACK",    text, @lineno ) ] }
          {RBRACK}                   { [ :RBRACK,      ASSymbol.new( "RBRACK",    text, @lineno ) ] }
          {LBRACE}                   { [ :LBRACE,      ASSymbol.new( "LBRACE",    text, @lineno ) ] }
          {RBRACE}                   { [ :RBRACE,      ASSymbol.new( "RBRACE",    text, @lineno ) ] }
          {DOT}                      { [ :DOT,         ASSymbol.new( "DOT",       text, @lineno ) ] }
          {PLUS}                     { [ :PLUS,        ASSymbol.new( "PLUS",      text, @lineno ) ] }
          {MINUS}                    { [ :MINUS,       ASSymbol.new( "MINUX",     text, @lineno ) ] }
          {TIMES}                    { [ :TIMES,       ASSymbol.new( "TIMES",     text, @lineno ) ] }
          {DIVIDE}                   { [ :DIVIDE,      ASSymbol.new( "DIVIDE",    text, @lineno ) ] }
          {EQ}                       { [ :EQ,          ASSymbol.new( "EQ",        text, @lineno ) ] }
          {NEQ}                      { [ :NEQ,         ASSymbol.new( "NEQ",       text, @lineno ) ] }
          {LT}                       { [ :LT,          ASSymbol.new( "LT",        text, @lineno ) ] }
          {LE}                       { [ :LE,          ASSymbol.new( "LE",        text, @lineno ) ] }
          {GT}                       { [ :GT,          ASSymbol.new( "GT",        text, @lineno ) ] }
          {GE}                       { [ :GE,          ASSymbol.new( "GE",        text, @lineno ) ] }
          {AND}                      { [ :AND,         ASSymbol.new( "AND",       text, @lineno ) ] }
          {OR}                       { [ :OR,          ASSymbol.new( "OR",        text, @lineno ) ] }
          {ASSIGN}                   { [ :ASSIGN,      ASSymbol.new( "ASSIGN",    text, @lineno ) ] }
          {ARRAY}                    { [ :ARRAY,       ASSymbol.new( "ARRAY",     text, @lineno ) ] }
          {IF}                       { [ :IF,          ASSymbol.new( "IF",        text, @lineno ) ] }
          {THEN}                     { [ :THEN,        ASSymbol.new( "THEN",      text, @lineno ) ] }
          {ELSE}                     { [ :ELSE,        ASSymbol.new( "ELSE",      text, @lineno ) ] }
          {WHILE}                    { [ :WHILE,       ASSymbol.new( "WHILE",     text, @lineno ) ] }
          {FOR}                      { [ :FOR,         ASSymbol.new( "FOR",       text, @lineno ) ] }
          {TO}                       { [ :TO,          ASSymbol.new( "TO",        text, @lineno ) ] }
          {DO}                       { [ :DO,          ASSymbol.new( "DO",        text, @lineno ) ] }
          {LET}                      { [ :LET,         ASSymbol.new( "LET",       text, @lineno ) ] }
          {IN}                       { [ :IN,          ASSymbol.new( "IN",        text, @lineno ) ] }
          {END}                      { [ :END,         ASSymbol.new( "END",       text, @lineno ) ] }
          {OF}                       { [ :OF,          ASSymbol.new( "OF",        text, @lineno ) ] }
          {BREAK}                    { [ :BREAK,       ASSymbol.new( "BREAD",     text, @lineno ) ] }
          {NIL}                      { [ :NIL,         ASSymbol.new( "NIL",       text, @lineno ) ] }
          {FUNCTION}                 { [ :FUNCTION,    ASSymbol.new( "FUNCTION",  text, @lineno ) ] }
          {VAR}                      { [ :VAR,         ASSymbol.new( "VAR",       text, @lineno ) ] }
          {TYPE}                     { [ :TYPE,        ASSymbol.new( "TYPE",      text, @lineno ) ] }

          
          {STRING}                   { [ :STRING,      ASSymbol.new( "STRING",    text.gsub( /\A"/m, "" ).gsub( /"\Z/m, "" ), @lineno ) ] }
          {ID}                       { [ :ID,          ASSymbol.new( "ID",        text, @lineno ) ] }

     
          .                         { puts "Could not match #{text}; }
 


inner
  
#  class Symbol
#    attr_reader :lineno
#    attr_reader :value
#    attr_reader :type
#    def initialize( type, value, lineno )
#      @type   = type
#      @value  = value
#      @lineno = lineno
#    end
#    def to_s
#      "<Symbol: #{@type}: '#{@value}' at line #{@lineno}>"
#    end
#  end

end

