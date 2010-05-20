
dir = File.dirname( __FILE__ )
require dir + '/Exception'

module RBTiger

class Type

  def Type.assert_matches expected, received, lineno
    raise TypeMismatch.new( expected, received, lineno ) if( !received.matches( expected ) ) 
  end
  
  def Type.assert_int received, lineno
    Type.assert_matches INT.new, received, lineno
  end
    
  def Type.assert_unit received, lineno
    Type.assert_matches UNIT.new, received, lineno
  end
  
  def Type.assert_is_a received, expected_class, lineno
    raise TypeMismatch.new( expected_class, received, lineno ) if( !received.is_a?( expected_class ) )
  end
    
  def matches t2
    return self.is_a?( t2.class )
  end


  def actual
    return self
  end
end


class INT < Type
  def initialize
  end
  
  def to_s
    "INT"
  end
end

class STRING < Type
  def initialize
  end
  
  def to_s
    "STRING"
  end
end


class RECORD < Type

  attr_accessor :fields

  def initialize( fields )
    # Hash : Symbol => Type 
    @fields = fields
  end

  def matches t2
    # see comment under ArrayType.matches for explanation
    return self.equal?( t2 )
  end

  def fieldType( field_name )
    return fields[ field_name ]
  end
  
  def to_s
    "RECORD { #{@fields.to_a.join( ", " ) } }"
  end
end


class ARRAY < Type

  attr_accessor :elem_type

  def initialize( elem_type)
    @elem_type = elem_type
  end

  def matches t2
    # Array.new should only be called from an ArrayType record type creation
    # expression (ie, 'array of string').  This ensures each Array type is unique:
    # type arrtype1 = array of int                # creates unique array type
    # type arrtype2 = array of int                # creates unique array type
    # type arrtype3 = arrtype1                    # does NOT create a new type 
    # var arr1: arrtype1 := arrtype2 [10] of 0    # error, type mismatch
    # var arr2: arrtype1 := arrtype3 [10] of 0    # valid; arrtype1 same as arrtype3 
    #
    # This behavior extends to Records as well

    return self.equal?( t2 )
  end

  def to_s
    "ARRAY of #{@elem_type}"
  end

end


class NIL < Type
  
  def initialize
  end

  def matches t2
    # nil is a valid record type
    return ( (t2.is_a? RECORD ) || (self.equal? t2) )
  end

  def to_s
    "NIL"
  end
end


class UNIT < Type
  def initialize
  end
  
  def to_s
    "UNIT"
  end
end


class NAME < Type

  attr_accessor :type_name
  attr_accessor :type_ref  # Will be nil for partially resolved types

  def initialize( type_name, type_ref )
    @type_name = type_name
    @type_ref  = type_ref
    @ordered_vars = %w(@type_name) 
  end

  def bind( type_ref )
    @type_ref  = type_ref
  end

  def actual
    return @type_ref
  end

  def detectCycle
    return true if @type_ref.nil?
    return false if !@type_ref.is_a?( NAME )
    return @type_ref.detectCycleInternal( self )
  end

  def detectCycleInternal( start_type )
    return true if @type_ref.eql?( start_type )
    return false if !@type_ref.is_a?( NAME )
    return @type_ref.detectCycleInternal( start_type )
  end

  
  def to_s
    "NAME: '#{type_name}' -> '#{type_ref}'"
  end
end


end # module RBTiger
