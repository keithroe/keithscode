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

#ifndef CCUP_MCTSAI_H__
#define CCUP_MCTSAI_H__

#include "AI.h"
#include "Board.h"

//------------------------------------------------------------------------------
//
// MCTSAI 
//
//------------------------------------------------------------------------------

class MCTSAI : public AI
{
public:
    MCTSAI();

    ~MCTSAI();


    void chooseMove( 
            Color color,
            const Board& board,
            Move& move
            );

    class Node
    {
    public:
        Node( Node* parent )
            : m_parent( parent ),
              m_num_wins( 0 ),
              m_num_visits( 0 ),
              m_explorations( NUM_GRID_CELLS ),
              m_expansions( NUM_GRID_CELLS )
        {
            for( int i = 0; i < NUM_GRID_CELLS; ++i )
                m_explorations[i] = m_expansions[i] = i;

            std::random_shuffle( m_explorations.begin(), m_explorations.end() );
            std::random_shuffle( m_expansions.begin(), m_expansions.end() ); 
        }

        const Board& board()const       { return m_board; }
        Board& board()                  { return m_board; }

        int numWins()const              { return m_num_wins;   }
        int numVisits()const            { return m_num_visits; }

        void addResult( int score )     { ++m_num_visits; m_num_wins += score; }

        void addChild( Node* child )    { m_children.push_back( child ); }

        void  setParent( Node* parent ) { m_parent = parent; }
        Node* parent()                  { return m_parent; }

        const Move& move()const         { return m_move; }

        const std::vector<Node*>& children()const { return m_children; }
        std::vector<Node*>& children()            { return m_children; }

        Node* bestMove();

        float score()const;             

        void deleteTree();

        bool select( Node** next, Color color );

    private:
        Node*              m_parent;
        Move               m_move;
        int                m_num_wins;
        int                m_num_visits;
        Board              m_board;

        std::vector<Node*>          m_children;
        std::vector<unsigned char>  m_explorations;
        std::vector<unsigned char>  m_expansions;
    };

    static void printGraph( const Node* node, const std::string& filename );
    static void printGraphNode( const Node* node, std::ostream& out );

    unsigned m_move_number;
    Node* m_root;

};


#endif //CCUP_MCTSAI_H__
