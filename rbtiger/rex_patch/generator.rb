#
# generator.rb
#
#   Copyright (c) 2005-2006 ARIMA Yasuhiro <arima.yasuhiro@nifty.com>
#
#   This program is free software.
#   You can distribute/modify this program under the terms of
#   the GNU Lesser General Public License version 2 or later.
#

module Rex

## ---------------------------------------------------------------------
  class ParseError < StandardError ; end
  class RexError < StandardError ; end

## ---------------------------------------------------------------------
  class Generator
    require 'strscan'
    require 'rex/info'

## ---------------------------------------------------------------------
    attr_accessor :grammar_file
    attr_accessor :scanner_file
    attr_accessor :module_name
    attr_accessor :class_name
    attr_accessor :lineno
    attr_accessor :rules
    attr_accessor :exclusive_states
    attr_accessor :ignorecase
    attr_accessor :independent
    attr_accessor :debug

## ---------------------------------------------------------------------
    def initialize(opts)
      @lineno  =  0
      @macro  =  {}
      @rules  =  []
      @exclusive_states = [nil]
      @grammar_lines  =  nil
      @scanner_header  =  ""
      @scanner_footer  =  ""
      @scanner_inner  =  ""
      @opt  =  opts
    end

## ---------------------------------------------------------------------
    def add_header( st )
      @scanner_header  +=  "#{st}\n"
    end

## ---------------------------------------------------------------------
    def add_footer( st )
      @scanner_footer  +=  "#{st}\n"
    end

## ---------------------------------------------------------------------
    def add_inner( st )
      @scanner_inner  +=  "#{st}\n"
    end

## ---------------------------------------------------------------------
    def add_option( st )
      opts = st.split
      opts.each do |opt|
        case opt
        when /ignorecase/i
          @opt['--ignorecase'] = true
        when /stub/i
          @opt['--stub'] = true
        when /independent/i
          @opt['--independent'] = true
        end
      end
    end

## ---------------------------------------------------------------------
    def add_macro( st )
      ss  =  StringScanner.new(st)
      ss.scan(/\s+/)
      key = ss.scan(/\S+/)
      ss.scan(/\s+/)
      st = ss.post_match
      len  =  st.size
      ndx  =  0
      while ndx <= len
        c  =  st[ndx,1]
        ndx  +=  1
        case  c
        when '\\'
          ndx  +=  1
          next
        when '#', ' '
          ndx  -=  1
          break
        end
      end
      expr = st[0,ndx]
      expr = expr.gsub(/\\/, '\\\\\\')
      key  =  '{' + key + '}'
      @macro.each_pair do |k, e|
        expr.gsub!  k, e
      end
      @macro[key]  =  expr
    rescue
      raise ParseError, "parse error in add_macro:'#{st}'"
    end

## ---------------------------------------------------------------------
    def add_rule( rule_state, rule_expr, rule_action=nil )
      st = rule_expr.dup
      @macro.each_pair do |k, e|
        rule_expr.gsub!  k, e
      end
      if rule_state.to_s[1,1] =~ /[A-Z]/
        @exclusive_states << rule_state  unless @exclusive_states.include?(rule_state)
        exclusive_state = rule_state
        start_state = nil
      else
        exclusive_state = nil
        start_state = rule_state
      end
      rule = [exclusive_state, start_state, rule_expr, rule_action]
      @rules << rule
    rescue
      raise ParseError, "parse error in add_rule:'#{st}'"
    end

## ---------------------------------------------------------------------
    def read_grammar
      if !File.exist?(grammar_file)
        raise  Rex::RexError, "can not file open: " + grammar_file
      end
      f = File.open(grammar_file, 'r')
      @grammar_lines = StringScanner.new(f.read)
    end

## ---------------------------------------------------------------------
    def next_line
      @lineno += 1
      @grammar_lines.scan(/[^\n]*\n/).chomp
    rescue
      nil
    end

## ---------------------------------------------------------------------
    def parse
      state1  =  :HEAD
      state2  =  nil
      state3  =  nil
      lastmodes  =  []
      while st = next_line
        case state1
        when :FOOT
          add_footer  st

        when :HEAD
          ss  =  StringScanner.new(st)
          if ss.scan(/class/)
            state1  =  :CLASS
            st  =  ss.post_match.strip
            if st =~ /(\S+)::(\S+)/
              @module_name  =  $1
              @class_name  =  $2
            else
              @module_name  =  nil
              @class_name  =  st
            end
          else
            add_header  st
          end

        when :CLASS
          s = st.strip
          next  if s.size == 0 or s[0,1] == '#'

          ss  =  StringScanner.new(st)
          if ss.scan(/option.*$/)
            state2 = :OPTION
            next
          end
          if ss.scan(/inner.*$/)
            state2 = :INNER
            next
          end
          if ss.scan(/macro.*$/)
            state2 = :MACRO
            next
          end
          if ss.scan(/rule.*$/)
            state2 = :RULE
            next
          end
          if ss.scan(/end.*$/)
            state1 = :FOOT
            next
          end

          case state2
          when :OPTION
            add_option  st

          when :INNER
            add_inner  st

          when :MACRO
            add_macro  st

          when :RULE
            case state3
            when nil
              rule_state, rule_expr, rule_action  =  parse_rule(st)
              if rule_action =~ /\s*\{/
                lastmodes = parse_action(rule_action, lastmodes)
                if lastmodes.empty?
                  add_rule  rule_state, rule_expr, rule_action
                else
                  state3  =  :CONT
                  rule_action  +=  "\n"
                end
              else
                add_rule  rule_state, rule_expr
              end

            when :CONT
              rule_action  +=  "#{st}\n"
              lastmodes = parse_action(st, lastmodes)
              if lastmodes.empty?
                state3  =  nil
                add_rule  rule_state, rule_expr, rule_action
              else
              end

            end # case state3

          end # case state2

        end # case state1

      end # while

    end

## ---------------------------------------------------------------------
    def parse_rule(st)
      st.strip!
      return  if st.size == 0 or st[0,1] == '#'
      ss  =  StringScanner.new(st)
      ss.scan(/\s+/)
      rule_state  =  ss.scan(/\:\S+/)
      ss.scan(/\s+/)
      rule_expr  =  ss.scan(/\S+/)
      ss.scan(/\s+/)
      [rule_state, rule_expr, ss.post_match]
    end

## ---------------------------------------------------------------------
    def parse_action(st, lastmodes=[])
      modes  =  lastmodes
      mode  =  lastmodes[-1]
      ss  =  StringScanner.new(st)
      until ss.eos?
        c  =  ss.scan(/./)
        case  c
        when '#'
          if (mode == :brace) or (mode == nil)
            #p [c, mode, modes]
            return  modes
          end
        when '{'
          if (mode == :brace) or (mode == nil)
            mode = :brace
            modes.push  mode
          end
        when '}'
          if (mode == :brace)
            modes.pop
            mode = modes[0]
          end
        when "'"
          if (mode == :brace)
            mode = :quote
            modes.push  mode
          elsif (mode == :quote)
            modes.pop
            mode = modes[0]
          end
        when '"'
          if (mode == :brace)
            mode = :doublequote
            modes.push  mode
          elsif (mode == :doublequote)
            modes.pop
            mode = modes[0]
          end
        when '`'
          if (mode == :brace)
            mode = :backquote
            modes.push  mode
          elsif (mode == :backquote)
            modes.pop
            mode = modes[0]
          end
        end
      end
      #p [c, mode, modes]
      return  modes
    end

## ---------------------------------------------------------------------

REX_HEADER = <<-REX_EOT
#
# DO NOT MODIFY!!!!
# This file is automatically generated by rex %s
# from lexical definition file "%s".
#

REX_EOT

REX_UTIL = <<-REX_EOT
  require 'strscan'

  class ScanError < StandardError ; end

  attr_reader :lineno
  attr_reader :filename

  def scan_setup ; end

  def action &block
    yield
  end

  def scan_str( str )
    scan_evaluate  str
    do_parse
  end

  def load_file( filename )
    @filename = filename
    open(filename, "r") do |f|
      scan_evaluate  f.read
    end
  end

  def scan_file( filename )
    load_file  filename
    do_parse
  end

REX_EOT

REX_NEXTTOKEN_DEBUG = <<-REX_EOT
  def next_token
    p token = @rex_tokens.shift
    token
  end
REX_EOT

REX_NEXTTOKEN = <<-REX_EOT
  def next_token
    @rex_tokens.shift
  end
REX_EOT

REX_STUB = <<-REX_EOT

if __FILE__ == $0
  exit  if ARGV.size != 1
  filename = ARGV.shift
  rex = %s.new
  begin
    rex.load_file  filename
    while  token = rex.next_token
      p token
    end
  rescue
    $stderr.printf  %s, rex.filename, rex.lineno, $!.message
  end
end
REX_EOT

## ---------------------------------------------------------------------
    def write_scanner
      unless  scanner_file = @opt['--output-file']
        scanner_file  =  grammar_file + ".rb"
      end
      f = File.open(scanner_file, 'w')

      ## scan flag
      flag = ""
      flag += "i"  if @opt['--ignorecase']
      ## header
      f.printf REX_HEADER, Rex::Version, grammar_file

      unless @opt['--independent']
        f.printf "require 'racc/parser'\n"
      end

      @scanner_header.each_line do |s|
        f.print s
      end
      if @module_name
        f.puts "module #{@module_name}"  
      end
      if @opt['--independent']
        f.puts "class #{@class_name}"
      else
        f.puts "class #{@class_name} < Racc::Parser"
      end

      ## utility method
      f.print REX_UTIL

      if @opt['--debug']
        f.print REX_NEXTTOKEN_DEBUG
      else
        f.print REX_NEXTTOKEN
      end

      ## scanner method

      f.print <<-REX_EOT

  def scan_evaluate( str )
    scan_setup
    @rex_tokens = []
    @lineno  =  1
    ss = StringScanner.new(str)
    state = nil
    until ss.eos?
      text = ss.peek(1)
      @lineno  +=  1  if text == "\\n"
      case state
      REX_EOT

      exclusive_states.each do |es|
        f.printf <<-REX_EOT
      when #{es ? es.to_s : "nil"}
        current_match_len     = 0
        current_match_expr    = nil 
        current_match_action  = nil 
        REX_EOT
        rules.each do |rule|
          exclusive_state, start_state, rule_expr, rule_action = *rule
          if es == exclusive_state

            if rule_action
              if start_state
                f.print <<-REX_EOT
        if (state == #{start_state}) and ((text = ss.check(/#{rule_expr}/#{flag})) and ss.matched_size > current_match_len  )
           current_match_len    = ss.matched_size
           current_match_expr   = /#{rule_expr}/#{flag}
           current_match_action = action #{rule_action}
        end

                REX_EOT
              else
                f.print <<-REX_EOT
        if ((text = ss.check(/#{rule_expr}/#{flag})) and ss.matched_size > current_match_len )
           current_match_len    = ss.matched_size
           current_match_expr   = /#{rule_expr}/#{flag}
           current_match_action = action #{rule_action}
        end

                REX_EOT
              end
            else
              if start_state
                f.print <<-REX_EOT
        if (state == #{start_state}) and (text = ss.check(/#{rule_expr}/#{flag})) and ss.matched_size > current_match_len )
          current_match_len    = ss.matched_size
          current_match_expr   = /#{rule_expr}/#{flag}
        end

                REX_EOT
              else
                f.print <<-REX_EOT
        if (( text = ss.check(/#{rule_expr}/#{flag})) and ss.matched_size > current_match_len )
          current_match_len    = ss.matched_size
          current_match_expr   = /#{rule_expr}/#{flag}
        end

                REX_EOT
              end
            end

          end
        end
        f.print <<-REX_EOT
        if current_match_len == 0 
          text = ss.string[ss.pos .. -1]
          raise  ScanError, "can not match: '" + text + "'"
        else
          text = ss.scan( current_match_expr )
          if current_match_action 
            @rex_tokens.push current_match_action
          end
        end

        REX_EOT
      end
      f.print <<-REX_EOT
      else
        raise  ScanError, "undefined state: '" + state.to_s + "'"
      end  # case state
    end  # until ss
  end  # def scan_evaluate

      REX_EOT

      ## inner method
      @scanner_inner.each_line do |s|
        f.print s
      end
      f.puts "end # class"
      f.puts "end # module"  if @module_name

      ## footer
      @scanner_footer.each_line do |s|
        f.print s
      end # case

      ## stub main
      f.printf REX_STUB, @class_name, '"%s:%d:%s\n"'  if @opt['--stub']

    end ## def
  end ## class
end ## module


## ---------------------------------------------------------------------
## test

if __FILE__ == $0
  rex = Rex::Generator.new(nil)
  rex.grammar_file = "sample.rex"
  rex.read_grammar
  rex.parse
  rex.write_scanner
end

