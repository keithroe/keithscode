
$dir = File.dirname(__FILE__)
require $dir + '/test_helper'

class TestSymbol < Test::Unit::TestCase

  def test_private_new
    assert_raises( NoMethodError ) {  s1 = RBTiger::Symbol.new }
  end

  def test_create
    s1 = RBTiger::Symbol.create "foo"
    s2 = RBTiger::Symbol.create "bar"
    assert_not_same( s1, s2 )
    
    s3 = RBTiger::Symbol.create "foo"
    assert_same( s1, s3 )

    assert_equal( s1.string, "foo" )
  end
end
