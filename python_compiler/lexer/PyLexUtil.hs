
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
unescapeString ( '\\':'a':xs )       = '\a' : unescapeString xs
unescapeString ( '\\':'n':xs )       = '\n' : unescapeString xs
unescapeString ( '\\':'\r':'\n':xs ) =   unescapeString xs
unescapeString ( '\\':'\n':xs )      =        unescapeString xs
unescapeString ( '\\':'\r':xs )      =        unescapeString xs
unescapeString ( x:xs )              = x    : unescapeString xs 
unescapeString []                    = [] 


-- TODO: rework escape string to be like unescapeString and handle unrecognized escapes as ( ['\\' : x : xs ] )
escapeString :: String -> String
escapeString xs = concat xs''
    where
    xs'  = [ if x == '\"' || x == '\'' || x == '\\' then '\\':[x] else [x] | x <- xs ]
    xs'' = [ if x == '\n' then '\\':'n':xs else x:xs | (x:xs) <- xs' ]

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

