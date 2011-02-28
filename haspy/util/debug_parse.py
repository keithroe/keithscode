#!/usr/bin/env python

import ast
import sys 
import tokenize 
import token

print( "foo" )

if len( sys.argv ) != 2:
    print( "usage: %s: <filename.txt>" % (sys.argv[0]) )
    sys.exit(0)


print( "------------------------------------------------------------------" )
print( "Token stream for file <", sys.argv[0], ">:\n" )
file = open( sys.argv[1], 'r' )
g = tokenize.generate_tokens( file.readline )
for toknum, tokval, _, _, _  in g:
    if( tokval == "\n" ):
        tokval = '\\n'
    print( token.tok_name[ toknum ], ": '", tokval, "'", sep='' )
file.close()


def printNode( node, prefix = '', first = False ):
    out = sys.stdout
    local_prefix = '' if first else prefix

    if isinstance(node, ast.AST):
        name = node.__class__.__name__
        out.write( local_prefix + name + ' ( ' )
        
        for child_name in node._fields[0:1]:
            child_node = getattr( node, child_name )
            out.write( child_name + "=" )
            printNode( child_node, prefix + ' ' * ( len(name)+len(child_name) + 3 ), True )
        for child_name in node._fields[1:]:
            child_node = getattr( node, child_name )
            out.write( prefix + child_name + "=" )
            printNode( child_node, prefix + ' ' * ( len(name)+len(child_name)  ), True )

        if( len( node._fields ) == 0 ):
          out.write( ')\n' )
        else: 
          out.write(  prefix + ')\n' )

    elif isinstance( node, list ):
        out.write( local_prefix + '[ ' )
        for item in node[0:1]:
            printNode( item, prefix + '  ', True )
        for item in node[1:]:
            printNode( item, prefix + '  ' )
        
        if( len( node ) == 0 ):
            out.write( ']\n' )
        else: 
            out.write( prefix + ']\n' )
    else:
        out.write( local_prefix + repr(node) + '\n' )


print( "\n\n------------------------------------------------------------------" )
print( "Parse tree for file <", sys.argv[0], ">:\n" )
file = open( sys.argv[1], 'r' )
string = file.read()
node = ast.parse( string )
printNode( node )
print( ast.dump( node ) )


