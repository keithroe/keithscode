
dir = File.dirname( __FILE__ )
require dir + '/AbstractSyntax'
require dir + '/SymbolTable'

module RBTiger

################################################################################
#
#
#
################################################################################

class Translate

  def initialize( ast )
    @type_env = SymbolTable.new
    @var_env  = SymbolTable.new

    initTypeEnv()
    initVarEnv()
    pushScopes()

    ast.setState self
    ast.translate
  end


  def initTypeEnv()
    @type_env.insert( Symbol.create( 'int' ), INT.new )
    @type_env.insert( Symbol.create( 'string' ), STRING.new )
  end
  
  def initVarEnv()
  end
  
  def pushScopes()
    @type_env.pushScope
    @var_env.pushScope
  end
  
  def popScopes()
    @type_env.popScope
    @var_env.popScope
  end
  
  def insertType( type_name, type )
    @type_env.insert( type_name, type )
  end

  def insertVar( var_name, var )
    @var_env.insert( var_name, var )
  end

  def locateType( type_name )
    type = @type_env.locate( type_name )
    return type.actual if type
    raise UndefinedSymbol.new( type_name, @lineno )
  end

  def locateTypeShallow( type_name )
    type = @type_env.locate( type_name )
    return type if type
    raise UndefinedSymbol.new( type_name, @lineno )
  end


  def locateVar( var_name )
    var = @var_env.locate( var_name )
    return var if var 
    raise UndefinedSymbol.new( var_name, @lineno )
  end

end


################################################################################
#
# Expression
#
################################################################################

class NilExp < Exp
  def translate
    return [ nil, NIL.new ]
  end
end


class IntExp < Exp
  def translate
    return [ nil, INT.new ]
  end
end


class StringExp < Exp
  def translate
    return [ nil, STRING.new ]
  end
end


class CallExp < Exp
  def translate
   entry = @@state.locateVar( @func_name )
   formals = entry.formals
   if formals.size != @args.size
     raise RBException.new( "Call to func '#{func_name.string}' expected (#{formals.size}) params, not (#{@args.size})",
                         @lineno ) 
   end
   @args.each_with_index do |arg, i|
     translate_arg = arg.translate
     Type::assert_matches( formals[i], translate_arg[1], @lineno )
   end

   return [ nil, entry.ret_type.actual ]
  end
end


class OpExp < Exp
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
  def translate
    record_type = @@state.locateType( @type )

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
  def translate
    lvalue = @var.translate
    rvalue = @exp.translate

    Type::assert_matches( lvalue[1], rvalue[1], lineno )

    return [ nil, UNIT.new ]
  end
end


class IfExp < Exp
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
  def translate
    test_translate = @test_exp.translate
    Type::assert_int( test_translate[1], @test_exp.lineno )

    body_translate = @body_exp.translate
    Type::assert_unit( body_translate[1], body_exp.lineno )

    return [ nil, UNIT.new ]
  end
end


class ForExp < Exp
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
  def translate
    @@state.pushScopes

    @dec_list.each do |dec|
      dec.translate
    end

    let_type = UNIT.new
    @exp_seq.each do |exp|
      exp_translate = exp.translate
      let_type = exp_translate[1]
    end

    @@state.popScopes
    
    return [ nil, let_type ]
  end
end


class ArrayExp < Exp
  def translate
    array_type = @@state.locateType( @type )

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

class FuncDecs < Dec
  def translate
    # check for duplicate func decls in this sequence
    seen = Array.new
    @funcs.each do |func_dec|
      if seen.include?( func_dec.func_name )
        raise RBException.new("'#{func_dec.func_name.string}' multiply defined in func decl sequence", func_dec.lineno)
      end
      seen.push func_dec.func_name
    end
    @funcs.each { |func| func.translateHeader }
    @funcs.each { |func| func.translateBody }
  end
end


class FuncDec < Dec
  def translateHeader
    formals = Array.new
    @params.each do |param|
      formals.push @@state.locateType( param[1] )
    end

    # if ret_type is nil, we need to create Unit type for return type
    ret_type = @ret_type ? @@state.locateType( @ret_type ) : UNIT.new

    @@state.insertVar( @func_name, SymbolTable::FuncEntry.new( ret_type, formals ) )
  end

  def translateBody

    # Place formal params into the local function namespace
    @@state.pushScopes

    @params.each do |param|
      @@state.insertVar( param[0], @@state.locateType( param[1] ) )
    end
 
    body_translate = @body_exp.translate

    ret_type = @ret_type ? @@state.locateType( @ret_type ) : UNIT.new
    Type::assert_matches( ret_type, body_translate[1], @lineno )

    @@state.popScopes
  end
end


class VarDec < Dec
  def translate
    translate_init = @init_exp.translate
      
    if @var_type 
      var_type = @@state.locateType( @var_type )
      Type::assert_matches( var_type, translate_init[1], @lineno )
      @@state.insertVar( @var_name, var_type )
    else
      if( translate_init[1].is_a?( NIL ) )
        raise RBException.new( "'nil' value assigned to non-record variable", @lineno )
      end
      @@state.insertVar( @var_name, translate_init[1] )
    end
  end
end


class TypeDecs < Dec
  def translate
    # check for duplicate type decls in this sequence
    seen = Array.new
    @type_decs.each do |type_dec|
      if seen.include?( type_dec.type_name )
        raise RBException.new("'#{type_dec.type_name.string}' multiply defined in type decl sequence", type_dec.lineno)
      end
      seen.push type_dec.type_name
    end

    @type_decs.each { |type_dec| type_dec.translateHeader }
    @type_decs.each { |type_dec| type_dec.translateBody }
    @type_decs.each { |type_dec| type_dec.detectCycle}
  end
end


class TypeDec < Dec
  def translateHeader
    @@state.insertType( @type_name, NAME.new( @type_name, nil ) )
  end
  
  def translateBody
    # translate the body
    translate_body = @type.translate

    # bind the name ref to this type now
    @@state.locateTypeShallow( @type_name ).bind( translate_body[1] )
  end
  
  def detectCycle 
    if @@state.locateTypeShallow( @type_name ).detectCycle
      raise RBException.new( "type cycle detected for type '#{@type_name.string}'", @lineno )
    end
  end
end


################################################################################
#
# Types
#
################################################################################

class RecordSpec < TypeSpec
  def translate
    fields = Hash.new
    @fields.each do |field|
      fields[ field[0] ] = @@state.locateTypeShallow( field[1] )
    end
    [ nil, RECORD.new( fields ) ]
  end
end


class ArraySpec < TypeSpec
  def translate
    [ nil, ARRAY.new( @@state.locateType( @elem_type ) ) ]
  end
end


class NameSpec < TypeSpec
  def translate
    [ nil, @@state.locateTypeShallow( @type_name ) ]
  end
end


################################################################################
#
# Variable accesors 
#
################################################################################

class SimpleVar < Var
  def translate
    [ nil, @@state.locateVar( @var_name ).actual ]
  end
end


class RecordVar < Var
  def translate
    # TODO: is using hash reasonable?
    translate_var = @var.translate
    Type::assert_is_a( translate_var[1], RECORD, @linena )

    if( !( translate_var[1].fields.include?( @field_name ) ) )
      raise UndefinedSymbol.new( "#{@var.var_name.string}.#{@field_name.string}", @lineno )
    end
    [ nil, translate_var[1].fields[ @field_name ].actual ]
  end
end


class SubscriptVar < Var
  def translate
    translate_var = @var.translate
    Type::assert_is_a( translate_var[1], ARRAY, @lineno )
    [ nil, translate_var[1].elem_type.actual ]
  end
end


end # module RBTiger
