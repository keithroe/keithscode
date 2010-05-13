task :default => [:parser, :test]

task :test do
  require File.dirname(__FILE__) + '/test/all_tests.rb'
end


task :lexer => 'lib/tiger.rex' do
	sh 'rex lib/tiger.rex -o lib/tiger_lex.rb'
end

task :parser => [ :lexer, 'lib/tiger.y' ] do
	sh 'racc -v lib/tiger.y -o lib/tiger_parse.rb'
end
