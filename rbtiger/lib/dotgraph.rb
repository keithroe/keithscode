
  
module RBTiger

class AST 

  def AST.printDotGraph( node, stream = $stdout )


    stream.puts "digraph g {"
    node.printDot( stream, "Root", []   )
    stream.puts "}"
  end


  def AST.dotName( node )
    "#{node.class.to_s.gsub( 'RBTiger::', '' )}_#{node.object_id}"
  end

  def printDot( stream, member_name, node_stack )
    pred =  node_stack.empty?() ? "" : AST.dotName( node_stack.last ) + " -> " 
    stream.puts "  #{pred}#{ AST.dotName(self)}"
    
    nstr  = "    #{AST.dotName( self )} "
    nstr += "[ shape=#{shape()}, label=\"#{member_name}:#{self.class.to_s.gsub('RBTiger::', '')}\\n"

    node_stack.push self
    self.ordered_vars.each do |varname|
      obj = self.instance_variable_get varname
      if obj.is_a? AST
        obj.printDot( stream, varname, node_stack )
      elsif( obj.is_a? Enumerable and not obj.is_a? String )
        obj.each_with_index do |element, i|
          element.printDot( stream, "#{varname}[#{i}]", node_stack )if element.is_a? AST
        end
      else
        nstr += "#{varname} = #{obj}\\n"
      end
    end
    node_stack.pop 

    nstr += "\" ]"

    stream.puts nstr 
    
  end
end


class Symbol < AST
  def shape
    "box"
  end
end

class Exp < AST
  def shape
    "ellipse"
  end
end

class Dec < AST
  def shape
    "octagon"
  end
end

class FuncDec < Dec
  def shape
    "doubleoctagon"
  end
end

class TypeDec < Dec
  def shape
    "doubleoctagon"
  end
end

class TypeSpec < AST
  def shape
    "hexagon"
  end
end

class Var < AST
  def shape
    "trapezium"
  end
end

class Op < AST
  def shape
    "triangle"
  end
end


end # module RBTiger
