
dir = File.dirname( __FILE__ )
require dir + '/SymbolTable'


module RBTiger

################################################################################
#
# Base AS class
#
################################################################################
  
class AbstractSyntax
  attr_reader   :ordered_vars
  attr_reader   :lineno

  @@type_env = SymbolTable.new
  @@var_env  = SymbolTable.new

  def initialize( lineno )
    @lineno       = lineno
    @ordered_vars = []
  end

  def AbstractSyntax.printGraph( node, stream = $stdout )
    @@stream     = stream 
    @@node_stack = []

    @@stream.puts "digraph g {"
    node.printNode( "Root" )
    @@stream.puts "}"
  end
  
  def name
    "#{self.class.to_s.gsub( 'RBTiger::', '' )}_#{self.object_id}"
  end

  def printNode( member_name )
    pred =  @@node_stack.empty?() ? "" : @@node_stack.last.name + " -> " 
    @@stream.puts "  #{pred}#{self.name}"

    nstr = "    #{self.name} [ shape=#{self.shape}, label=\"#{member_name}:#{self.class.to_s.gsub('RBTiger::', '')}\\n"

    @@node_stack.push self
    self.ordered_vars.each do |varname|
      obj = self.instance_variable_get varname
      if obj.is_a? AbstractSyntax
        obj.printNode varname
      elsif( obj.is_a? Enumerable and not obj.is_a? String )
        obj.each_with_index do |element, i|
          element.printNode( "#{varname}[#{i}]" )if element.is_a? AbstractSyntax
        end
      else
        nstr += "#{varname} = #{obj}\\n"
      end
    end
    @@node_stack.pop 

    nstr += "\" ]"

    @@stream.puts nstr 
    
  end
end


################################################################################
#
# Symbol
#
################################################################################

class Symbol < AbstractSyntax

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
    @ordered_vars = %w(@string ) 
  end

  def shape
    "box"
  end
end


################################################################################
#
# Expression
#
################################################################################
#
class Exp < AbstractSyntax
  def shape
    "ellipse"
  end
end

class NilExp < Exp
  def initialize
  end

  def translate
    return [ nil, NilType.new ]
  end
end

class IntExp < Exp
  attr_accessor :value

  def initialize( lineno, value )
    super lineno
    @value = value
    @ordered_vars = %w(@value) 
  end

  def translate
    return [ nil, IntType.new ]
  end
end

class StringExp < Exp
  attr_accessor :value

  def initialize( lineno, value )
    @value = value
    @ordered_vars = %w(@value)
  end
  
  def translate
    return [ nil, StringType.new ]
  end
end

class CallExp < Exp
  attr_accessor :func_name
  attr_accessor :args

  def initialize( lineno, func_name, args )
    super lineno
    @func_name = func_name
    @args      = args
    @ordered_vars = %w(@func_name @args) 
  end
  
  def translate
    attributes = @@venv.locate @func_name
    return [ nil, @@venv.locate( @func_name ).type ]
  end
end

class OpExp < Exp
  attr_accessor :left_exp
  attr_accessor :op
  attr_accessor :right_exp

  def initialize( lineno, left_exp, op, right_exp )
    super lineno
    @left_exp  = left_exp
    @op        = op
    @right_exp = right_exp
    @ordered_vars = %w(@left_exp @op @right_exp) 
  end
  
  def translate
    ltype = left_exp.translate()[1]
    rtype = right_exp.translate()[1]
    #raise TypeMismatch( if( !ltype.matches( rtype ) ) 
    return [ nil, @@venv.locate( @func_name ).type ]
  end
end

class RecordExp < Exp
  attr_accessor :type
  attr_accessor :fields

  def initialize( lineno, type, fields )
    super lineno
    @type = type
    @fields = fields
    @ordered_vars = %w(@type @fields) 
  end
end

class SeqExp < Exp
  attr_accessor :exps

  def initialize( lineno, exps )
    super lineno
    @exps = exps
    @ordered_vars = %w(@exps) 
  end
end

class AssignExp < Exp
  attr_accessor :var
  attr_accessor :exp

  def initialize( lineno, var, exp )
    super lineno
    @var = var
    @exp = exp
    @ordered_vars = %w(@var @exp) 
  end
end

class IfExp < Exp
  attr_accessor :test_exp
  attr_accessor :then_exp
  attr_accessor :else_exp

  def initialize( lineno, test_exp, then_exp, else_exp )
    super lineno
    @test_exp = test_exp
    @then_exp = then_exp
    @else_exp = else_exp
    @ordered_vars = %w(@test_exp @then_exp @else_exp) 
  end
end

class WhileExp < Exp
  attr_accessor :test_exp
  attr_accessor :body_exp

  def initialize( lineno, test_exp, body_exp )
    super lineno
    @test_exp = test_exp
    @body_exp = body_exp
    @ordered_vars = %w(@test_exp @body_exp) 
  end
end

class ForExp < Exp
  attr_accessor :var
  attr_accessor :escape
  attr_accessor :lo_exp
  attr_accessor :hi_exp
  attr_accessor :body_exp

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

  def initialize( lineno, dec_list, body_exp )
    super lineno
    @dec_list = dec_list
    @body_exp = body_exp
    @ordered_vars = %w(@dec_list @body_exp) 
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

class Dec < AbstractSyntax
  def shape
    "octagon"
  end
end

class FuncDecs < Dec
  attr_accessor :funcs

  def initialize( funcs )
    @funcs = funcs
    @ordered_vars = %w(@funcs)
  end
end
  
class FuncDec < Dec
  attr_accessor :func_name
  attr_accessor :params
  attr_accessor :result
  attr_accessor :body_exp

  def initialize( lineno, func_name, params, result, body_exp )
    super lineno
    @func_name = func_name
    @params    = params
    @result    = result
    @body_exp  = body_exp
    @ordered_vars = %w(@func_name @params @result @body_exp)
  end
  
  def shape
    "doubleoctagon"
  end
end
  
class VarDec < Dec
  attr_accessor :var_name
  attr_accessor :type
  attr_accessor :init_exp
  attr_accessor :escape

  def initialize( lineno, var_name, type, init_exp, escape )
    super lineno
    @var_name = var_name
    @type     = type
    @init_exp = init_exp
    @escape   = escape
    @ordered_vars = %w(@var_name @type @init_exp @escape) 
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
  
  def shape
    "doubleoctagon"
  end
end


################################################################################
#
# Types
#
################################################################################

class Type < AbstractSyntax
  def shape
    "hexagon"
  end
    
  def matches t2
    return self.is_a?( t2.class )
  end
end


class IntType < Type
  def initialize
  end
end

class StringType < Type
  def initialize
  end
end

class RecordType < Type

  attr_accessor :fields

  def initialize( lineno, fields )
    super lineno
    # Array of ( symbol, type ) doubles
    @fields = fields
    @ordered_vars = %w(@fields) 
  end

  def matches t2
    # see comment under ArrayType.matches for explanation
    return self.equal?( t2 )
  end

end

class ArrayType < Type

  attr_accessor :elem_type

  def initialize( lineno, elem_type)
    super lineno
    @elem_type = elem_type
    @ordered_vars = %w(@elem_type) 
  end

  def matches t2
    # ArrayType.new should only be called from a record type creation expression
    # (ie, 'array of string').  This ensures that each Array type is unique:
    # type arrtype1 = array of int                # creates unique array type
    # type arrtype2 = array of int                # creates unique array type
    # type arrtype3 = arrtype1                    # does NOT create a new type 
    # var arr1: arrtype1 := arrtype2 [10] of 0    # error, type mismatch
    # var arr2: arrtype1 := arrtype3 [10] of 0    # valid; arrtype1 same as arrtype3 
    #
    # This behavior extends to Records as well

    return self.equal?( t2 )
  end

end

class NilType < Type
  
  def initialize
  end

  def matches t2
    # nil is a valid record type
    return ( (t2.is_a? RecordType) || (self.equal? t2) )
  end
end

class UnitType < Type
  def initialize
  end
end

class NameType < Type

  attr_accessor :symbol
  attr_accessor :type

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

class Var < AbstractSyntax
  def shape
    "trapezium"
  end
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

  def initialize( lineno, var_name, field_name )
    super lineno
    @var_name   = var_name
    @field_name = field_name
    @ordered_vars = %w(@var_name @field_name) 
  end
end

class SubscriptVar < Var
  attr_accessor :var_name
  attr_accessor :subscript_exp

  def initialize( lineno, var_name, subscript_exp )
    super lineno
    @var_name      = var_name
    @subscript_exp = subscript_exp
    @ordered_vars = %w(@var_name @subscript_exp) 
  end
end


################################################################################
#
# Operators 
#
################################################################################

class Op < AbstractSyntax
  def shape
    "triangle"
  end
end

class PlusOp < Op 
  def initialize
  end
end

class MinusOp < Op 
  def initialize
  end
end

class TimesOp < Op 
  def initialize
  end
end

class DivideOp < Op 
  def initialize
  end
end

class EqOp < Op 
  def initialize
  end
end

class NeqOp < Op 
  def initialize
  end
end

class LtOp < Op 
  def initialize
  end
end

class LeOp < Op 
  def initialize
  end
end

class GtOp < Op 
  def initialize
  end
end

class GeOp < Op 
  def initialize
  end
end

end # module RBTiger
