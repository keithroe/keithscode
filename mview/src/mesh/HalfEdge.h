
/*****************************************************************************\ 
 *  
 *  filename: HalfEdge.h
 *  author  : r. keith morley
 *
 *  Changelog:
 *
\*****************************************************************************/


#ifndef _HALFEDGE_H_
#define _HALFEDGE_H_

#include <utility>

class Vertex;
class Face;

class  HalfEdge
{
public:
    ~HalfEdge();
    HalfEdge(); 
    HalfEdge( Vertex* origin, Face* face ); 
    HalfEdge( HalfEdge* twin, HalfEdge* next, 
              Vertex* origin, Face* face ); 
    
    typedef std::pair<HalfEdge*, HalfEdge*> EdgePair;

    static EdgePair createEdgePair();
    static EdgePair createEdgePair( Vertex* origin1, Vertex* origin2 );
    static EdgePair createEdgePair( Vertex* origin1, Vertex* origin2,
                                    Face* face1, Face* face2 );

    HalfEdge* twin()               { return _twin; }
    HalfEdge* next()               { return _next; }
    Vertex*   origin()             { return _origin; }
    Face*     face()               { return _face; }
    HalfEdge* nextAroundVertex()   { return _twin->next(); }
    HalfEdge* prevAroundVertex();   
    HalfEdge* prev();   

    void      setTwin( HalfEdge* twin )      { _twin = twin; }
    void      setNext( HalfEdge* next)       { _next = next; }
    void      setFace( Face*     face)       { _face = face; }
    void      setOrigin( Vertex* origin)     { _origin = origin; }

    bool         checkConsistent();
    unsigned int flags()                     { return _flags; }
    void         setFlags( unsigned int f )  { _flags = f; }
private:


    HalfEdge* _twin;
    HalfEdge* _next;
    Vertex*   _origin;
    Face*     _face;
    

    unsigned int _flags;
};


#endif // _HALFEDGE_H_
