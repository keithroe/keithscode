
module PyLexUtil where  -- Export everything for now

import Numeric (readInt, readHex, readOct)
import Data.Char (digitToInt, isDigit)


stringToInt :: String -> Int
stringToInt str@(x:[]) = read str
stringToInt str@(x0:x1:xs)
    | x0         /= '0'      = read str
    | x1 == 'x' || x1 == 'X' = read str
    | x1 == 'o' || x1 == 'O' = read str
    | x1 == 'b' || x1 == 'B' = readBin xs
    | x1 == '0'              = 0 
    | otherwise              = error $ "failed to convert '" ++ str  ++ "'"
    where 
        readBin :: String -> Int
        readBin str = fst $ Numeric.readInt 2 (`elem` "01") Data.Char.digitToInt str !! 0
       

stringToDouble :: String -> Double
stringToDouble str@('.':_) = read ('0':stringToDoubleRest str)
stringToDouble str         = read (stringToDoubleRest str)

stringToDoubleRest :: String -> String
stringToDoubleRest []     = []
stringToDoubleRest ['.']  = ".0"
stringToDoubleRest (x:xs) = x : stringToDoubleRest xs



strsub :: Eq a => [a] -> [a] -> [a] -> [a]
strsub [] _ _ = []
strsub s find repl =
    if take (length find) s == find
        then repl ++ (strsub(drop (length find) s) find repl)
        else [head s] ++ (strsub (tail s) find repl)


{--
unescapeString :: String -> String
unescapeString xs = xs''''''' 
    where
    xs'         = strsub xs       "\\\\"   "\\"
    xs''        = strsub xs'      "\\\""   "\""
    xs'''       = strsub xs''     "\\'"    "'"
    xs''''      = strsub xs'''    "\\n"    "\n"
    xs'''''     = strsub xs''''   "\\\r\n" ""
    xs''''''    = strsub xs'''''  "\\\r"   ""
    xs'''''''   = strsub xs'''''' "\\\n"   ""
--}

unescapeString :: String -> String
unescapeString ( '\\':'\\':xs )      = '\\' : unescapeString xs
unescapeString ( '\\':'\'':xs )      = '\'' : unescapeString xs
unescapeString ( '\\':'"':xs )       = '"'  : unescapeString xs
unescapeString ( '\\':'b':xs )       = '\b' : unescapeString xs
unescapeString ( '\\':'f':xs )       = '\f' : unescapeString xs
unescapeString ( '\\':'t':xs )       = '\t' : unescapeString xs
unescapeString ( '\\':'a':xs )       = '\a' : unescapeString xs
unescapeString ( '\\':'n':xs )       = '\n' : unescapeString xs
unescapeString ( '\\':'\r':'\n':xs ) =        unescapeString xs
unescapeString ( '\\':'\n':xs )      =        unescapeString xs
unescapeString ( '\\':'\r':xs )      =        unescapeString xs
unescapeString ('\\':rest@(o:_))
   | o `elem` octalDigits = unescapeNumeric 3 octalDigits (fst . head . readOct) rest
unescapeString ('\\':'x':rest@(h:_))
   | h `elem` hexDigits = unescapeNumeric 2 hexDigits (fst . head . readHex) rest
unescapeString ( x:xs )              = x    : unescapeString xs 
unescapeString []                    = [] 

escapeString :: String -> String
escapeString ('\\' : xs) = '\\':'\\':escapeString xs
escapeString ('\'' : xs) = '\\':'\'':escapeString xs
escapeString ('"'  : xs) = '\\':'"' :escapeString xs
escapeString ('\b' : xs) = '\\':'b' :escapeString xs       
escapeString ('\f' : xs) = '\\':'f' :escapeString xs       
escapeString ('\t' : xs) = '\\':'t' :escapeString xs       
escapeString ('\a' : xs) = '\\':'a' :escapeString xs       
escapeString ('\n' : xs) = '\\':'n' :escapeString xs       
escapeString ('\r' : xs) = '\\':'r' :escapeString xs       
escapeString ( x:xs )    = x        :escapeString xs 
escapeString []          = [] 

unescapeNumeric :: Int -> String -> (String -> Int) -> String -> String
unescapeNumeric n numericDigits readNumeric str
   = loop n [] str
   where
   loop _ acc [] = [numericToChar acc]
   loop 0 acc rest
      = numericToChar acc : unescapeString rest
   loop n acc (c:cs)
      | c `elem` numericDigits = loop (n-1) (c:acc) cs
      | otherwise = numericToChar acc : unescapeString (c:cs)
   numericToChar :: String -> Char
   numericToChar = toEnum . readNumeric . reverse

octalDigits, hexDigits :: String
-- | The set of valid octal digits in Python.
octalDigits = "01234567"
-- | The set of valid hex digits in Python.
hexDigits = "0123456789abcdef"


-- TODO: rework escape string to be like unescapeString and handle unrecognized escapes as ( ['\\' : x : xs ] )
{--
escapeString :: String -> String
escapeString xs = concat xs''
    where
    xs'  = [ if x == '\"' || x == '\'' || x == '\\' then '\\':[x] else [x] | x <- xs ]
    xs'' = [ if x == '\n' then '\\':'n':xs else x:xs | (x:xs) <- xs' ]
--}
stripShortQuotes :: String -> String
stripShortQuotes xs = init $ tail $ xs 

stripLongQuotes :: String -> String
stripLongQuotes xs = drop 3 ( take (length xs - 3) xs )
                             

processShortString :: String -> String
processShortString [] = []
processShortString (x:xs)
  | x == 'b' || x == 'B'   = processShortString xs 
  | x == 'r' || x == 'R'   = escapeString $ stripShortQuotes (xs)
  | otherwise              = escapeString $ unescapeString $ stripShortQuotes (x:xs)
  -- | otherwise              = escapeString $ unescapeString $ stripShortQuotes (x:xs)


processLongString :: String -> String
processLongString [] = [] 
processLongString (x:xs) 
  | x == 'b' || x == 'B'   = processLongString xs 
  | x == 'r' || x == 'R'   = escapeString $ stripLongQuotes (xs)
  | otherwise              = escapeString $ unescapeString $ stripLongQuotes (x:xs)

