
module RBTiger

  class Type
    private_class_method :new

    def matches t2
      return self.is_a? t2.class
    end
  end

  class IntType < Type
  end

  class StringType < Type
  end

  class RecordType < Type
    
    @@next_unique_id = 0

    attr_accessor :fields

    def initialize fields
      # Array of ( symbol, type ) doubles
      @fields = fields
      @unique_id = @@next_unique_id
      @@next_unique_id += 1
    end

    def matches t2
      return self == t2
    end

  end

  class ArrayType < Type
    
    @@next_unique_id = 0

    attr_accessor :elem_type

    def initialize elem_type 
      @elem_type = elem_type 
      @unique_id = @@next_unique_id
      @@next_unique_id += 1
    end
    
    def matches t2
      return self == t2
    end

  end

  class NilType < Type
    
    def matches t2
      return t2.is_a? RecordType || self.is_a? t2.class
    end
  end

  class UnitType < Type
  end

  class NameType < Type

    attr_accessor :symbol
    attr_accessor :type

    def initialize symbol, type 
      @symbol = symbol
      @type   = type
    end
  end

end
