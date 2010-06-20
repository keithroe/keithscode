

#include <mesh/HalfEdge.h>
#include <mesh/Vertex.h>
#include <mesh/Face.h>
#include <cstdlib> 
#include <cassert> 


HalfEdge::HalfEdge() 
: _twin(NULL), _next(NULL), _origin(NULL), _face(NULL), _flags(0u)
{
}

HalfEdge::HalfEdge( Vertex* origin, Face* face )
: _twin(NULL), _next(NULL), _origin(origin), _face(face), _flags(0u)
{ 
}


HalfEdge::HalfEdge( HalfEdge* twin, HalfEdge* next,
                    Vertex* origin, Face* face )
: _twin(twin), _next(next), _origin(origin), _face(face) , _flags(0u)
{ 
}


HalfEdge::~HalfEdge()
{
}


HalfEdge::EdgePair HalfEdge::createEdgePair()
{
    HalfEdge* e1 = new HalfEdge();
    HalfEdge* e2 = new HalfEdge();
    
    e1->setTwin( e2 );
    e2->setTwin( e1 );

    return EdgePair( e1, e2 );
}


HalfEdge::EdgePair HalfEdge::createEdgePair( Vertex* origin1, 
        Vertex* origin2 )
{
    assert( origin1 && origin2 );

    HalfEdge* e1 = new HalfEdge();
    HalfEdge* e2 = new HalfEdge();
    
    e1->setTwin( e2 );
    e1->setOrigin( origin1 );
    e2->setTwin( e1 );
    e2->setOrigin( origin2 );

    origin1->setEdge( e1 );
    origin2->setEdge( e2 );

    return EdgePair( e1, e2 );
} 

    
HalfEdge::EdgePair HalfEdge::createEdgePair( Vertex* origin1, 
        Vertex* origin2, Face* face1, Face* face2 )
{
    assert( origin1 && origin2 );

    HalfEdge* e1 = new HalfEdge();
    HalfEdge* e2 = new HalfEdge();
    
    e1->setTwin( e2 );
    e1->setOrigin( origin1 );
    e1->setFace( face1 );
    e2->setTwin( e1 );
    e2->setOrigin( origin2 );
    e2->setFace( face2 );

    origin1->setEdge( e1 );
    origin2->setEdge( e2 );
    face1->setEdge( e1 );
    face2->setEdge( e2 );

    return EdgePair( e1, e2 );
} 


HalfEdge* HalfEdge::prevAroundVertex()
{
    HalfEdge* p = prev();

    // if no prev, this is a boundary edge
    if ( !p ) return NULL;
    
    return p->twin();
}


HalfEdge* HalfEdge::prev()
{
    HalfEdge* current = _next;

    while ( current != NULL && current->_next != this ) 
        current = current->_next;

    return current;
}


bool HalfEdge::checkConsistent() {
    if ( !_twin ) {
        std::cerr << " HalfEdge::checkConsistent() - twin pointer is NULL " 
                  << std::endl;
    }

    if ( _face && !_next) {
        std::cerr << " HalfEdge::checkConsistent() - nextedge  pointer is NULL " 
                  << std::endl;
    }

    if ( !_origin ) {
        std::cerr << " HalfEdge::checkConsistent() - origin pointer is NULL " 
                  << std::endl;
    }

    /*
    if ( !_face ) {
        std::cerr << " HalfEdge::checkConsistent() - face pointer is NULL " 
                  << std::endl;
    }
    */

    if ( !_twin || !_origin ) return false;
    //if (!_face || !_twin || !_origin || !_next ) return false;
    
    if (_twin->_twin != this )  { 
        std::cerr << " HalfEdge::checkConsistent()  -I am not my twin's twin " 
                  << std::endl;
        return false; 
    }
    
    return true;
}

