
$dir = File.dirname(__FILE__)
require $dir + '/test_helper'

class TestSymbol < Test::Unit::TestCase

  def test_types
    int_type     = RBTiger::INT.new
    string_type1 = RBTiger::STRING.new
    string_type2 = RBTiger::STRING.new
    nil_type     = RBTiger::NIL.new
    array_type1  = RBTiger::ARRAY.new( RBTiger::INT.new )
    array_type2  = RBTiger::ARRAY.new( RBTiger::INT.new )
    array_type3  = array_type1 
    record_type1 = RBTiger::RECORD.new( [ [RBTiger::Symbol::create( "author" ), RBTiger::STRING.new ] ,
                                          [RBTiger::Symbol::create( "isbn" ),   RBTiger::INT.new] ] )
    record_type2 = RBTiger::RECORD.new( [ [RBTiger::Symbol::create( "author" ), RBTiger::STRING.new ] ,
                                          [RBTiger::Symbol::create( "isbn" ),   RBTiger::INT.new] ] )
    record_type3 = RBTiger::RECORD.new( [ [RBTiger::Symbol::create( "foo" ),    RBTiger::STRING.new ] ] )
    record_type4 = record_type2 

    assert( !int_type.matches( string_type1 ) )
    assert( string_type2.matches( string_type1 ) )
    assert( string_type1.matches( string_type2 ) )
    assert( !array_type2.matches( array_type1 ) )
    assert( !array_type1.matches( array_type2 ) )
    assert( array_type1.matches( array_type3 ) )
    assert( !record_type1.matches( record_type2 ) )
    assert( !record_type1.matches( record_type3 ) )
    assert( record_type2.matches( record_type4 ) )
    assert( record_type4.matches( record_type2 ) )
    
  end
end
