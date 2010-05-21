
$dir = File.dirname(__FILE__)
require $dir + '/test_helper'

class TestTypeChecker < Test::Unit::TestCase
  
  @@working_progs = %w( 
                       test1.tig
                       test2.tig
                       test3.tig
                       test4.tig
                       test5.tig
                       test6.tig
                       test7.tig
                       test8.tig
                       test12.tig
                       test19.tig
                       test20.tig
                       test21.tig
                       test22.tig
                       test23.tig
                       test24.tig
                       test25.tig
                       test26.tig
                       test27.tig
                       test28.tig
                       test29.tig
                       test30.tig
                       test31.tig
                       test32.tig
                       test33.tig
                       test34.tig
                       test35.tig
                       test36.tig
                       test37.tig
                       test38.tig
                       test39.tig
                       test40.tig
                       test41.tig
                       test42.tig
                       test43.tig
                       test44.tig
                       test45.tig
                       test46.tig
                       test47.tig
                       test48.tig
                       matrix.tig
                       queens.tig
                      ).collect { |t| $dir + '/progs/' + t } 

  # merge,db fail due to referencing builtin functions, which we do not define yet
  # test[#].tig are intentional failure tests
  @@failure_progs = %w( 
                       test9.tig 
                       test10.tig
                       test11.tig
                       test13.tig
                       test14.tig
                       test15.tig
                       test16.tig
                       test17.tig
                       test18.tig
                       db.tig
                       merge.tig    
                      ).collect { |t| $dir + '/progs/' + t } 


  def test_working_progs
    parser = RBTiger::Parser.new
    @@working_progs.each do |filename|
      print "type-checking #{filename} ... "
      assert_nothing_raised() do 
        RBTiger::AbstractSyntax.translateProgram(  parser.scan_file( filename ) )
      end
      puts "succeeded"
    end
  end
  
  def test_failure_progs
    parser = RBTiger::Parser.new
    @@failure_progs.each do |filename|
      print "type-checking #{filename} ... "
      assert_raises( RBTiger::TypeMismatch, RBTiger::UndefinedSymbol, RBTiger::RBException ) do
        RBTiger::AbstractSyntax.translateProgram(  parser.scan_file( filename ) )
      end
      puts "failed as expected"
    end
  end
end
