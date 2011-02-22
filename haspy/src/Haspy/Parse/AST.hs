
module Haspy.Parse.AST where


data Ident = Ident {
       identName :: String
    }

data Mod = Module {
        modBody :: [ Stmt ] 
    }


data Stmt
    = FuncDef {
          funcDefName               :: Ident,
          functionDefArgs           :: Args,
          functionDefBody           :: [Stmt],
          functionDefDecoratorListt :: [Expr],
          functionDefReturns        :: Maybe Expr
      }
    | ClassDef {
          classDefName          :: Ident,
          classDefBases         :: [ Expr ],
          classDefKeywords      :: [ Keyword ],
          classDefStarArgs      :: Maybe Expr,
          classDefKwargs        :: Maybe Expr,
          classDefBody          :: [ Stmt ],
          classDefDecoratorList :: [ expr ]
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
          expr target, operator op, expr value)
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
data expr = BoolOp(boolop op, expr* values)
    | BinOp(expr left, operator op, expr right)
    | UnaryOp(unaryop op, expr operand)
    | Lambda(arguments args, expr body)
    | IfExp(expr test, expr body, expr orelse)
    | Dict(expr* keys, expr* values)
    | Set(expr* elts)
    | ListComp(expr elt, comprehension* generators)
    | SetComp(expr elt, comprehension* generators)
    DictComp(expr key, expr value, comprehension* generators)
    | GeneratorExp(expr elt, comprehension* generators)
    -- the grammar constrains where yield expressions can occur
    | Yield(expr? value)
    -- need sequences for compare to distinguish between
    -- x < 4 < 3 and (x < 4) < 3
    | Compare(expr left, cmpop* ops, expr* comparators)
    | Call(expr func, expr* args, keyword* keywords,
    expr? starargs, expr? kwargs)
    | Num(object n) -- a number as a PyObject.
    | Str(string s) -- need to specify raw, unicode, etc?
    | Bytes(string s)
    | Ellipsis
    -- other literals? bools?

    -- the following expression can appear in assignment context
    | Attribute(expr value, identifier attr, expr_context ctx)
    | Subscript(expr value, slice slice, expr_context ctx)
    | Starred(expr value, expr_context ctx)
    | Name(identifier id, expr_context ctx)
    | List(expr* elts, expr_context ctx) 
    | Tuple(expr* elts, expr_context ctx)


data expr_context 
    = Load
    | Store
    | Del
    | AugLoad
    | AugStore
    | Param

data slice 
    = Slice(expr? lower, expr? upper, expr? step) 
    | ExtSlice(slice* dims) 
    | Index(expr value) 

data boolop 
    = And 
    | Or 

data operator 
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

data cmpop
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

data comprehension
    = (expr target, expr iter, expr* ifs)

  -- not sure what to call the first argument for raise and except
data excepthandler
    = ExceptHandler(expr? type, identifier? name, stmt* body)

data arguments
    = (arg* args, identifier? vararg, expr? varargannotation,
                     arg* kwonlyargs, identifier? kwarg,
                     expr? kwargannotation, expr* defaults,
                     expr* kw_defaults)
data arg 
    = (identifier arg, expr? annotation)

        -- keyword arguments supplied to call
data keyword 
    = (identifier arg, expr value)

        -- import name with optional 'as' alias.
data alias
    = (identifier name, identifier? asname)

--}
