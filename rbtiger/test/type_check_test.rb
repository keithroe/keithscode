
$dir = File.dirname(__FILE__)
require $dir + '/test_helper'
require $dir + '/progs/test_cases'


class TestTypeChecker < Test::Unit::TestCase
  def TestTypeChecker.create_method(name, &block)
    TestTypeChecker.send(:define_method, name, &block)
  end
end


$success_test_cases.each do |filename|
  TestTypeChecker.create_method( :"test_#{File.basename( filename, '.tig' )}" ) do
    parser = RBTiger::Parser.new
    assert_nothing_raised() do
      RBTiger::AST.translateTree(  parser.scan_file( File.join( $dir, "progs", filename ) ) )
    end
  end
end


$failure_test_cases.each do |exception, filenames|
  filenames.each do |filename|
    TestTypeChecker.create_method( :"test_#{File.basename( filename, '.tig' )}" ) do
      parser = RBTiger::Parser.new
      assert_raises( exception ) do
        RBTiger::AST.translateTree(  parser.scan_file( File.join( $dir, "progs", filename ) ) )
      end
    end
  end
end

