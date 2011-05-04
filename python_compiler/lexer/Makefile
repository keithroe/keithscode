
all: pylex test

PyToken.o: PyToken.hs PyLexUtil.o
	ghc -c PyToken.hs

PyLex.hs: PyLex.x
	alex PyLex.x 

PyLexUtil.o: PyLexUtil.hs
	ghc -c PyLexUtil.hs

pylex: PyLex.hs PyToken.o PyLexUtil.o
	ghc -o pylex PyLex.hs PyToken.o PyLexUtil.o

test:
	./pylex < testcases/indent.py
	#./pylex < testcases/blanklines.py
	#./pylex < testcases/simple.py
	#./pylex < testcases/input.py
	#./pylex < testcases/string.py


clean:
	rm *.o *.hi PyLex.hs pylex
