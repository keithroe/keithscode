#!/usr/bin/env python

import sys 
import os
import subprocess
import ast


namemap = { 
    "identName":"Name",
    "modBody":"Body",
    "decoratedFuncDefDecorators":"Decorators",
    "decoratedFuncDefFunc":"Func",
    "funcDefName":"Name",
    "functionDefParams":"Params",
    "functionDefReturns":"Returns",
    "functionDefBody":"Body",
    "decoratedclassDefDecorators":"Decorators",
    "decoratedClassDefClass":"ClassClass",
    "classDefName":"Name",
    "classDefBases":"Bases",
    "classDefBody":"Body",
    "returnValue":"Value",
    "deleteTargets":"Targets",
    "assignTargets":"Targets",
    "assignValue":"Value",
    "augAssignTarget":"Target",
    "augAssignOp":"Op",
    "augAssignValue":"Value",
    "forTarget":"Target",
    "forIter":"Iter",
    "forBody":"Body",
    "forOrElse":"OrElse",
    "whileTest":"Test",
    "whileBody":"Body",
    "whileOrElse":"OrElse",
    "ifTest":"Test",
    "ifBody":"Body",
    "ifOrElse":"OrElse",
    "withContextExpr":"ContextExpr",
    "withOptionalVars":"OptionalVars",
    "withBody":"Body",
    "raiseExc":"Exc",
    "raiseCuase":"Cuase",
    "tryBody":"Body",
    "tryHandler":"Handler",
    "tryOrElse":"OrElse",
    "tryFinal":"Final",
    "assertTest":"Test",
    "assertMsg":"Msg",
    "importNames":"Names",
    "importFromModule":"FromModule",
    "importFromNames":"FromNames",
    "importFromLevel":"FromLevel",
    "globalNames":"Names",
    "nonlocalNames":"Names",
    "expressionValue":"Value",
    "testIfTrue":"IfTrue",
    "testCond":"Cond",
    "testIfFalse":"IfFalse",
    "binOpLeft":"Left",
    "binOpOp":"Op",
    "binOpRight":"Right",
    "unaryOpOp":"Op",
    "unaryOperand":"Operand",
    "lambdaArgs":"Args",
    "lambdaBody":"Body",
    "ifExpTest":"ExpTest",
    "ifExpBody":"ExpBody",
    "ifExpOrElse":"ExpOrElse",
    "dictKeys":"Keys",
    "dictValues":"Values",
    "setElts":"Elts",
    "listCompElt":"CompElt",
    "listCompGenerators":"CompGenerators",
    "setCompElt":"CompElt",
    "setCompGenerators":"CompGenerators",
    "dictCompKey":"CompKey",
    "dictCompValue":"CompValue",
    "dictCompGenerators":"CompGenerators",
    "generatorExpElt":"ExpElt",
    "generatorExpGenerators":"ExpGenerators",
    "yieldValue":"Value",
    "compareLeft":"Left",
    "compareOps":"Ops",
    "compareComparators":"Comparators",
    "callFunc":"Func",
    "callArgs":"Args",
    "floatVal":"Val",
    "intVal":"Val",
    "imageVal":"Val",
    "strVal":"Val",
    "bytesVal":"Val",
    "attributeValue":"Value",
    "attributeAttr":"Attr",
    "subscriptValue":"Value",
    "subscriteSlice":"Slice",
    "starredValue":"Value",
    "nameID":"ID",
    "listElts":"Elts",
    "tupleElts":"Elts",
    "decoratorName":"Name",
    "decoratorArgs":"Args",
    "sliceLower":"Lower",
    "sliceUpper":"Upper",
    "sliceStep":"Step",
    "extSiceDims":"SiceDims",
    "indexValue":"Value",
    "comprehensionTarget":"Target",
    "comprehensionIter":"Iter",
    "comprehensionIfs":"Ifs",
    "exceptType":"Type",
    "exceptName":"Name",
    "exceptBody":"Body",
    "paramName":"Name",
    "paramAnnotation":"Annotation",
    "paramDefault":"Default",
    "varParamName":"ParamName",
    "varParamAnnotation":"ParamAnnotation",
    "keywordVarParamName":"VarParamName",
    "keywordVarParamAnnotation":"VarParamAnnotation",
    "positionalValue":"Value",
    "keywordName":"Name",
    "keywordValue":"Value",
    "starArgsValue":"ArgsValue",
    "kwArgsValue":"ArgsValue",
    "aliasName":"Name",
    "aliasAsName":"AsName",
    "trailerCall":"Call",
    "trailerSlice":"Slice",
    "trailerAttribute":"Attribute"
    }



def getOutput( filename ):
        
    with open( filename, 'r' ) as infile:
        string = infile.read()
        node   = ast.parse( string )
        print( ast.dump( node ), "\n" )

        #os.system( "haspy-parse < {0}".format( filename ) )
        output = subprocess.getoutput( "haspy-parse < {0}".format( filename ) )
        output = output.replace(' ', '').replace( ',', ', ' ).replace('{', '(').replace('}', ')')

        for a,b in namemap.items():
            output = output.replace( a, b )

        print( output )
    
    print( "\n" )

if len( sys.argv ) < 2:
    print( "usage: {0}: <filename.txt>".format( sys.argv[0] ) )
    sys.exit(0)

for filename in sys.argv[1:]:
    print( "---- {0}:\n".format( filename ) )
    getOutput( filename )
