
module Haspy.Parse.Util where  -- Export everything for now

import Numeric (readInt, readHex, readOct)
import Data.Char (digitToInt)


stringToInt :: String -> Int
stringToInt [] = error "stringToInt: string input is empty." 
stringToInt str@(_:[]) = read str
stringToInt str@(x0:x1:xs)
    | x0         /= '0'      = read str
    | x1 == 'x' || x1 == 'X' = read str
    | x1 == 'o' || x1 == 'O' = read str
    | x1 == 'b' || x1 == 'B' = readBin xs
    | x1 == '0'              = 0 
    | otherwise              = error $ "failed to convert '" ++ str  ++ "'"
    where 
        readBin :: String -> Int
        readBin binstr = fst $ Numeric.readInt 2 (`elem` "01") Data.Char.digitToInt binstr !! 0
       

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
   | o `elem` octaldigits = 
        unescapeNumeric 3 octaldigits (fst . head . readOct) rest
unescapeString ('\\':'x':rest@(h:_))
   | h `elem` hexdigits = 
        unescapeNumeric 2 hexdigits   (fst . head . readHex) rest
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


-- from language-python package
unescapeNumeric :: Int -> String -> (String -> Int) -> String -> String
unescapeNumeric numdigits numericDigits readNumeric str
   = loop numdigits [] str
   where
   loop _ acc []   = [numericToChar acc]
   loop 0 acc rest = numericToChar acc : unescapeString rest
   loop n acc (x:xs)
      | x `elem` numericDigits = loop (n-1) (x:acc) xs
      | otherwise = numericToChar acc : unescapeString (x:xs)
   numericToChar :: String -> Char
   numericToChar = toEnum . readNumeric . reverse

octaldigits :: String
octaldigits = "01234567"

hexdigits :: String
hexdigits   = "0123456789abcdef"


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


processLongString :: String -> String
processLongString [] = [] 
processLongString (x:xs) 
  | x == 'b' || x == 'B'   = processLongString xs 
  | x == 'r' || x == 'R'   = escapeString $ stripLongQuotes (xs)
  | otherwise              = escapeString $ unescapeString $ stripLongQuotes (x:xs)

