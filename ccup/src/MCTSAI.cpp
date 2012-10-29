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

#include "RandomAI.h"
#include "MCTSAI.h"
#include "Timer.h"
#include "Logger.h"

#include <sstream>
#include <fstream>


MCTSAI::MCTSAI()
    : AI(),
      m_move_number( 0 ),
      m_root( new Node( 0 ) )
{
}


MCTSAI::~MCTSAI()
{
}


void MCTSAI::chooseMove( 
            Color color,
            const Board& board,
            Move& move
            )
{
    m_move_number++;
    
    std::cerr << " sim board :" << m_move_number << std::endl << m_root->board() << std::endl;

    /*
    RandomAI random_ai;
    random_ai.chooseMove( color, board, move );
    return;
    */

    const double time_allowed = 0.05;
    Timer timer;
    timer.start();
    
    std::vector< Node* > visited;
    unsigned iter_count = 0u;
    unsigned max_depth = 0u;
    while( timer.getTimeElapsed() < time_allowed )
    {
        iter_count++;
        LDEBUG << " Iteration: " << iter_count << std::endl;

        /*
        if( iter_count % 10 == 0 )
        {
            std::ostringstream oss;
            oss << "graph_" << m_move_number << "_" << iter_count << ".dot";
            printGraph( m_root, oss.str() );
        }
        */

        //
        // SELECT: Walk tree, selecting moves until we hit never before seen pos
        //
        LDEBUG << "    SELECT: ";
        visited.clear();
        visited.push_back( m_root ); // TODO: use this

        Node* cur  = m_root;
        Node* next = 0;

        unsigned depth = 1u;
        while( cur->select( &next, color ) )
        {
            depth++;
            cur = next;
            visited.push_back( cur );

            if( cur->board().gameFinished() )
            {
                LDEBUG << "Reached finished game state during select!";
                break;
            }
        }
        if( depth > max_depth )
            max_depth = depth;

        //
        // EXPAND: add new node to  the tree
        //
        LDEBUG << "    EXPAND";
        cur->addChild( next );

        //
        // SIMULATE: Run simulation
        //
        LDEBUG << "    SIMULATE";
        RandomAI  random_ai;
        Board     sim_board = next->board();
        Color     cur_color = color;
        Move      move;
        while( !sim_board.gameFinished() )
        {
            random_ai.chooseMove( cur_color, sim_board, move );
            sim_board.set( move, cur_color );

            cur_color = (cur_color == WHITE ? BLACK : WHITE );
        }

        assert( sim_board.winner() != NONE );
        const int score = sim_board.winner() == color ? 1 : 0;

        //
        // PROPAGATE: Back propagate the score
        //
        LDEBUG << "    PROPAGATE";
        next->addResult( score );
        Node* parent = next->parent();
        while( parent )
        {
            parent->addResult( score );
            parent = parent->parent();
        }
    }

    std::cerr << "iter count: " << iter_count << std::endl;
    std::cerr << "max_depth : " << max_depth << std::endl;

    std::ostringstream oss;
    oss << "graph_" << m_move_number << "_final.dot";
    printGraph( m_root, oss.str() );

    Node* new_root = m_root->bestMove();
    new_root->setParent( 0 );

    // Delete unused portion of tree
    for( std::vector<Node*>::iterator it = m_root->children().begin();
         it != m_root->children().end();
         ++it )
    {
        Node* child = *it;
        if( child != new_root )
        {
            child->deleteTree();
            delete child;
        }
    }

    move.assign( new_root->move().begin(), new_root->move().end() );
    
    delete m_root;
    m_root = new_root;
}


void MCTSAI::printGraph( const Node* node, const std::string& filename )
{
    std::ofstream out( filename.c_str() );
    out << "digraph g {\n"
        << "  graph [ size=\"8.0,10.5\""
        << " bgcolor=white"
        << " overlap=false"
        << " splines=true"
        << " labelloc=\"t\"];\n";

    out << "  node [fontname=\"Courier\","
        << "fontsize=10,"
        << "textalign=left,"
        << "shape=Mrecord];\n";

    printGraphNode( node, out );

    out <<"}";
}
    

void MCTSAI::printGraphNode( const Node* n, std::ostream& out )
{
    out << "  " << reinterpret_cast<unsigned long long>( n )
        << " [style=filled,fillcolor=\"#FFFFFF\",labeljust=l,label=\"Move: ";

    for( Move::const_iterator it = n->move().begin();
         it != n->move().end();
         ++it )
    {
        out << toString( it->first, it->second );
        if( it + 1 != n->move().end() )
            out << "-";
    }
    out << "\\n";
    out << "Wins  : " << n->numWins() << "\\n";
    out << "Visits: " << n->numVisits();
    out << "\"];\n";

    for( std::vector<Node*>::const_iterator it = n->children().begin();
         it != n->children().end();
         ++it )
    {
        out << "  " 
            << reinterpret_cast<unsigned long long>( n ) << " -> " 
            << reinterpret_cast<unsigned long long>( *it ) << ";\n";
    }
    
    for( std::vector<Node*>::const_iterator it = n->children().begin();
         it != n->children().end();
         ++it )
    {
        printGraphNode( *it, out );
    }
}


MCTSAI::Node* MCTSAI::Node::bestMove()
{
    assert( !m_children.empty() );

    float max_score = -1.0f; 
    Node* best = 0;

    for( std::vector<Node*>::iterator it = m_children.begin();
         it != m_children.end();
         ++it )
    {
        Node* child = *it;
        if( child->score() > max_score )
        {
            best = child;
            max_score = child->score();
        }
    }
    return best;
}


float MCTSAI::Node::score()const
{ 
    assert( m_num_visits ); 
    return static_cast<float>( m_num_wins ) / 
           static_cast<float>( m_num_visits );
}


void MCTSAI::Node::deleteTree()
{
    for( std::vector<Node*>::iterator it = m_children.begin();
         it != m_children.end();
         ++it )
    {
        Node* child = *it;
        child->deleteTree();
        delete child;
    }
}


bool MCTSAI::Node::select( Node** node, Color c )
{
    LDEBUG << "        selecting from " << m_children.size() << " children";

    const float new_node_uct_score = uct( 0, 0, m_num_visits ); 
    float best_node_uct_score = -1e16f; //std::numeric_limits<float>::min();
    Node* best_node = 0;
        
    LDEBUG << "        initial UCT score " << new_node_uct_score;

    // If we have surpassed the threshold, find the best existing uct score
    const size_t T = 4;
    if( m_children.size() > T ) 
    {
        for( std::vector<Node*>::iterator it = m_children.begin();
             it != m_children.end();
             ++it )
        {
            Node* c = *it;
            float uct_score = uct( c->score(), c->numVisits(), m_num_visits );
            LDEBUG << "        new UCT score " << uct_score;
            if( uct_score > best_node_uct_score )
            {
                best_node = c;
                best_node_uct_score = uct_score;
            }
        }
    }

    if( new_node_uct_score > best_node_uct_score )
    {
        //
        // Expand the tree here
        //

        Node* n = new Node( this ); 

        const float p_explore = 0.5f; // TODO:  make some function of mov
        if( m_board.numStones( c ) == 0 || drand48() < p_explore )
        {
            if( chooseExploration( c, m_board, m_explorations, n->m_move ) )
            {
                *node = n;
                n->m_board.set( n->m_move, c );
                return false;
            }
        }
        else
        {
            if( chooseExpansion( c, m_board, m_expansions, n->m_move ) )
            {
                *node = n;
                n->m_board.set( n->m_move, c );
                return false;
            }
        }

        delete n;    
    }

    // If a preexisting node was selected
    assert( best_node );
    *node = best_node;;
    return true;
}

