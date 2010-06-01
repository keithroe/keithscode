
module RBTiger

  class RBException <  Exception
    def initialize message, lineno
      @message = message
      @lineno = lineno
    end

    def to_s
      "Error: '#{@message}' at line #{@lineno}"
    end
  end

  class TypeMismatch < RBException

    def initialize expected, received, lineno
      @expected  = expected 
      @received  = received
      @lineno    = lineno
    end

    def to_s
      "Type mismatch: <<#{@received}>> does not match expected <<#{@expected}>> at line #{@lineno}."
    end
  end
  
  class UndefinedSymbol < RBException
    def initialize symbol_name, lineno
      @symbol_name = symbol_name
      @lineno      = lineno
    end
    
    def to_s
      "Undefined symbol <<#{@symbol_name}>> at line #{@lineno}."
    end
  end

  class MultiplyDefined < RBException

    def initialize symbol_name, current_lineno, previous_lineno
      @symbol_name     = symbol_name 
      @current_lineno  = current_lineno 
      @previous_lineno = previous_lineno 
    end

    def to_s
      "Multiply defined symbol <<#{@symbol_name}>> at '#{@current_lineno}' previously defined at line #{@previous_lineno}."
    end
  end
  
  class SyntaxError < RBException

    def initialize filename, lineno, message 
      @filename = filename
      @lineno   = lineno 
      @message  = message
    end

    def to_s
      "Syntax error at '#{@symbol_name}':#{@current_lineno}.  #{@message}" 
    end
  end

end
