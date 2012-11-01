//
// MIT License
//
// Copyright (c) 2008 r. keith morley 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE
//


//------------------------------------------------------------------------------
//
//  main
//
//------------------------------------------------------------------------------

#include "Logger.h"
#include "Player.h"
#include "Timer.h"
#include "Board.h"

#include <iostream>
#include <iterator>
#include <sstream>

int main( int argc, char** argv )
{
    if( argc > 1 && std::string( argv[1] ) == "-v" )
        Log::setReportingLevel( Log::DEBUG );
    else if( argc > 1 && std::string( argv[1] ) == "-V" )
        Log::setReportingLevel( Log::DEBUG1 );
    else
        Log::setReportingLevel( Log::INFO );
    
    Player player;

    LoopTimerInfo main_loop_time( "Main loop" );
    std::vector< std::string > opponent_moves;
    while( true )
    {
        std::string opponent_move;
        std::cin >> opponent_move;

        AutoTimerRef<LoopTimerInfo> schedule_timer( main_loop_time );

        opponent_moves.push_back( opponent_move );

        if( std::cin.eof() || opponent_move == "Quit" )
            break;

        std::string my_move = player.doMove( opponent_move );

        LINFO << "opp move: " << opponent_move;
        LINFO << "my move : " << my_move;
        LINFO << "\n" << player.board();
        std::cout << my_move << std::endl;
        std::cout.flush();
    }

    std::cerr << "WHITE: " << player.board().score( WHITE ) << "\n"
              << "BLACK: " << player.board().score( BLACK ) << "\n";
    main_loop_time.log();

    std::cerr << "REPLAY ------------------------------------\n";
    std::ostream_iterator< std::string > out( std::cerr, "\n" );
    std::copy( opponent_moves.begin(), opponent_moves.end(), out );
}



