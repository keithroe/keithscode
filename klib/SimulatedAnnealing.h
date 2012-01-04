
#ifndef SIMULATED_ANNEALING_H_
#define SIMULATED_ANNEALING_H_


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
        : initial_greediness( 0.5f * initial_greediness )
    {
    }





private:

};


//
// Evaluate:
//     * Needs to evaluate the 'energy' of a given node
//     * identify a perfect sol'n
//     * Evaluate::Score is used for scores
// CoolingSchedule:
//     * Temp should be decreasing and in [1.0, 0.0]
//     * Report when cooling is finished
// P:
//     * P( score, score_prime, Temp ) should decrease as Temp decreases 
//     * Must accept Evaluate::Score as input for P
//
template< typename Graph,
          typename Evaluate,
          typename CoolingSchedule = LinearCooling,
          typename TransitionP = SimpleTransitionP >
class SimulatedAnnealing
{
public:
    SimulatedAnnealing( Graph&          graph,
                        Evaluate        evaluate,
                        CoolingSchedule cooling_schedule,
                        TransitionP     transition_p );

    void run( const Graph::Node& initial_node, Graph::Node& solution );

private:


    Graph&          m_graph;
    Evaluate        m_evaluate;
    CoolingSchedule m_cooling_schedule;
    TransitionP     m_transition_p;
};


#endif // SIMULATED_ANNEALING_H_
