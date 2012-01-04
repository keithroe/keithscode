
#ifndef SIMULATED_ANNEALING_H_
#define SIMULATED_ANNEALING_H_

// TODO: allow arbitrary Score type -- right now hardcoded to float

class LinearCoolingSchedule
{
public:
    LinearCoolingSchedule( unsigned max_iterations )
        : m_max_steps( static_cast<float>( max_steps ) ),
          m_current_step( 0.0f )
    {
    }

    float step()
    { 
        m_current_step += 1.0f;
        return ( m_max_steps - m_current_step ) / m_max_steps;
    }

    bool finished()const
    {
        return m_current_step >= m_max_steps;
    }

private:
    const float m_max_steps;
    float m_current_step;
};


class SimpleTransitionP 
{
public:
    // Initial greediness should be in [0,1].
    //   * Initial greediness of zero; P starts at .5/.5 transition
    //     probablilities for score improving/worsening transitions respectively
    //   * Initial greediness of one; P starts at 1.0/0.0 (fully greeedy).
    //   * In either case, P moves linearly to fully greedy by end of cooling.
    SimpleTransitionP( float initial_greediness )
        : m_initial_greediness( 0.5f * initial_greediness )
    {
        assert( initial_greediness <=  1.000001f );
        assert( initial_greediness >= -0.000001f );
    }


    float get( float s0, float s1, float temperature )
    {
        float t = lerp( m_initial_greediness, 0.0f, temperature );
        if( s0 > s1 ) return 1.0f-t;
        else          return 0.0f+t
    }

private:
    static inline float lerp( float a, float b, float t )
    { return a + t*( b-a ); }

    float m_initial_greediness;
};


//
// Graph:
//     * Needs to evaluate the 'energy' of a given state
//     * identify a perfect sol'n
//     * Graph::State is a state in the search space
//     * Graph::Energy represents energy of a State
//     * Graph::transition( Graph::State ) modifies the state in place to a
//       neighbor state
// CoolingSchedule:
//     * Temp should be decreasing and in [1.0, 0.0]
//     * Report when cooling is finished
// P:
//     * P( score, score_prime, Temp ) should decrease as Temp decreases 
//     * Must accept Graph::Score as input for P
//
template< typename Graph,
          typename CoolingSchedule = LinearCooling,
          typename TransitionP = SimpleTransitionP >
class SimulatedAnnealing
{
public:
    SimulatedAnnealing( Graph&          graph,
                        CoolingSchedule cooling_schedule,
                        TransitionP     transition_p )
        : m_graph( graph ),
          m_cooling_schedule( cooling_schedule ),
          m_transition_p( transition_p )
    {
    }


    void run( const Graph::State& initial_state, Graph::State& solution_state )
    {
        solution_state              = initial_state;
        Graph::Energy lowest_energy = m_graph.evaluate( initial_state ); 

        Graph::State  current_state  = initial_state;
        Graph::Energy current_energy = lowest_energy;
        while( !m_graph.isOptimal( best_score ) && !m_cooling_schedule.isFinished() )
        {
            m_graph.transition( current_state );
            Graph::Energy new_energy = m_graph.evaluate( current_state );

            if( current_energy < lowest_energy )
            {
                lowest_energy  = current_energy;
                solution_state = current_state;
            }

            float temperature = m_cooling_schedule.step();
            float p = m_transition_p.get( last_score, current_score, temperature );
            float e = drand48();
            if( p > e )
            {
                cur_energy = new_energy;
            }
            else
            {
                m_graph.revert( current_state );
            }
        }
    }

private:
    Graph&          m_graph;
    CoolingSchedule m_cooling_schedule;
    TransitionP     m_transition_p;
};


#endif // SIMULATED_ANNEALING_H_
