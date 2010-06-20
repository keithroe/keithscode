
/*****************************************************************************\ 
 *  
 *  filename: Mesh.h
 *  author  : r. keith morley
 *
 *  Changelog:
 *
\*****************************************************************************/


#ifndef _MESH_H_
#define _MESH_H_

#include <mesh/Face.h>
#include <list>
#include <vector>

class Vertex;
class Vector3;
class Face;
class HalfEdge;
class string;
class BBox;

class  Mesh
{
public:

    typedef std::list<Face*>       FaceList;
    typedef std::list<Vertex*>     VertexList;
    typedef std::list<HalfEdge*>   HalfEdgeList;
    typedef HalfEdgeList::iterator EdgeIterator;
    typedef VertexList::iterator   VertexIterator;
    typedef FaceList::iterator     FaceIterator;
    

    enum NormalStyle { FACE, VERTEX, NONE };
    Mesh();
    ~Mesh();

    static Mesh* loadMesh(const std::string& filename);
    static Mesh* loadPLY(const std::string& filename);
    static Mesh* loadOFF(const std::string& filename);

    static Mesh* create( std::vector<Vertex*>& verts, 
                         std::vector<int*>& faces );

    bool  checkConsistent();
   
    void addFace( Face* face )  
    { assert( face ); _faces.push_back( face ); }

    void addVertex ( Vertex* vert)  
    { assert( vert); _verts.push_back( vert); }
    
    void addEdge( HalfEdge* edge)  
    { assert( edge ); _edges.push_back( edge); }

    BBox getBBox();
    
    bool changed()                         { return _changed; }

    void draw( NormalStyle style );
    void debugDraw();

    void drawFaces( bool v )            { _draw_faces = v; }
    void drawEdges( bool v )            { _draw_edges = v; }
    void drawVerts( bool v )            { _draw_verts = v; }
    

    void drawEdgeHandles( bool v )      { _draw_edge_handles = v; }
    void drawFaceHandles( bool v )      { _draw_face_handles = v; }
    void drawVertHandles( bool v )      { _draw_vert_handles = v; }

    void resetDrawStyle();

    HalfEdgeList::iterator edgeBegin()     { return _edges.begin(); }
    HalfEdgeList::iterator edgeEnd()       { return _edges.end(); }
    FaceList::iterator faceBegin()         { return _faces.begin(); }
    FaceList::iterator faceEnd()           { return _faces.end(); }
    VertexList::iterator vertexBegin()     { return _verts.begin(); }
    VertexList::iterator vertexEnd()       { return _verts.end(); }

    void computeFaceNormals();
    void computeVertexNormals();
private:
 
    void drawQuads( NormalStyle style );
    void drawTris( NormalStyle style );


    FaceList         _faces;
    VertexList       _verts;
    HalfEdgeList     _edges;

    Face             _infinite_face; 

    bool             _changed;          // flag for viewer client
    bool             _debug;       

    bool             _draw_faces;
    bool             _draw_verts;
    bool             _draw_edges;
    bool             _draw_edge_handles;
    bool             _draw_face_handles;
    bool             _draw_vert_handles;

    bool             _quad_mesh;
    bool             _tri_mesh;
};


#endif // _MESH_H_
