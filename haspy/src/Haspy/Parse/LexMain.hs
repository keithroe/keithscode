
module Main (main) where

import Haspy.Parse.Lex

main :: IO ()
main = do
  s <- getContents
  printTokens ( tokens s )
  --printTokensForClass ( tokens s )

