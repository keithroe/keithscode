

#include <mesh/Face.h>
#include <mesh/Vertex.h>
#include <iostream>

Face::Face() : _edge(NULL)
{
}


Face::~Face()
{
}


void Face::computeNormal() {

    Vertex* v1 = edge()->origin();
    Vertex* v2 = edge()->next()->origin();
    Vertex* v3 = edge()->next()->next()->origin();

    _norm = cross( v3->coord() - v2->coord(), v1->coord() - v2->coord() ); 
    _norm.normalize();
}


bool Face::checkConsistent()
{

    if ( !_edge) {
        std::cerr << "Face::checkConsistent - _edge is NULL" 
                  << std::endl;
        return false;
    }

    if ( _edge->face() != this ) {
        std::cerr << "Face::checkConsistent - I am not my edges face.  this: " 
                  << this << " edge->face: " << _edge->face() << std::endl;
        return false;
    }
    return true;
}
