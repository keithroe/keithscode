
/*****************************************************************************\ 
 *  
 *  filename: MeshIO.h
 *  author  : r. keith morley
 *
 *  Changelog:
 *
\*****************************************************************************/


#ifndef _MESHIO_H_
#define _MESHIO_H_

#include <vector>
#include <io/rply.h>


class Vector3;
class Mesh;
class Vertex;


class  MeshIO
{
public:
    MeshIO();
    ~MeshIO();

    static Mesh* loadPLY( const char* filename );
    static Mesh* loadOFF( const char* filename );

private:
    static std::vector<Vertex*>   verts;
    static std::vector<int*>      faces;

    // callbacks for rply
    static int plyVertex(p_ply_argument argument);
    static int plyFace(p_ply_argument argument);
};


#endif // _MESHIO_H_
