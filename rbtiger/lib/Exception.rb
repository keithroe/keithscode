
module RBTiger

  class RBException <  Exception
  end

  class TypeMismatch < RBException

    def initialize expected, received, lineno
      @expected  = expected 
      @received  = received
      @lineno    = lineno
    end

    def to_s
      "Type mismatch: '#{@received}' does not match expected '#{@expected}' at line #{@lineno}."
    end
  end
  
  class UndefinedSymbol < RBException
    def initialize symbol_name, lineno
      @symbol_name = symbol_name
      @lineno      = lineno
    end
    
    def to_s
      "Undefined sumbol '#{@symbol_name}' at line #{@current_lineno}."
    end
  end

  class MultiplyDefined < RBException

    def initialize symbol_name, current_lineno, previous_lineno
      @symbol_name     = symbol_name 
      @current_lineno  = current_lineno 
      @previous_lineno = previous_lineno 
    end

    def to_s
      "Multiply defined symbol '#{@symbol_name}' at '#{@current_lineno}' previously defined at line #{@previous_lineno}."
    end
  end
end
