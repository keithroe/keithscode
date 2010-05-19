
module RBTiger

  class RBException <  Exception
  end

  class TypeMismatch < RBException

    def initialize expected, received, lineno
      super
      @expected  = expected 
      @received  = received
      @lineno    = lineno
    end

    def to_s
      "Type mismatch: '#{@received}' does not match expected '#{@expected}' at line #{@lineno}."
    end
  end
  
  class MultiplyDefined < RBException

    def initialize symbol_name, current_lineno, previous_lineno
      super
      @symbol_name     = symbol_name 
      @current_lineno  = current_lineno 
      @previous_lineno = previous_lineno 
    end

    def to_s
      "Multiply defined symbol '#{@symbol_name}' at '#{@current_lineno}' previously defined at #{@previous_lineno}."
    end
  end
end
