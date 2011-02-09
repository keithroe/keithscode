#! /usr/bin/env ruby

passed    = 0
num_tests = 523

1.upto( num_tests ) do |i|
  infile       = "submitted/test.#{i}.py"
  outfile      = "submitted/test.#{i}.py.mine"
  expectedfile = "submitted/test.#{i}.py.expected"

  print "[#{i}/#{num_tests}] Running test ... "

  `./pylex < #{infile} > #{outfile}`
  if( !($?.success?) )
    puts "(./pylex < #{infile}) failed to run" 
    next
  end

  diff = `diff #{outfile} #{expectedfile}`
  

  if( diff.empty? )
    puts "passed."
    passed += 1
  else
    puts "failed."
    puts diff
    exit
  end
end

print "[#{passed}/#{num_tests}] tests passed."
