
all: alex pylex test

PyToken: PyToken.hs
	ghc -c PyToken.hs

alex: PyLex.x
	alex PyLex.x

pylex: alex PyToken
	ghc -o pylex PyLex.hs PyToken.o

test:
	./pylex < testcases/simple.py
	./pylex < testcases/input.py

