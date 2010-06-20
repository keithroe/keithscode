

#include <mesh/Vertex.h>
#include <mesh/Face.h>


bool Vertex::checkConsistent() 
{
    if ( ! _edge ) {
        std::cerr << " Vertex::checkConsistent - edge pointer is NULL " 
                  << std::endl;
        return false;
    }

    if ( _edge->origin() != this ) {
        std::cerr << " Vertex::checkConsistent - I am not my edge's origin " 
                  << std::endl;
        return false;
    }
    return true;
}

void Vertex::computeNormal() 
{
    HalfEdge* e = _edge;
    _norm = Vector3(0.0f, 0.0f, 0.0f);

    do {
        if ( e->face() ) 
            _norm += e->face()->normal();
        e = e->nextAroundVertex(); 
    } while ( e != NULL && e != _edge );

    _norm.normalize();
}

HalfEdge* Vertex::findFirstEdge()
{
    HalfEdge* curr = _edge;
    HalfEdge* next = curr->prevAroundVertex(); 
  
    assert( curr ); 
    while ( next != NULL && next != _edge ) {
        curr = next;
        next = curr->prevAroundVertex(); 
    }

    assert( curr->origin() == this ); 
    return curr;
}
    

