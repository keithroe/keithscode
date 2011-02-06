
all: alex pylex test

PyToken: PyToken.hs
	ghc -c PyToken.hs

alex: PyLex.x
	alex PyLex.x 

PyLexUtil: PyLexUtil.hs
	ghc -c PyLexUtil.hs

pylex: alex PyToken PyLexUtil
	ghc -o pylex PyLex.hs PyToken.o PyLexUtil.o

test:
	./pylex < testcases/blanklines.py
	#./pylex < testcases/simple.py
	#./pylex < testcases/input.py
	#./pylex < testcases/string.py

