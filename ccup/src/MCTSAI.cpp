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

#include "MCTSAI.h"
#include "Timer.h"
#include "Logger.h"

#include <fstream>
#include <limits>
#include <sstream>


//------------------------------------------------------------------------------
//
// Node class
//
//------------------------------------------------------------------------------

class Node
{
public:
    typedef std::vector<Node*> Children; 

    explicit Node( Color color );

    Node( Node* parent, const Move& move );

    const Board& board()const        { return m_board; }
    Board& board()                   { return m_board; }

    int accumScore()const            { return m_accum_score;   }
    int numVisits()const             { return m_num_visits; }

    void addResult( int score )      { ++m_num_visits; m_accum_score += score; }

    void addChild( Node* child )     { m_children.push_back( child ); }

    void  setParent( Node* parent )  { m_parent = parent; }
    Node* parent()                   { return m_parent; }

    const Color color()const         { return m_color; }
    const Move& move()const          { return m_move; }

    const Children& children()const  { return m_children; }
    //Children& children()             { return m_children; }

    Node* bestMove();

    float score()const;             

    void deleteTree();

    bool select( Node** next, Color ai_color );

private:
    void createNewMove( Move& move );
    bool selectAIMove( Node** node );
    bool selectOppMove( Node** node );

    Node*              m_parent;
    Color              m_color;
    Move               m_move;
    int                m_accum_score;
    int                m_num_visits;
    Board              m_board;

    Children                    m_children;
    std::vector<unsigned char>  m_explorations;
    std::vector<unsigned char>  m_expansions;

    const static unsigned T = 4;  // Minimal expansion threshold
};


Node::Node( Color color )
    : m_parent( 0 ),
      m_color( color ),
      m_accum_score( 0 ),
      m_num_visits( 0 ),
      m_explorations( NUM_GRID_CELLS ),
      m_expansions( NUM_GRID_CELLS )
{
    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        m_explorations[i] = m_expansions[i] = i;

    std::random_shuffle( m_explorations.begin(), m_explorations.end() );
    std::random_shuffle( m_expansions.begin(), m_expansions.end() ); 
}


Node::Node( Node* parent, const Move& move )
    : m_parent( parent ),
      m_color( parent->m_color == WHITE ? BLACK : WHITE ),
      m_move( move ),
      m_accum_score( 0 ),
      m_num_visits( 0 ),
      m_board( parent->board() ),
      m_explorations( NUM_GRID_CELLS ),
      m_expansions( NUM_GRID_CELLS )
{
    m_board.set( move, m_color );

    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        m_explorations[i] = m_expansions[i] = i;

    std::random_shuffle( m_explorations.begin(), m_explorations.end() );
    std::random_shuffle( m_expansions.begin(), m_expansions.end() ); 
}


Node* Node::bestMove()
{
    assert( !m_children.empty() );

    float max_score = -std::numeric_limits<float>::max(); 
    Node* best = 0;

    for( Children::iterator it = m_children.begin();
         it != m_children.end();
         ++it )
    {
        // TODO: use safety selection rather than best score
        Node* child = *it;
        if( child->score() > max_score )
        {
            best = child;
            max_score = child->score();
        }
    }
    return best;
}


float Node::score()const
{ 
    assert( m_num_visits ); 
    return static_cast<float>( m_accum_score ) / 
           static_cast<float>( m_num_visits );
}


void Node::deleteTree()
{
    for( Children::iterator it = m_children.begin();
         it != m_children.end();
         ++it )
    {
        Node* child = *it;
        child->deleteTree();
        delete child;
    }
}

void Node::createNewMove( Move& move )
{
    LDEBUG << "Create new move ...";
    move.clear();
    Color move_color = otherColor( m_color );

    bool explore_valid = !m_explorations.empty();
    bool expand_valid  = m_board.numStones( move_color ) != 0;
    if( !explore_valid && !expand_valid )
        return;

    if( explore_valid && ( !expand_valid || drand48() < 0.7 ) )
    {
        LDEBUG << "\tTrying exploration ...";
        chooseRandomExploration( move_color, m_board, m_explorations, move );
    }

    if( expand_valid && move.empty() )
    {
        LDEBUG << "\tTrying expansion... ";
        std::random_shuffle( m_expansions.begin(), m_expansions.end() );
        if( !chooseRandomExpansion( move_color, m_board, m_expansions, move ) )
            chooseRandomExploration(move_color, m_board, m_explorations, move);

    }

    if( !move.empty() )
    {
        LDEBUG << "\t\t\tSUCCESS";

        // Now check for duplicate move
        // TODO: there could still be a duplicate board with expansions
        //       with different move orders
        for( Children::iterator it = m_children.begin();
             it != m_children.end();
             ++it )
        {
            Node* child = *it;
            if( move == child->move() )
            {
                LDEBUG << "\t\t\t\tDUPLICATE MOVE -- clearing";
                move.clear();
                break;
            }
        }
    }
}

bool Node::selectAIMove( Node** node )
{
    // TODO: handle finished games -- 
    LDEBUG << "\tselectAI from " << m_children.size() << " children";

    //
    // We are selecting our own move, choose optimal score
    //
    
    float best_score   = -std::numeric_limits<float>::max(); 
    Node* best_node    = 0;

    for( Children::iterator it = m_children.begin();
         it != m_children.end();
         ++it )
    {
        Node* c = *it;

        float score = uct( c->score(), c->numVisits(), m_num_visits );
        LDEBUG << "\t\tchild score: " << score;
        if( score > best_score )
        {
            best_node  = c;
            best_score = score;
        }
    }

    float expand_score = uct( 0, 0, m_num_visits );

    LDEBUG << "\tselectAI - bestscore: " << best_score << " newscore: " 
           << expand_score;
    //if( m_children.size() < T || expand_score >= best_score )
    if( expand_score >= best_score )
    {
        Move move;
        createNewMove( move );
        if( !move.empty() )
        {
            LDEBUG << "\t\tcreated new move " << toString( move );
            *node = new Node( this, move );
            return false;
        }
        LDEBUG << "\t\tcreate new move FAILED" << toString( move );
    }

    LDEBUG << "\tUsing best preexisting move " << toString( best_node->move() );
    assert( best_node );
    *node = best_node;
    return true;
}


bool Node::selectOppMove( Node** node )
{
    LDEBUG << "\tselectOpp from " << m_children.size() << " children";

    //
    // We are selecting our opponent's move, choose pessimal score
    //

    float best_score   = std::numeric_limits<float>::max();
    Node* best_node    = 0;

    for( Children::iterator it = m_children.begin();
         it != m_children.end();
         ++it )
    {
        Node* c = *it;

        float score = uctOpp( c->score(), c->numVisits(), m_num_visits );
        if( score < best_score )
        {
            best_node  = c;
            best_score = score;
        }
    }

    float expand_score = uctOpp( 0, 0, m_num_visits );
    LDEBUG << "\tselectOpp - bestscore: " << best_score << " newscore: " 
           << expand_score;
    //if m_children.size() < T || expand_score <= best_score )
    if( expand_score <= best_score )
    {
        Move move;
        createNewMove( move );
        if( !move.empty() )
        {
            LDEBUG << "\t\tcreated new move " << toString( move );
            *node = new Node( this, move );
            return false;
        }
        LDEBUG << "\t\tcreate new move FAILED" << toString( move );
    }

    LDEBUG << "\tUsing best preexisting move " << toString( best_node->move() );
    assert( best_node );
    *node = best_node;
    return true;
}


bool Node::select( Node** node, Color ai_color )
{
    //LDEBUG1 << "selecting on board:\n" << m_board;
    if( m_color != ai_color )
    {
        LDEBUG << "selecting for AI...";
        return selectAIMove( node );
    }
    else
    {
        LDEBUG << "selecting for OPP...";
        return selectOppMove( node );
    }
}

//------------------------------------------------------------------------------
//
//  Helpers 
//
//------------------------------------------------------------------------------

namespace 
{
    void printGraphNode( const Node* n, std::ostream& out )
    {
        std::string color = n->color() == WHITE ? 
                            "#FFFFFF"           :
                            "#AAAAAA"           ;
        out << "  " << reinterpret_cast<unsigned long long>( n )
            << " [style=filled,fillcolor=\"" << color 
            << "\",labeljust=l,label=\"Move:";

        for( Move::const_iterator it = n->move().begin();
             it != n->move().end();
             ++it )
        {
            out << toString( it->first, it->second );
            if( it + 1 != n->move().end() )
                out << "-";
        }
        out << "\\n";
        out << "Accum score: " << n->accumScore() << "\\n";
        out << "Visits     : " << n->numVisits();
        out << "\"];\n";

        for( Node::Children::const_iterator it = n->children().begin();
             it != n->children().end();
             ++it )
        {
            out << "  " 
                << reinterpret_cast<unsigned long long>( n ) << " -> " 
                << reinterpret_cast<unsigned long long>( *it ) << ";\n";
        }
        
        for( Node::Children::const_iterator it = n->children().begin();
             it != n->children().end();
             ++it )
        {
            printGraphNode( *it, out );
        }
    }


    void printGraph( const Node* node, const std::string& filename )
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
}


//------------------------------------------------------------------------------
//
// MCTSAI class
//
//------------------------------------------------------------------------------

MCTSAI::MCTSAI()
    : AI(),
      m_root( 0 ),
      m_time_budget( 0.75 ) // seconds
{
}


MCTSAI::~MCTSAI()
{
    if( m_root )
    {
        m_root->deleteTree();
        delete m_root;
    }
}



void MCTSAI::doGetMove( Move& move )
{
    // If this is our first move, we need to create root node
    if( !m_root )
    {
        m_root = new Node( m_color );
    }

    if( m_opp_moves.size() )
    {
        updateTreeWithOppMove( m_opp_moves.back() );
    }

    assert( m_root->board() == m_board );

    LDEBUG << "MCTSAI board :" << m_move_number << "\n" << m_root->board();

    //
    // Update our random walk seeds
    //
    std::vector<unsigned char> exploration_seeds;
    exploration_seeds.reserve( NUM_GRID_CELLS );

    std::vector<unsigned char> expansion_seeds( NUM_GRID_CELLS );
    for( int i = 0; i < NUM_GRID_CELLS; ++i )
        expansion_seeds[i] = i;

    Timer timer;
    timer.start();
    
    unsigned iter_count = 0u;
    unsigned max_depth = 0u;

    //while( timer.getTimeElapsed() < m_time_budget )
    while( iter_count < 5000 )
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

        int score; 
        bool  game_finished = false;
        Node* cur  = m_root;
        Node* next = 0;


        unsigned depth = 1u;
        while( cur->select( &next, m_color ) )
        {
            if( next->board().gameFinished() )
            {
                // TODO: handle this finished game state better
                LDEBUG << "Reached finished game state during select!";
                score = next->board().winner() == m_color ? 1 : -1;
                game_finished = true;
                break;
            }

            depth++;
            cur = next;
        }

        if( depth > max_depth )
            max_depth = depth;

        if( !game_finished )
        {
            //
            // EXPAND: add new node to  the tree
            //
            LDEBUG << "    EXPAND";
            cur->addChild( next );

            //
            // SIMULATE: Run simulation TODO: funcify
            //
            LDEBUG << "    SIMULATE";
            Board     sim_board = next->board();
            Color     cur_color = next->color();
            Move      move;

            exploration_seeds.resize( NUM_GRID_CELLS );
            for( int i = 0; i < NUM_GRID_CELLS; ++i )
                exploration_seeds[i] = i;
            
            std::random_shuffle( exploration_seeds.begin(),
                                 exploration_seeds.end() ); 
            std::random_shuffle( expansion_seeds.begin(),
                                 expansion_seeds.end() ); 

            while( !sim_board.gameFinished() )
            {
                cur_color = ( cur_color == WHITE ? BLACK : WHITE );
                chooseRandomMove( cur_color, sim_board, 0.5f,
                                  expansion_seeds, exploration_seeds,
                                  move );
                sim_board.set( move, cur_color );
            }

            /*
            LDEBUG << "sim finished.  winner is: " << sim_board.winner();
            LDEBUG << "\t: " << sim_board.score( m_color ) << "-"
                             << sim_board.score( otherColor( m_color ) );
            LDEBUG << "\n" << sim_board;
            */
            assert( sim_board.winner() != NONE );
            score = sim_board.winner() == m_color ? 1 : -1;
        }

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

#ifdef LOCAL
    std::ostringstream oss;
    oss << "graph_" << m_move_number << "_final.dot";
    ::printGraph( m_root, oss.str() );
#endif //LOCAL

    Node* new_root = m_root->bestMove();
    new_root->setParent( 0 );

    // Delete unused portion of tree
    for( Node::Children::const_iterator it = m_root->children().begin();
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


void MCTSAI::updateTreeWithOppMove( const Move& move )
{
    Node* new_root = 0;

    for( Node::Children::const_iterator it = m_root->children().begin();
         it != m_root->children().end();
         ++it )
    {
        Node* child = *it;
        if( child->board() == m_board )
        {
            LDEBUG << "*****************Reusing oppmove tree!!!!!";
            new_root = child;
            new_root->setParent( 0 );
        }
        else
        {
            child->deleteTree();
            delete child;
        }
    }

    if( !new_root )
    {
        new_root = new Node( m_root, move );
        new_root->setParent( 0 );
    }

    delete m_root;
    m_root = new_root;
}


