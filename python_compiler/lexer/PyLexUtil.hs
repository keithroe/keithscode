
module PyLexUtil where  -- Export everything for now

import Numeric (readInt)
import Data.Char (digitToInt, isDigit)


stringToInt :: String -> Int
stringToInt str@(x:[]) = read str
stringToInt str@(x0:x1:xs)
    | x0         /= '0'      = read str
    | x1 == 'x' || x1 == 'X' = read str
    | x1 == 'o' || x1 == 'O' = read str
    | x1 == 'b' || x1 == 'B' = readBin xs
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


unescapeString :: String -> String
unescapeString xs = xs'''''
    where
    xs'       = strsub xs     "\\\""   "\""
    xs''      = strsub xs'    "\\'"    "'"
    xs'''     = strsub xs''   "\\\n"   ""
    xs''''    = strsub xs'''  "\\\r\n" ""
    xs'''''   = strsub xs'''' "\\\r"   ""

escapeString :: String -> String
escapeString xs = concat [ if x == '"' || x == '\'' then '\\':[x] else [x] | x <- xs ]

stripShortQuotes :: String -> String
stripShortQuotes xs = init $ tail $ xs 

stripLongQuotes :: String -> String
stripLongQuotes xs = drop 3 ( take (length xs - 3) xs )
                             

processShortString :: String -> String
processShortString [] = []
processShortString (x:xs)
  | x == 'r' || x == 'R'   = stripShortQuotes (x:xs)
  | otherwise              = escapeString $ stripShortQuotes (x:xs)


processLongString :: String -> String
processLongString [] = [] 
processLongString (x:xs) 
  | x == 'r' || x == 'R'   = stripLongQuotes (x:xs)
  | otherwise              = escapeString $ stripShortQuotes (x:xs)

