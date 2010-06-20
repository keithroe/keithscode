
#include <iostream>
#include <io/readPLY.h>
#include <io/rply.h>
#include <math/Vector3.h>



Vector3* verts;
int**    faces;
int curr_face;
int curr_vert;

static int vertex_cb(p_ply_argument argument) 
{
    static Vector3 vertex;
    long coord; 
    ply_get_argument_user_data(argument, NULL, &coord);
    double value = ply_get_argument_value(argument);
    switch ( coord ) {
        case 0:
            vertex[0] = value;
            break;
        case 1:
            vertex[1] = value;
            break;
        case 2:
            vertex[2] = value;
            break;
    }
    //printf("%g", ply_get_argument_value(argument));
    if ( coord == 2 ) 
        std::cerr << "\n";
    else 
        std::cerr << " ";
    return 1;
}

static int face_cb(p_ply_argument argument) {
    
    long value_index;
    ply_get_argument_property(argument, NULL, NULL, &value_index);

    if ( value_index == -1 ) {
        // this is the start of a new face
        int length = static_cast<int>( ply_get_argument_value(argument) );
        std::cerr << " face length: " << length << std::endl;
    }
    else {
            std::cerr << " valueindex: " << value_index << " " 
                      << ply_get_argument_value(argument) << " ";
    }
    return 1;
}

static int tristrip_cb(p_ply_argument argument) {
    long length, value_index;
    ply_get_argument_property(argument, NULL, &length, &value_index);
    std::cerr << " valueindex: " << value_index << " " 
              << ply_get_argument_value(argument) << "\n";
    return 1;
}

/*
int main(void) {
    long nvertices, ntriangles;

    if (!ply) return 1;
    if (!ply_read_header(ply)) return 1;
    nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, NULL, 0);
    ply_set_read_cb(ply, "vertex", "y", vertex_cb, NULL, 0);
    ply_set_read_cb(ply, "vertex", "z", vertex_cb, NULL, 1);
    ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, NULL, 0);
    //printf("%ld\n%ld\n", nvertices, ntriangles);
    if ( !ply_read(ply) ) 
        return 1;
    ply_close(ply);
    return 0;
}
*/

bool readPLY( const char* filename )
{
    p_ply ply = ply_open(filename, NULL);
    if (!ply) {
        return 1;
    }

    if (!ply_read_header(ply))  {
        return 1;
    }

    curr_vert = -1;    
    curr_face = -1;    
    long nverts = ply_set_read_cb(ply, "vertex", "x", vertex_cb, NULL, 0);
    ply_set_read_cb(ply, "vertex", "y", vertex_cb, NULL, 1);
    ply_set_read_cb(ply, "vertex", "z", vertex_cb, NULL, 2);

    long nfaces = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, 
            NULL, 0);
    long nstrips = ply_set_read_cb(ply, "tristrips", "vertex_indices", 
            tristrip_cb, NULL, 0);

    std::cerr << " nverts : " << nverts << "\n" 
              << " nfaces : " << nfaces << "\n"
              << " nstrips: " << nstrips << "\n";
    //printf("%ld\n%ld\n", nvertices, ntriangles);
    if ( !ply_read(ply) ) 
        return 1;
    ply_close(ply);
    return 0;
    return true;
}


