#!/usr/bin/env python
# author: r. keith

import ast
import sys 
import tokenize 
import token


def printTokens( filename, out = sys.stdout ):
    with open( filename, 'r' ) as infile:
        g = tokenize.generate_tokens( infile.readline )
        for toknum, tokval, _, _, _  in g:
            if( tokval == "\n" ):
                tokval = '\\n'
            out.write( "{0:10s} : {1}\n".format( token.tok_name[ toknum ], tokval ) )


def printAST( filename, out = sys.stdout ):
    with open( sys.argv[1], 'r' ) as infile:
        string = infile.read()
        node   = ast.parse( string )
        printNode( node, '', False, out )

        out.write( "\n\n" + "-" * 80 );
        out.write( "string dump of parser tree for file <{0}>:\n".format( filename ) )
        out.write( ast.dump( node ) )


def printNode( node, prefix = '', output_prefix = False, out = sys.stdout ):
    
    local_prefix = prefix  if output_prefix else '' 

    if isinstance(node, ast.AST):
        name = node.__class__.__name__
        
        if( len( node._fields ) == 0 ):
            out.write( local_prefix + '()\n' )
        else:
            out.write( local_prefix + name + ' (\n' )
            for child_name in node._fields:
                out.write( "{0}{1} = ".format( prefix + '    ', child_name ) )
                printNode( getattr( node, child_name ), prefix + '    ', False )
            out.write(  prefix + ')\n' )

    elif isinstance( node, list ):

        if( len( node ) == 0 ):
            out.write( local_prefix + '[]\n' )
        else: 
            out.write( local_prefix + '[\n' )
            for item in node:
                printNode( item, prefix + '    ', True )
            out.write( prefix + ']\n' )
        
    else:
        out.write( local_prefix + repr(node) + '\n' )


def main():

    if len( sys.argv ) != 2:
        print( "usage: %s: <filename.txt>" % (sys.argv[0]) )
        sys.exit(0)

    print( "-" * 80 );
    print( "Token stream for file <", sys.argv[1], ">:\n" )
    printTokens( sys.argv[1] )

    print( "\n\n" + "-" * 80 );
    print( "Parse tree for file <", sys.argv[1], ">:\n" )
    printAST( sys.argv[1] )


if __name__ == '__main__':
  main()

