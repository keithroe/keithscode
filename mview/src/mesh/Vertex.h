
/*****************************************************************************\ 
 *  
 *  filename: Vertex.h
 *  author  : r. keith morley
 *
 *  Changelog:
 *
\*****************************************************************************/


#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <math/Vector3.h>
#include <mesh/HalfEdge.h>


class  Vertex
{
public:
    Vertex()                                : _edge(NULL) {}
    Vertex( const Vector3& c )              : _edge(NULL), _coord(c) {}
    Vertex( HalfEdge* e, const Vector3& c ) : _edge(e), _coord(c) {}
    Vertex( HalfEdge* e, const Vector3& c, int id ) 
        : _edge(e), _coord(c), _id(id)  {}

    ~Vertex() {}

    HalfEdge* edge()                          { return _edge; }
    void      setEdge( HalfEdge* edge )       { _edge = edge; }
    Vector3&  coord( )                        { return _coord; }
    void      setCoord( const Vector3& c)     { _coord = c; }
    Vector3&  normal( )                       { return _norm; }
    void      setNormal( const Vector3& n)    { _norm = n; }
    int       id()                            { return _id; }
    void      setID( int id )                 { _id = id; }
    void      computeNormal();
    HalfEdge* findFirstEdge();

    bool checkConsistent();

private:
    HalfEdge*   _edge;
    Vector3     _coord;
    Vector3     _norm;
    int         _id;
};


#endif // _VERTEX_H_
