
module PyLexUtil where  -- Export everything for now

import PyToken

{--
data PyLexPosition = AlexPn !Int  -- absolute character offset
                            !Int  -- line number
                            !Int  -- column number

data LexState = ParseState {
    position          :: !PyLexPosition,  -- position at current input location
    input             :: !String,         -- the current input
    indent_stack      :: [Int],           -- stack of source column positions of indentation levels
    delim_stack       :: [Token],         -- stack of matched delimiters (eg, [], () )
    start_code_stack  :: [Int]            -- stack of start codes
  }
  deriving Show


--}
