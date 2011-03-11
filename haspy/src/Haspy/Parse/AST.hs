
module Haspy.Parse.AST where

--
-- TODO: reconcile token naming with AST naming (ie, Token.PLUS -> AST.ADD)
--
--
data Ident
    = Ident {
       identName :: String
    }

data Module
    = Module {
        modBody :: [ Stmt ] 
    }


data Stmt
    -- Make this DecoratedFuncDef composed of FuncDef
    = DecoratedFuncDef {
          decoratedFuncDefDecorators :: [Decorator],
          decoratedFuncDefFunc       :: Stmt 
      }
    | FuncDef {
          funcDefName               :: Ident,
          functionDefParams         :: Params,
          functionDefReturns        :: Maybe Expr,
          functionDefBody           :: [Stmt]
      }
    | DecoratedClassDef {
          decoratedclassDefDecorators :: [Decorator],
          decoratedClassDefClass      :: Stmt 
      }
    | ClassDef {
          classDefName          :: Ident,
          classDefBases         :: Maybe Args,
          classDefBody          :: [Stmt]
      }
    | Return {
          returnValue :: Maybe Expr
      }
    | Delete {
          deleteTargets :: [Expr]
      }
    | Assign {
          assignTargets :: [Expr],
          assignValue   :: Expr
      }
    | AugAssign {
          augAssignTarget :: Expr,
          augAssignOp     :: Op,
          augAssignValue  :: Expr
      }
    | For {
          forTarget :: [Expr],  -- Should this be scalar expr?  using tuple?
          forIter   :: Expr,
          forBody   :: [Stmt],
          forOrElse :: [Stmt]
      }
    | While {
          whileTest   :: Expr,
          whileBody   :: [Stmt],
          whileOrElse :: [Stmt]
      }
    | If {
          ifTest   :: Expr,
          ifBody   :: [Stmt],
          ifOrElse :: [Stmt]
      }
    | With {
          withContextExpr  :: Expr,
          withOptionalVars :: Maybe Expr,
          withBody         :: [Stmt]
      }

    | Raise {
          raiseExc   :: Maybe Expr,
          raiseCuase :: Maybe Expr
      }
    | Try {
          tryBody    :: [Stmt],
          tryHandler :: [ExceptHandler],
          tryOrElse  :: [Stmt],
          tryFinal   :: [Stmt]
      }
    | Assert {
          assertTest :: Expr,
          assertMsg  :: Maybe Expr
      }
    | Import { 
          importNames :: [Alias]
      }
    | ImportFrom {
          importFromModule :: Ident,
          importFromNames  :: [Alias],
          importFromLevel  :: Maybe Int
      }

    | Global {
          globalNames :: [Ident]
      }
    | Nonlocal {
          nonlocalNames :: [Ident]
      }
    | Expression {
          expressionValue :: Expr
      }
    | Pass 
    | Break 
    | Continue

        -- BoolOp() can use left & right?
data Expr 
    = Test {
          testIfTrue  :: Expr,
          testCond    :: Expr,
          testIfFalse :: Expr
      }
    | BinOp {
          binOpLeft  :: Expr,
          binOpOp    :: Op,
          binOpRight :: Expr
      }
    | UnaryOp {
          unaryOpOp    :: UnaryOp,
          unaryOperand :: Expr
      }
    | Lambda {
          lambdaArgs :: Params,
          lambdaBody :: Expr
      }
      
    | IfExp {
          ifExpTest   :: Expr,
          ifExpBody   :: Expr,
          ifExpOrElse :: Expr
      }
    | Dict {
          dictKeys   :: [Expr],
          dictValues :: [Expr]
      }
    | Set {
          setElts :: [Expr]
      }
    | ListComp {
          listCompElt        :: Expr,
          listCompGenerators :: Comprehension
      }
    | SetComp {
          setCompElt        :: Expr,
          setCompGenerators :: Comprehension
      }
    | DictComp {
          dictCompKey        :: Expr,
          dictCompValue      :: Expr,
          dictCompGenerators :: Comprehension
      }
    | GeneratorExp {
          generatorExpElt        :: Expr,
          generatorExpGenerators :: [Comprehension]
      }
    -- the grammar constrains where yield expressions can occur
    | Yield {
        yieldValue :: Maybe Expr
      }
    -- need sequences for compare to distinguish between
    -- x < 4 < 3 and (x < 4) < 3
    | Compare {
          compareLeft        :: Expr,
          compareOps         :: [ CmpOp ],
          compareComparators :: [ Expr ]
      }
    | Call {
          callFunc     :: Expr,
          callArgs     :: Args
      }
    | Float {
          floatVal :: Double
      }
    | Int {
          intVal :: Int
      }
    | Imag {
          imageVal :: Double 
      }
    | Str {
          strVal :: String
      }
    | Bytes {
          bytesVal :: String
      }
    | Ellipsis

    -- the following expression can appear in assignment context
    | Attribute {
          attributeValue :: Expr, 
          attributeAttr  :: Ident
          --attributeCtx   :: ExprContext
      }
    | Subscript {
          subscriptValue :: Expr,
          subscriteSlice :: Slice
          --subscriptCtx   :: ExprContext
      }
    | Starred {
          starredValue :: Expr
          --starredCtx   :: ExprContext
      }
    | Name {
          nameID  :: Ident
          --nameCtx :: ExprContext 
      }
    | List {
          listElts :: [Expr]
          --listCtx  :: ExprContext
      } 
    | Tuple {
          tupleElts :: [Expr]
          --tupleCtx  :: ExprContext
      }
    | NoneVal   -- Not sure how python treats this
    | TrueVal   -- Not sure how python treats this
    | FalseVal  -- Not sure how python treats this


data Decorator
    = Decorator {
          decoratorName :: Ident,
          decoratorArgs :: Maybe Args
      }

-- cannot figure out why we need this
{-
data ExprContext 
    = Load
    | Store
    | Del
    | AugLoad
    | AugStore
    | Param
-}

data Slice 
    = Slice {
          sliceLower :: Maybe Expr,
          sliceUpper :: Maybe Expr,
          sliceStep  :: Maybe Expr
      }
    | ExtSlice {
          extSiceDims :: [Slice]
      }
    | Index {
          indexValue :: Expr
      }

data Op 
    = And 
    | Or 
    | Add 
    | Sub
    | Mult
    | Div
    | Mod
    | Pow
    | LShift 
    | RShift
    | BitOr
    | BitXor
    | BitAnd
    | FloorDiv

data UnaryOp
    = Invert
    | Not
    | UAdd
    | USub

data CmpOp
    = Eq
    | NotEq
    | Lt
    | LtE
    | Gt
    | GtE
    | Is
    | IsNot
    | In
    | NotIn

data Comprehension
    = Comprehension {
          comprehensionTarget :: Expr,
          comprehensionIter   :: Expr,
          comprehensionIfs    :: [Expr]
      }


-- not sure what to call the first argument for raise and except
data ExceptHandler
    = ExceptHandler {
          exceptType :: Maybe Expr,
          exceptName :: Maybe Ident,
          exceptBody :: [Stmt]
      }

-- Used for FuncDefs and Lambdas
data Params
    = Params  {
          paramsArgs             :: [Param],
          paramsVarArg           :: Maybe Param,      -- default field will always be Nothing 
          paramsKWOnlyArgs       :: [Param],
          paramsKWArg            :: Maybe Param       -- default field will always be Nothing 
      }

data Param 
    = Param {
          paramParam      :: Ident,
          paramAnnotation :: Maybe Expr,
          paramDefault    :: Maybe Expr
      }


-- For ClassDef, FuncCall and Decorators
data Args 
    = Args {
       argsArgs      :: [Expr],
       argsKeywords  :: [Keyword],
       starArgs      :: Maybe Expr, 
       kwArgs        :: Maybe Expr

      }

-- keyword arguments supplied to call
data Keyword 
    = Keyword {
          keywordArg   :: Ident,
          keywordValue :: Expr
      }

-- import name with optional 'as' alias.
data Alias
    = Alias {
          aliasName   :: Ident,
          aliasAsName :: Maybe Ident
      }


data Trailer
    = TrailerCall { 
          trailerCall :: Args
      }
    | TrailerSubscript {
          trailerSlice :: Slice 
      }
    | TrailerAttribute{
          trailerAttribute :: Ident
      }


makeWith :: [ (Expr, Maybe Expr) ] -> [ Stmt ] -> Stmt
makeWith (x:[]) suite = With (fst x) (snd x) suite
makeWith (x:xs) suite = With (fst x) (snd x) [ makeWith xs suite ]


makeTest :: Expr -> Maybe( Expr, Expr ) -> Expr
makeTest expr Nothing            = expr
makeTest expr (Just( test, e  )) = Test expr test e 


makeCompare :: Expr -> [ (CmpOp, Expr) ] -> Expr
makeCompare e1 xs
    =  Compare e1 ops operators
       where
           (ops, operators) = unzip xs
            


bindTrailer :: Expr -> Trailer -> Expr
bindTrailer e t 
    = case t of
          TrailerCall      args  -> Call e args
          TrailerSubscript slice -> Subscript e slice
          TrailerAttribute id    -> Attribute e id

makeAtomTrailer :: Expr -> [ Trailer ] -> Expr
makeAtomTrailer e (t:[]) =  bindTrailer e t
makeAtomTrailer e (t:ts) =  bindTrailer ( makeAtomTrailer e ts )  t


makeTestList :: [Expr] -> Expr
makeTestList (x:[]) = x
makeTestList xs     = Tuple xs


makeParams :: [Param] -> Maybe ( Maybe Param, [Param], Maybe Param ) -> Params
makeParams args Nothing = Params args Nothing [] Nothing 
makeParams args (Just (vararg, kwonlyargs, kwarg)) = Params args vararg kwonlyargs kwarg

