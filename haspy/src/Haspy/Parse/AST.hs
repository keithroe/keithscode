
module Haspy.Parse.AST where

--
-- TODO: reconcile token naming with AST naming (ie, Token.PLUS -> AST.ADD)
--
--
data Ident = Ident {
       identName :: String
    }

data Mod = Module {
        modBody :: [ Stmt ] 
    }


data Stmt
    -- Make this DecoratedFuncDef composed of FuncDef
    = FuncDef {
          funcDefName               :: Ident,
          functionDefParams         :: Params,
          functionDefBody           :: [Stmt],
          functionDefDecoratorListt :: [Decorator],
          functionDefReturns        :: Maybe Expr
      }
    | ClassDef {
          classDefName          :: Ident,
          classDefBases         :: Args,
          classDefBody          :: [Stmt],
          classDefDecoratorList :: [Decorator]
      }
    | Return {
          returnValue :: Maybe expr
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
          forTarget :: Expr,
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
    | TryExcept {
          tryExceptBody    :: [Stmt],
          tryExceptHandler :: [ExceptHandler],
          tryExceptOrElse  :: [Stmt]
      }
    | TryFinally {
          tryFinallyBody      :: [Stmt],
          tryFinallyFinalBody :: [Stmt]
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
          testIfTrue  :: Expr
          testCond    :: Expr
          testIfFalse :: Expr
      }
    | BoolOp { 
          boolOpOp     :: BoolOp,
          boolOpValues :: [Expr]
      }
    | BinOp {
          binOpLeft  :: Expr,
          binOpOp    :: Op,
          binOpRight :: Expr
      }
    | UnaryOp {
          unaryOpOp     :: Op,
          unaryOperand :: Expr
      }
    | Lambda {
          lambdaArgs :: Params,
          lambdaBody :: Expr
      
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
          compareComparators :: [ expr ]
      }
    | Call {
          callFunc     :: Expr,
          callArgs     :: Args
      }
    | Float {
          floatVal :: Double
      }
    | Int {
          intVal :: Integer 
      }
    | Imag {
          imageVal :: Double 
      }
    | Str {
          strVal :: String
      }
    | Bytes(string s)
          bytesVal :: String
      }
    | Ellipsis
    -- the following expression can appear in assignment context
    | Attribute {
          attributeValue :: Expr, 
          attributeAttr  :: Ident,
          --attributeCtx   :: ExprContext
      }
    | Subscript {
          subscriptValue :: Expr,
          subscriteSlice :: Slice,
          --subscriptCtx   :: ExprContext
      }
    | Starred {
          starredValue :: Expr,
          --starredCtx   :: ExprContext
      }
    | Name {
          nameID  :: Ident,
          --nameCtx :: ExprContext 
      }
    | List {
          listElts :: [Expr],
          --listCtx  :: ExprContext
    | Tuple {
          tupleElts :: [Expr],
          --tupleCtx  :: ExprContext
      }


data Decorator
    = Decorator {
          decoratorName :: Name
          decoratorArgs :: [Arg]
      }

-- cannot figure out why we need this
data ExprContext 
    = Load
    | Store
    | Del
    | AugLoad
    | AugStore
    | Param

data Slice 
    = Slice {
          sliceLower :: Maybe Expr,
          sliceUpper :: Maybe Expr,
          sliceStep  :: Maybe Expr,
      }
    | ExtSlice {
          extSiceDims :: [Slice]
      }
    | Index {
          indexValue :: Expr
      }

data boolop 
    = And 
    | Or 

data Op 
    = Add 
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

data unaryop
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
          paramsVarArg           :: Maybe Ident,
          paramsVarArgAnnotation :: Maybe Expr,
          paramsKWOnlyArgs       :: [Param],
          paramsKWArg            :: Maybe Ident,
          paramsKWArgAnnotation  :: Maybe Expr,
          paramsDefaults         :: [Expr],
          paramsKWDefaults       :: [Expr]
      }

data Param 
    = Param {
          paramParam      :: Ident,
          paramAnnotation :: Maybe Expr
      }


-- For ClassDef, FuncCall and Decorators
data Args 
    = Args {
       argsArgs      :: [Expr],
       argsKeywords  :: [Keyword]
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

