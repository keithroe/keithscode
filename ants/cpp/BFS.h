
#ifndef BFS_H_
#define BFS_H_

class BFS
{
public:
    typedef bool (*Predicate)( const Square& square );
    AStar( const Map& map, const Location& start_loc, Predicate predicate );

    void setMaxDepth( unsigned max_depth )   { m_max_depth = max_depth; }

    bool search();

    void getPath( Path& path )const;
private:
    bool step();
};

#endif // BFS_H_
