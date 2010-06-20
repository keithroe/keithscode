
/*****************************************************************************\ 
 *  
 *  filename: Face.h
 *  author  : r. keith morley
 *
 *  Changelog:
 *
\*****************************************************************************/


#ifndef _FACE_H_
#define _FACE_H_

#include <mesh/HalfEdge.h>
#include <math/Vector3.h>

class  Face
{
public:
    Face();
    Face( HalfEdge* edge ) : _edge(edge)                       { }
    Face( HalfEdge* edge, int id ) : _edge(edge), _id(id)      { }
    ~Face();
    
    HalfEdge* edge()                         { return _edge; }
    Vector3   normal()                       { return _norm; }
    void      setEdge( HalfEdge* edge )      { _edge = edge; }
    void      setNormal( const Vector3& n)   { _norm = n; }
    int       id()                           { return _id; }
    void      setID( int id )                { _id = id; }

    void      computeNormal();

    bool      checkConsistent();

private:
    HalfEdge*       _edge;
    Vector3         _norm;
    int             _id;
};


#endif // _FACE_H_
