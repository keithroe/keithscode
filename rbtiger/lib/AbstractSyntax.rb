
dir = File.dirname( __FILE__ )
require dir + '/SymbolTable'
require dir + '/Type'


module RBTiger

################################################################################
#
# Base AS class
#
################################################################################
  
class AbstractSyntax

  @@type_env = SymbolTable.new
  @@var_env  = SymbolTable.new

  #TODO: this should live outside of AbsSyn.  environments probably shoud live elsewhere altogether
  def AbstractSyntax.initEnv
    @@type_env.insert( Symbol.create( 'int' ), INT.new )
    @@type_env.insert( Symbol.create( 'string' ), STRING.new )
  end
  
  def AbstractSyntax.printGraph( node, stream = $stdout )
    @@stream     = stream 
    @@node_stack = []

    @@stream.puts "digraph g {"
    node.printNode( "Root" )
    @@stream.puts "}"
  end


  attr_reader   :ordered_vars
  attr_reader   :lineno


  def initialize( lineno )
    @lineno       = lineno
    @ordered_vars = []
  end


  def locateType( type_name )
    type = @@type_env.locate( type_name )
    return type.actual if type
    raise UndefinedSymbol.new( type_name, @lineno )
  end

  def locateTypeShallow( type_name )
    type = @@type_env.locate( type_name )
    return type if type
    raise UndefinedSymbol.new( type_name, @lineno )
  end


  def locateVar( var_name )
    var = @@var_env.locate( var_name )
    return var if var 
    raise UndefinedSymbol.new( var_name, @lineno )
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

  def to_s
    "Symbol: '#{@string}'"
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
  def translate
    return [ nil, NIL.new ]
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
    return [ nil, INT.new ]
  end
end

class StringExp < Exp
  attr_accessor :value

  def initialize( lineno, value )
    @value = value
    @ordered_vars = %w(@value)
  end
  
  def translate
    return [ nil, STRING.new ]
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
   entry = locateVar( @func_name )
   return [ nil, entry.ret_type.actual ]
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
    
    if @op.integer_operation?
      Type::assert_int( ltype, @lineno )
      Type::assert_int( rtype, @lineno )
    else
      Type::assert_matches( ltype, rtype, @lineno )
      if( !( ltype.is_a?( INT ) || ltype.is_a?( ARRAY ) || ltype.is_a?( RECORD ) ) )
        raise RBException( "Operands must be of type INT, ARRAY, or RECORD", @lineno )
      end
    end 

    return [ nil, INT.new ]
  end
end

class RecordExp < Exp
  attr_accessor :type
  attr_accessor :fields

  def initialize( lineno, type, fields )
    super lineno
    @type   = type
    @fields = fields
    @ordered_vars = %w(@type @fields) 
  end

  def translate
    record_type = locateType( @type )
    fields.each do |field|
      translated_expr = field[1].translate
      field_type      = record_type.fieldType( field[0] )
      raise RBException( "Requested record field '#{field[0].name}' not found", @lineno ) if( !field_type )
      field_type      = field_type.actual
      expr_type       = translated_expr[ 1 ]
      Type::assert_matches( field_type, expr_type, lineno )
    end
    return [ nil, record_type ]
  end
end

class SeqExp < Exp
  attr_accessor :exps

  def initialize( lineno, exps )
    super lineno
    @exps = exps
    @ordered_vars = %w(@exps) 
  end

  def translate
    seq_type = UNIT.new
    @exps.each do |exp|
      translated = exp.translate
      seq_type = translated[1] 
    end
    return [ nil, seq_type ]
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

  def translate
    lvalue = @var.translate
    rvalue = @exp.translate

    Type::assert_matches( lvalue[1], rvalue[1], lineno )

    return [ nil, UNIT.new ]
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

  def translate
    test_translate = @test_exp.translate
    Type::assert_int( test_translate[1], @test_exp.lineno )

    then_translate = @then_exp.translate
    if @else_exp 
      else_translate = @else_exp.translate
      Type::assert_matches then_translate[1], else_translate[1], @else_exp.lineno
      return [ nil, then_translate[1] ]
    else
      Type::assert_unit( then_translate[1], then_exp.lineno )
      return [ nil, UNIT.new ]
    end
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

  def translate
    test_translate = @test_exp.translate
    Type::assert_int( test_translate[1], @test_exp.lineno )

    body_translate = @body_exp.translate
    Type::assert_unit( body_translate[1], body_exp.lineno )

    return [ nil, UNIT.new ]
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

  def translate
    lo_translate = @lo_exp.translate
    Type::assert_int( lo_translate[1], @lo_exp.lineno )
    
    hi_translate = @hi_exp.translate
    Type::assert_int( hi_translate[1], @hi_exp.lineno )

    body_translate = @body_exp.translate
    Type::assert_unit( body_translate[1], body_exp.lineno )

    return [ nil, UNIT.new ]
  end
end

class BreakExp < Exp
  def translate
    return [ nil, UNIT.new ]
  end
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

  def translate
    @@var_env.pushScope
    @@type_env.pushScope

    @dec_list.each do |dec|
      dec.translate
    end

    let_type = UNIT.new
    @exp_seq.each do |exp|
      exp_translate = exp.translate
      let_type = exp_translate[1]
    end

    @@var_env.popScope
    @@type_env.popScope
    
    return let_type
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

  def translate
    array_type = locateType( @type )

    size_translate = @size_exp.translate
    Type::assert_int size_translate[1], @size_exp.lineno
    
    init_translate = @init_exp.translate
    Type::assert_matches( array_type.elem_type.actual, init_translate[1], lineno )

    return [ nil, array_type ]
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
    @funcs        = funcs
    @ordered_vars = %w(@funcs)
  end

  def translate
    @funcs.each { |func| func.translateHeader }
    @funcs.each { |func| func.translateBody }
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
  
  def translateHeader
    formals = Array.new
    @params.each do |param|
      formals.push locateType( param[1] ) #TODO: do these need to be Name type enclosed?
    end

    # if ret_type is nil, we need to create Unit type for return type
    ret_type = @ret_type ? locateType( @ret_type ) : UNIT.new

    @@var_env.insert( @func_name, SymbolTable::FuncEntry.new( ret_type, formals ) )
  end

  def translateBody

    # Place formal params into the local function namespace
    @@var_env.pushScope
    @params.each do |param|
      @@var_env.insert( param[0], locateType( param[1] ) )
    end
 
    body_translate = @body_exp.translate

    ret_type = @ret_type ? locateType( @ret_type ) : UNIT.new
    Type::assert_matches( ret_type, body_translate[1], @lineno )

    @@var_env.popScope
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

  def initialize( lineno, var_name, var_type, init_exp, escape )
    super lineno
    @var_name = var_name
    @var_type = var_type
    @init_exp = init_exp
    @escape   = escape
    @ordered_vars = %w(@var_name @var_type @init_exp @escape) 
  end

  def translate
    translate_init = @init_exp.translate
    if @var_type 
      var_type = locateType( @var_type )
      Type::assert_matches( var_type, translate_init[1], @lineno )
      @@var_env.insert( @var_name, var_type )
    else
      @@var_env.insert( @var_name, translate_init[1] )
    end
  end
end


class TypeDecs < Dec

  attr_accessor :type_decs

  def initialize( type_decs )
    @type_decs = type_decs
    @ordered_vars = %w(@type_decs) 
  end

  def translate
    @type_decs.each { |type_dec| type_dec.translateHeader }
    @type_decs.each { |type_dec| type_dec.translateBody }
    @type_decs.each { |type_dec| type_dec.detectCycle}
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

  def translateHeader
    @@type_env.insert( @type_name, NAME.new( @type_name, nil ) )
  end
  
  def translateBody
    # translate the body
    translate_body = @type.translate

    # bind the name ref to this type now
    locateTypeShallow( @type_name ).bind( translate_body[1] )
  end
  
  def detectCycle 
    if locateTypeShallow( @type_name ).detectCycle
      raise RBException.new( "type cycle detected for type '#{@type_name.string}'", @lineno )
    end
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

class TypeSpec < AbstractSyntax
  def shape
    "hexagon"
  end
end


class RecordSpec < TypeSpec

  attr_accessor :fields

  def initialize( lineno, fields )
    super lineno
    # Array of ( symbol, type ) doubles
    @fields = fields
    @ordered_vars = %w(@fields) 
  end

  def translate
    fields = Hash.new
    @fields.each do |field|
      fields[ field[0] ] = locateTypeShallow( field[1] )
    end
    [ nil, RECORD.new( fields ) ]
  end
end


class ArraySpec < TypeSpec

  attr_accessor :elem_type

  def initialize( lineno, elem_type)
    super lineno
    @elem_type = elem_type
    @ordered_vars = %w(@elem_type) 
  end

  def translate
    [ nil, ARRAY.new( locateType( @elem_type ) ) ]
  end
end


class NameSpec < TypeSpec

  attr_accessor :type_name

  def initialize( lineno, type_name )
    super lineno
    @type_name = type_name
    @ordered_vars = %w(@type_name) 
  end
  
  def translate
    [ nil, locateType( @type_name ) ]
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

  def translate
    [ nil, locateVar( @var_name ).actual ]
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
  
  def translate
    # TODO: is using hash reasonable?
    translate_var = @var.translate

    if( !( translate_var[1].fields.include?( @field_name ) ) )
      raise "crap"
    end
    [ nil, translate_var[1].fields[ @field_name ].actual ]
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
      
  def translate
    translate_var = @var.translate
    Type::assert_is_a( translate_var[1], ARRAY, @lineno )
    [ nil, translate_var.elem_type.actual ]
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


#TODO: This should go somewhere else
AbstractSyntax.initEnv()

end # module RBTiger
