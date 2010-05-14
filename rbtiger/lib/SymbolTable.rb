
class SymbolTable

  class Binder
    attr_accessor :attributes
    attr_accessor :tail

    def initialize( symbol, attributes, tail )
      @symbol     = symbol
      @attributes = attributes
      @tail       = tail
    end
  end


  def initialize
    @lookup_table = Hash.new       # Symbol -> Binder
    @binder_stack = Array.new      # Binders
  end

  def pushScope
    # Push sentinel value onto binder stack to mark beginning of new scope

  end

  def popScope
    # Pop all binders and associated down to sentinel marker, removing
    # each from symbol table
    
  end

  def insert( symbol, attributes )
    # check that this symbol not already present in this scope
    
    # create new binder and add it to hash and binder stack
    #
  end

  def locate
    # lookup in hash

  end

end
