
module Main (main) where

import Haspy.Parse.Lex
import Haspy.Parse.Parse

main :: IO ()
main = do
  s <- getContents
  print $ parse $ tokens s 

