

#TODO: make all symbol vars be something like elem_type_sym rather than elem_type.
#TODO: look into encapsulating types for field_list elements, translate return type, formal params, etc
#TODO: document expected types for non-obvious params (FuncEntry.formals, Binder.tail, CallExp.args, etc ) 
#TODO: categorize test cases by exactly the type of failure they should produce (file with hash)
#TODO: move each iteration of unit test loops (each file parse or typecheck) into its own function
#      can add these test functions programmatically
#TODO: reworkd Exceptions to allow more specific error descriptions (eg, name of variable associated
#      with a type mismatch, "undefined type" or "undefined record field" instead of undefined type
#      perhaps blocks could be used as a callback mechanism for providing specific exceptions

module RBTiger

################################################################################
#
# Base AST node class
#
################################################################################
  
class AST

  @@state = nil  # Handle for pass specific data

  attr_reader   :ordered_vars
  attr_reader   :lineno


  def initialize( lineno )
    @lineno       = lineno
    @ordered_vars = []
  end


  def setState state
    @@state = state
  end

end


################################################################################
#
# Symbol
#
################################################################################

class Symbol < AST

  private_class_method :new
  attr_accessor :string

  # Global table of symbols.  Ensures 1 symbol per string
  @@table = Hash.new do |hash, key|
    symbol = new( key )
    hash[key] = symbol
    symbol
  end

  def Symbol.create( string )
      @@table[string]
  end

  def initialize( string )
    super lineno
    @string = string 
    @ordered_vars = %w(@string) 
  end

  def to_s
    "Symbol: '#{@string}'"
  end
end


################################################################################
#
# Expression
#
################################################################################

class Exp < AST
end


class NilExp < Exp
end


class IntExp < Exp
  attr_accessor :value

  def initialize( lineno, value )
    super lineno
    @value = value               # integer: value of the literal expression
    @ordered_vars = %w(@value) 
  end
end


class StringExp < Exp
  attr_accessor :value

  def initialize( lineno, value )
    @value = value              # string: value of the string literal expression
    @ordered_vars = %w(@value)
  end
end


class CallExp < Exp
  attr_accessor :func_name
  attr_accessor :args

  def initialize( lineno, func_name, args )
    super lineno
    @func_name = func_name        # Symbol: function name
    @args      = args             # Exp   : function argument expressions 
    @ordered_vars = %w(@func_name @args) 
  end
end


class OpExp < Exp
  attr_accessor :left_exp         # Exp   : left operand expression
  attr_accessor :op               # Op    : operator ( +, -, etc )
  attr_accessor :right_exp        # Exp   : right operand expression 

  def initialize( lineno, left_exp, op, right_exp )
    super lineno
    @left_exp  = left_exp
    @op        = op
    @right_exp = right_exp
    @ordered_vars = %w(@left_exp @op @right_exp) 
  end
end


class RecordExp < Exp
  attr_accessor :type              # Symbol: record type name
  attr_accessor :fields            # Array of [ Symbol, Exp ] : field name, initializer pairs

  def initialize( lineno, type, fields )
    super lineno
    @type   = type
    @fields = fields
    @ordered_vars = %w(@type @fields) 
  end
end


class SeqExp < Exp
  attr_accessor :exps             # Array of Exp : sequence of expressions 

  def initialize( lineno, exps )
    super lineno
    @exps = exps
    @ordered_vars = %w(@exps) 
  end
end


class AssignExp < Exp
  attr_accessor :var              # Symbol: lvalue identifier 
  attr_accessor :exp              # Exp   : rvalue expression

  def initialize( lineno, var, exp )
    super lineno
    @var = var
    @exp = exp
    @ordered_vars = %w(@var @exp) 
  end
end


class IfExp < Exp
  attr_accessor :test_exp         # Exp  : if test expression
  attr_accessor :then_exp         # Exp  : then clause 
  attr_accessor :else_exp         # Exp  : else clause, may be nil 

  def initialize( lineno, test_exp, then_exp, else_exp )
    super lineno
    @test_exp = test_exp
    @then_exp = then_exp
    @else_exp = else_exp
    @ordered_vars = %w(@test_exp @then_exp @else_exp) 
  end
end


class WhileExp < Exp
  attr_accessor :test_exp         # Exp  : while test expression
  attr_accessor :body_exp         # Exp  : body of while loop

  def initialize( lineno, test_exp, body_exp )
    super lineno
    @test_exp = test_exp
    @body_exp = body_exp
    @ordered_vars = %w(@test_exp @body_exp) 
  end
end


class ForExp < Exp
  attr_accessor :var              # Symbol  : loop iteration variable
  attr_accessor :escape           # boolean : default to true ... use later
  attr_accessor :lo_exp           # Exp     : int valued expr for loop var range begin 
  attr_accessor :hi_exp           # Exp     : int valued expr for loop var range end
  attr_accessor :body_exp         # Exp     : loop body

  def initialize( lineno, var, escape, lo_exp, hi_exp, body_exp )
    super lineno
    @var = var
    @escape = escape
    @lo_exp = lo_exp
    @hi_exp = hi_exp
    @body_exp = body_exp
    @ordered_vars = %w(@var @escape @lo_exp @hi_exp @body_exp) 
  end
end


class BreakExp < Exp
end


class LetExp < Exp
  attr_accessor :dec_list
  attr_accessor :body_exp

  def initialize( lineno, dec_list, exp_seq )
    super lineno
    @dec_list = dec_list
    @exp_seq  = exp_seq 
    @ordered_vars = %w(@dec_list @exp_seq) 
  end
end


class ArrayExp < Exp
  attr_accessor :type
  attr_accessor :size_exp
  attr_accessor :init_exp

  def initialize( lineno, type, size, init )
    super lineno
    @type = type
    @size_exp = size
    @init_exp = init
    @ordered_vars = %w(@type @size_exp @init_exp) 
  end
end


################################################################################
#
# Declarations
#
################################################################################

class Dec < AST
end


class FuncDecs < Dec
  attr_accessor :funcs

  def initialize( funcs )
    @funcs        = funcs
    @ordered_vars = %w(@funcs)
  end
end


class FuncDec < Dec
  attr_accessor :func_name
  attr_accessor :params
  attr_accessor :result
  attr_accessor :body_exp

  def initialize( lineno, func_name, params, ret_type, body_exp )
    super lineno
    @func_name = func_name
    @params    = params
    @ret_type  = ret_type
    @body_exp  = body_exp
    @ordered_vars = %w(@func_name @params @ret_type @body_exp)
  end
end


class VarDec < Dec
  attr_accessor :var_name
  attr_accessor :type
  attr_accessor :init_exp
  attr_accessor :escape

  def initialize( lineno, var_name, var_type, init_exp, escape )
    super lineno
    @var_name = var_name
    @var_type = var_type
    @init_exp = init_exp
    @escape   = escape
    @ordered_vars = %w(@var_name @var_type @init_exp @escape) 
  end
end


class TypeDecs < Dec

  attr_accessor :type_decs

  def initialize( type_decs )
    @type_decs = type_decs
    @ordered_vars = %w(@type_decs) 
  end
end


class TypeDec < Dec

  attr_accessor :type_name
  attr_accessor :type

  def initialize( lineno, type_name, type )
    super lineno
    @type_name = type_name
    @type      = type
    @ordered_vars = %w(@type_name @type) 
  end
end


################################################################################
#
# Types
#
################################################################################

class TypeSpec < AST
end


class RecordSpec < TypeSpec

  attr_accessor :fields

  def initialize( lineno, fields )
    super lineno
    # Array of ( symbol, type ) doubles
    @fields = fields
    @ordered_vars = %w(@fields) 
  end
end


class ArraySpec < TypeSpec

  attr_accessor :elem_type

  def initialize( lineno, elem_type)
    super lineno
    @elem_type = elem_type
    @ordered_vars = %w(@elem_type) 
  end
end


class NameSpec < TypeSpec

  attr_accessor :type_name

  def initialize( lineno, type_name )
    super lineno
    @type_name = type_name
    @ordered_vars = %w(@type_name) 
  end
end


################################################################################
#
# Variable accesors 
#
################################################################################

class Var < AST
end


class SimpleVar < Var

  attr_accessor :var_name

  def initialize( lineno, var_name )
    super lineno
    @var_name = var_name
    @ordered_vars = %w(@var_name) 
  end
end


class RecordVar < Var

  attr_accessor :var_name
  attr_accessor :field_name

  def initialize( lineno, var, field_name )
    super lineno
    @var        = var
    @field_name = field_name
    @ordered_vars = %w(@var @field_name) 
  end
end


class SubscriptVar < Var
  attr_accessor :var_name
  attr_accessor :subscript_exp

  def initialize( lineno, var, subscript_exp )
    super lineno
    @var           = var
    @subscript_exp = subscript_exp
    @ordered_vars  = %w(@var @subscript_exp) 
  end
end


################################################################################
#
# Operators 
#
################################################################################

class Op < AST
  def integer_operation?
    return true
  end
end


class PlusOp < Op 
end


class MinusOp < Op
end


class TimesOp < Op 
end


class DivideOp < Op 
end


class EqOp < Op 
  def integer_operation?
    return false
  end
end


class NeqOp < Op 
  def integer_operation?
    return false
  end
end


class LtOp < Op 
end


class LeOp < Op 
end


class GtOp < Op 
end


class GeOp < Op 
end


end # module RBTiger
