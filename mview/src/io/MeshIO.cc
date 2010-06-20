

#include <fstream>

#include <io/MeshIO.h>
#include <io/MeshIO.h>
#include <math/Vector3.h>
#include <mesh/Vertex.h>
#include <mesh/Mesh.h>

std::vector<Vertex*>    MeshIO::verts;
std::vector<int*>       MeshIO::faces;


MeshIO::MeshIO()
{
}


MeshIO::~MeshIO()
{
}


int MeshIO::plyVertex( p_ply_argument argument )
{
    static int vert_id = 0;
    long coord;
    ply_get_argument_user_data(argument, NULL, &coord);
    double value = ply_get_argument_value(argument);
    switch ( coord ) {
        case 0:
            verts.push_back( new Vertex() );
            verts.back()->coord().setX( value );
            verts.back()->setID( vert_id++ );
            break;
        case 1:
            verts.back()->coord().setY( value );
            break;
        case 2:
            verts.back()->coord().setZ( value );
            break;
    }
    return 1;
}


int MeshIO::plyFace( p_ply_argument argument )
{
    long value_index;
    ply_get_argument_property(argument, NULL, NULL, &value_index);
    long value = static_cast<long>( ply_get_argument_value(argument) );

    if ( value_index == -1 ) {
        // this is the start of a new face
        faces.push_back( new int[value] );
    }
    faces.back()[value_index+1] = value;

    return 1;
}


Mesh* MeshIO::loadPLY( const char* filename )
{
    p_ply ply = ply_open(filename, NULL);

    if (!ply)                   return NULL;
    if (!ply_read_header(ply))  return NULL;

    long nverts = ply_set_read_cb(ply, "vertex", "x", plyVertex, NULL, 0);
    ply_set_read_cb(ply, "vertex", "y", plyVertex, NULL, 1);
    ply_set_read_cb(ply, "vertex", "z", plyVertex, NULL, 2);

    long nfaces = ply_set_read_cb(ply, "face", "vertex_indices", plyFace,
            NULL, 0);

    verts.clear();
    faces.clear();
    verts.reserve( nverts );
    faces.reserve( nfaces);

    if ( !ply_read(ply) )       return NULL;
    ply_close(ply);

    Mesh* mesh = Mesh::create( verts, faces );
    faces.erase( faces.begin(), faces.end() ); 
    return mesh;
}


Mesh* MeshIO::loadOFF( const char* filename )
{
    int num_faces = 0;
    int num_verts = 0;
    int num_edges = 0;
    
    std::ifstream in( filename );
    std::string format;
    in >> format;

    if ( format != "OFF" ) {
        std::cerr << " Mesh::loadOFF() - WARNING: unsupported format tag\'"
                  << format << "\' ... bailing." << std::endl;
        return NULL;
    }

    in >> num_verts >> num_faces >> num_edges;
    if ( num_verts < 3 ) {
        std::cerr << " Mesh::loadOFF() - WARNING: invalid num verts:  "
                  << num_verts << std::endl;
        return NULL;
    }
    if ( num_faces < 1 ) {
        std::cerr << " Mesh::loadOFF() - WARNING: invalid num or faces: "
                  << num_faces << std::endl;
        return NULL;
    }

    verts.reserve( num_verts );
    Vector3 tempvert;
    for (int i = 0; i < num_verts; i++) { 
        in >> tempvert[0] >> tempvert[1] >> tempvert[2];
        verts.push_back( new Vertex(tempvert) );
        verts.back()->setID( i );
        
    }

    faces.reserve( num_faces );
    for (int i = 0; i < num_faces; i++) {
        int num_face_verts;
        in >> num_face_verts;
        if ( num_face_verts < 3 ) {
            std::cerr << " Mesh::loadOFF() - WARNING: face has < 3 verts ..."
                      << " ... skipping." << std::endl;
        } 
        
        // allocate an extra element per face for storing number of vertices
        faces.push_back( new int [num_face_verts+1] );
        faces.back()[0] = num_face_verts;
        
        for (int j = 0; j < num_face_verts; j++)
            in >> faces.back()[j+1];
    }

    Mesh* mesh = Mesh::create( verts, faces );

    faces.erase( faces.begin(), faces.end() ); 

    return mesh;
}

