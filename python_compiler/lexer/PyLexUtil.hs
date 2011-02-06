
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
       
