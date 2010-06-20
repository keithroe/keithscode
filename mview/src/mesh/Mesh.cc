

#include <mesh/Vertex.h> 
#include <mesh/Mesh.h>
#include <mesh/HalfEdge.h> 
#include <mesh/BBox.h> 
#include <math/Vector3.h> 
#include <io/MeshIO.h> 
#include <GLUT/glut.h> 
#include <string> 
#include <iostream> 
#include <map> 
#include <vector> 


Mesh::Mesh()
{
    _changed = true;
    _debug   = false;
    resetDrawStyle();
}


Mesh::~Mesh()
{
    _draw_faces        = false;
    _draw_edges        = true;
    _draw_verts        = false;
    _draw_edge_handles = false;
    _draw_face_handles = false;
    _draw_vert_handles = false;

    _tri_mesh          = false;
    _tri_mesh          = false;
}


void Mesh::resetDrawStyle()
{
}


Mesh* Mesh::loadMesh( const std::string& filename )
{
    std::string extension = filename.substr( filename.find_last_of(".") );
    
    if ( extension ==  ".ply")
        return MeshIO::loadPLY( filename.c_str() );
    else if ( extension ==  ".off")
        return MeshIO::loadOFF( filename.c_str() );
    else {
        std::cerr << " Mesh::loadMesh() - WARNING: unknown filename extension \'"
                  << extension << "\' ... bailing." << std::endl;
        return NULL;
    }
}


Mesh* Mesh::create( std::vector<Vertex*>& verts, std::vector<int*>& faces )
{
    typedef std::map<Vertex*, HalfEdge*>                 BorderMap;
    typedef std::map< std::pair< int, int >, HalfEdge* > TwinMap;
    
    // create a map of edges to faces so we can find twins later
    TwinMap pair_to_edge;
    std::vector< HalfEdge* > face_edges;
 
    std::vector<int> used_verts( verts.size(), false );

    Mesh* mesh = new Mesh(); 

    for ( std::vector<int*>::iterator it = faces.begin(); 
            it != faces.end(); ++it ) {
        int* flist = *it; 
        int num_face_verts = flist[0];

        face_edges.clear();
        Face* face = new Face();
        mesh->addFace( face );

            

        for ( int j = 0; j < num_face_verts; j++ ) {
            int curr_vert = flist[j+1];
            int next_vert = flist[ ( (j+1) % num_face_verts ) + 1 ];
            HalfEdge* edge = NULL; 

            // See if this edge has already been created 
            TwinMap::iterator twin; 
            twin = pair_to_edge.find( std::make_pair(curr_vert, next_vert) );
            if ( twin != pair_to_edge.end() ) {
                edge = twin->second;

            } else {
                // we need to create an edge pair and add it to the twin map
                
                if ( ! used_verts[ curr_vert ] )
                    mesh->addVertex( verts[curr_vert] );
                if ( ! used_verts[ next_vert ] )
                    mesh->addVertex( verts[next_vert] );
                used_verts[ curr_vert ] = true;
                used_verts[ next_vert ] = true;
                
                HalfEdge::EdgePair edge_pair = 
                    HalfEdge::createEdgePair(verts[curr_vert], verts[next_vert]);
                pair_to_edge[ std::make_pair(next_vert, curr_vert) ] =
                    edge_pair.second;

                mesh->addEdge(edge_pair.first);
                mesh->addEdge(edge_pair.second);
                
                edge = edge_pair.first;
            }
           
            edge->setFace( face ); 
            if (j == 0) face->setEdge( edge );

            face_edges.push_back( edge );
        }

        // connect ring of edges around this face
        for ( int j = 0; j < num_face_verts; j++ ) {
            face_edges[j]->setNext( face_edges[ (j+1) % num_face_verts ] );
        }
    }

    for ( Mesh::VertexIterator itr = mesh->vertexBegin(); 
            itr != mesh->vertexEnd(); ++itr) {
        Vertex* vert = *itr; 
        if ( !vert ) {
            std::cerr << " have a null vert " << std::endl;
            continue;
        }
        if ( vert->edge() == NULL ) 
            std::cerr << " vert has null edge " << std::endl;
    }
    
    // Make sure each vertex points to the first edge if on border
    for ( Mesh::VertexIterator itr = mesh->vertexBegin(); 
            itr != mesh->vertexEnd(); ++itr) {

        Vertex*   vert = *itr; 
        HalfEdge* edge = vert->findFirstEdge();
        edge->setFlags( 1u );
        vert->setEdge( edge );
    }

    // delete any unused verts
    for ( unsigned int i = 0; i < used_verts.size(); ++i )
        if ( !used_verts[i] ) delete verts[i];

  /*  
    BorderMap border_map;
    for ( TwinMap::iterator itr = pair_to_edge.begin(); 
          itr != pair_to_edge.end(); ++itr) {
        
        HalfEdge* edge = itr->second; 
    
        // we have a border edge
        if ( !edge->face() ) {
            
    //        edge->setFace( &( mesh->_infinite_face ) );
            
            // create a map of border vertices to their previous edge
            Vertex* next_vert = verts[ itr->first.second ];
            if ( border_map.find( next_vert) != border_map.end() ) {
                std::cerr << " WARNING - Mesh::create: more than one border edge"
                          << " found for a single vertex " << std::endl; 
            }
            next_vert->setEdge( edge->twin() );
            border_map[ next_vert ] = edge;
        }
    }
*/
    /*
    // now connect up border edges
    for ( BorderMap::iterator itr = border_map.begin(); 
          itr != border_map.end(); ++itr ) {
        HalfEdge* edge = itr->second; 
        Vertex*   vert = edge->origin();
        if ( border_map.find( vert ) == border_map.end() ) {
            std::cerr << " WARNING - Mesh::create: Border edge origin not in"
                      << " border map! " << std::endl;
        }
        HalfEdge* prev = border_map[ vert ];
        prev->setNext( edge );
    }
    */

    if ( !mesh->checkConsistent() ) {
        std::cerr << " Mesh failed consistency check ... exitting" << std::endl;
        exit(0);
    }
 
    return mesh;
}


bool Mesh::checkConsistent()
{

    bool consistent = true;
    
    std::cerr << " Checking edges ( " << _edges.size() << " ) " << std::endl;
    for ( Mesh::EdgeIterator itr = edgeBegin(); itr != edgeEnd(); ++itr) {
        consistent = consistent && (*itr)->checkConsistent();
    }
    
    std::cerr << " Checking faces( " << _faces.size() << " ) " << std::endl;
    for ( Mesh::FaceIterator itr = faceBegin(); itr != faceEnd(); ++itr) {
        consistent = consistent && (*itr)->checkConsistent();
    }
    
    std::cerr << " Checking verts( " << _verts.size() << " ) " << std::endl;
    for ( Mesh::VertexIterator itr = vertexBegin(); itr != vertexEnd(); ++itr) {
        consistent = consistent && (*itr)->checkConsistent();
    }
    
        
    return consistent;
}


BBox Mesh::getBBox() 
{
    Vector3 min, max;
    min = (*vertexBegin())->coord(); 
    max = (*vertexBegin())->coord(); 
    Vector3 cur;
    for ( VertexIterator itr = vertexBegin(); itr != vertexEnd(); itr++ ) {
        cur = (*itr)->coord();

        min[0] = min[0] < cur[0] ? min [0] : cur[0]; 
        min[1] = min[1] < cur[1] ? min [1] : cur[1]; 
        min[2] = min[2] < cur[2] ? min [2] : cur[2]; 
        
        max[0] = max[0] > cur[0] ? max [0] : cur[0]; 
        max[1] = max[1] > cur[1] ? max [1] : cur[1]; 
        max[2] = max[2] > cur[2] ? max [2] : cur[2]; 
    }
    return BBox(min, max);
}


void Mesh::computeFaceNormals()
{
    // TODO: Cache normal results 
    for ( FaceIterator it = _faces.begin(); it != _faces.end(); it++ ) {
        (**it).computeNormal();
    }
}


void Mesh::computeVertexNormals()
{
    // TODO: Cache normal results 
    for ( VertexIterator it = _verts.begin(); it != _verts.end(); it++ ) {
        (**it).computeNormal();
    }
}


void Mesh::draw( NormalStyle normal_style )
{

    /*
    debugDraw();
    return;
    */

    if ( _tri_mesh ) {
        drawTris( normal_style );
        return;
    } else if ( _quad_mesh ) {
        drawQuads( normal_style );
        return;
    } 

    // We have general polygons 
    if ( normal_style == FACE ) {
      
        computeFaceNormals();
        Vector3 coord;
     
        for ( FaceIterator it = _faces.begin(); it != _faces.end(); ++it ) {

            glBegin( GL_POLYGON );
            Face*     face    = *it;
            Vector3   norm    = face->normal();
            HalfEdge* first   = face->edge();
            HalfEdge* current = first;
            glNormal3d( norm[0], norm[1], norm[2] ); 

            do {
                coord = current->origin()->coord(); 
                glVertex3d(coord[0], coord[1], coord[2]);
                current = current->next();
            } while (current != first );

            glEnd();
        }
    } else if ( normal_style == VERTEX ) {
       
        computeFaceNormals();
        computeVertexNormals();
        Vector3 coord, norm;
        
        for ( FaceIterator it = _faces.begin(); it != _faces.end(); ++it ) {

            glBegin( GL_POLYGON );
            Face*     face    = *it;
            HalfEdge* first   = face->edge();
            HalfEdge* current = first;
            glNormal3d( norm[0], norm[1], norm[2] ); 

            do {
                coord = current->origin()->coord(); 
                norm  = current->origin()->normal(); 
                glNormal3d(norm[0], norm[1], norm[2]);
                glVertex3d(coord[0], coord[1], coord[2]);
                current = current->next();
            } while (current != first );

            glEnd();
        }
    } else { // normal_style = NONE 

        Vector3 coord;
        for ( FaceIterator it = _faces.begin(); it != _faces.end(); ++it ) {

            glBegin( GL_POLYGON );
            Face*     face    = *it;
            HalfEdge* first   = face->edge();
            HalfEdge* current = first;
            do {
                coord = current->origin()->coord(); 
                glVertex3d(coord[0], coord[1], coord[2]);
                current = current->next();
            } while (current != first );

            glEnd();
        }
    }
}


void Mesh::drawTris( NormalStyle normal_style ) 
{
}


void Mesh::drawQuads( NormalStyle normal_style ) 
{
}


void Mesh::debugDraw( ) 
{
    Vector3 coord;
    for ( EdgeIterator it = _edges.begin(); it != _edges.end(); ++it ) 
    {
        HalfEdge* edge = *it;
        if ( edge->twin() < edge ) continue;

        Vertex* v1 = edge->origin();
        Vertex* v2 = edge->twin()->origin();
        unsigned int flags = edge->flags() | edge->twin()->flags();

        if ( flags & 1 ) 
            glColor3f( 1.0f, 0.1f, 0.1f );
        else if ( flags & 1<<1 ) 
            glColor3f( 0.1f, 1.0f, 0.1f );
        else 
            glColor3f( 1.0f, 1.0f, 1.0f );
        glBegin( GL_LINES );
        coord = v1->coord();
        glVertex3d(coord[0], coord[1], coord[2]);
        coord = v2->coord();
        glVertex3d(coord[0], coord[1], coord[2]);
        glEnd();
    }
    

}


