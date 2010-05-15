
module RBTiger

class SymbolTable

  class Binder
    attr_accessor :symbol
    attr_accessor :attributes
    attr_accessor :scope_id
    attr_accessor :tail


    def initialize( symbol, attributes, scope_id, tail )
      @symbol       = symbol
      @attributes   = attributes
      @scope_id     = scope_id
      @tail         = tail
    end


    def to_s
    "%-20s :%s %i" % [ @symbol.string,  @attributes.to_s, @scope_id ]
    end
  end


  def initialize
    @lookup_table  = Hash.new       # Symbol -> Binder
    @binder_stack  = Array.new      # Binders
    @current_scope = 0
  end


  def pushScope
    # update current scope id
    @current_scope += 1
  end


  def popScope
    raise "SymbolTable.popScope: called on global_scope " if @current_scope == 0

    # Pop all binders in this scope, removing each associated symbol from hash
    while  @binder_stack.last && @binder_stack.last.scope_id == @current_scope
      binder = @binder_stack.pop

      # This sanity check can go away in future
      hash_binder  = @lookup_table[ binder.symbol.object_id ]
      if !hash_binder || !hash_binder.equal?( binder ) 
        raise "SymbolTable.popScope: hash/stack mismatch"
      end

      if binder.tail 
        @lookup_table[ binder.symbol.object_id ] = binder.tail
      else
        @lookup_table.delete binder.symbol.object_id 
      end 
    end
    
    @current_scope -= 1
  end


  # Returns false if symbol is already present in scope, else true
  def insert( symbol, attributes )
    tail_binder = @lookup_table[ symbol.object_id ]

    # check that symbol being added is not already present in this scope
    return false if tail_binder && tail_binder.scope_id == @current_scope

    # create new binder and add it to hash and binder stack
    binder = Binder.new( symbol, attributes, @current_scope, tail_binder )
    @binder_stack.push binder
    @lookup_table[ symbol.object_id ] = binder 

    return true
  end


  # Returns nil if symbol not present in SymbolTable, else returns attributes
  def locate( symbol )
    binder = @lookup_table[ symbol.object_id ]
    return binder ? binder.attributes : nil
  end


  def to_s 
    scope_id = 0 

    s  = "Global scope begin" + "\n"
    s += "-" * 80 + "\n" 

    @binder_stack.each do |binder|
      if binder.scope_id != scope_id
        (scope_id ... binder.scope_id ).each { s += "-" * 80 + "\n" }
        scope_id = binder.scope_id
      end
      s += binder.to_s + "\n"
    end
    (scope_id ... @current_scope ).each { s += "-" * 80 + "\n" }

    s += "-" * 80 + "\n"
    s += "Current scope end\n"
  end
end

end # module RBTiger
