
$dir = File.dirname(__FILE__)
require $dir + '/test_helper'

class TestSymbol < Test::Unit::TestCase

  def test_table
    table = RBTiger::SymbolTable.new

    assert( table.insert( RBTiger::Symbol.create( "a" ), [ "int:", 1 ] ) )
    assert( table.insert( RBTiger::Symbol.create( "b" ), [ "int:", 2 ] ) )
    assert( table.insert( RBTiger::Symbol.create( "c" ), [ "func:", "args1" ] ) )
    assert( table.insert( RBTiger::Symbol.create( "d" ), [ "string:", "foo" ] ) )

    table.pushScope

    table.pushScope
    assert( table.insert( RBTiger::Symbol.create( "a" ), [ "string:", "bar"] ) )
    assert( table.insert( RBTiger::Symbol.create( "c" ), [ "func:", "args2"] ) )
    assert( table.insert( RBTiger::Symbol.create( "e" ), [ "int:", 3 ] ) )
    
    table.pushScope
    assert( table.insert( RBTiger::Symbol.create( "a" ), [ "int:", 4 ] ) )
    assert( table.insert( RBTiger::Symbol.create( "f" ), [ "string:", "foobar"] ) )

    assert_equal( table.locate( RBTiger::Symbol.create( "a" ) )[1], 4 )
    assert_equal( table.locate( RBTiger::Symbol.create( "b" ) )[1], 2 )
    assert_equal( table.locate( RBTiger::Symbol.create( "c" ) )[1], "args2" )
    assert_equal( table.locate( RBTiger::Symbol.create( "f" ) )[1], "foobar" )
    assert_nil( table.locate( RBTiger::Symbol.create( "g" ) ) )

    table.popScope
    assert_equal( table.locate( RBTiger::Symbol.create( "a" ) )[1], "bar" )
    assert_equal( table.locate( RBTiger::Symbol.create( "e" ) )[1], 3 )
    assert_nil( table.locate( RBTiger::Symbol.create( "f" ) ) )
    assert_nil( table.locate( RBTiger::Symbol.create( "g" ) ) )
    
    table.popScope
    assert_equal( table.locate( RBTiger::Symbol.create( "a" ) )[1], 1 )
    assert_equal( table.locate( RBTiger::Symbol.create( "d" ) )[1], "foo" )
    assert_nil( table.locate( RBTiger::Symbol.create( "e" ) ) )
    
    table.pushScope
    assert( table.insert( RBTiger::Symbol.create( "h" ), [ "string:", "foofoo"] ) )
    assert_equal( table.locate( RBTiger::Symbol.create( "a" ) )[1], 1 )
    assert_equal( table.locate( RBTiger::Symbol.create( "h" ) )[1], "foofoo" )
    assert_nil( table.locate( RBTiger::Symbol.create( "e" ) ) )
  end
end
