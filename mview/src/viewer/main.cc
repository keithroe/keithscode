
/*****************************************************************************\
 *                                                                           *
 *  infile : main.cc                                                       *
 *  author   : R. Keith Morley                                               *
 *  last mod : 01/19/04                                                      *
 *                                                                           *
\*****************************************************************************/

#include <iostream>
#include <string>
#include <mesh/Mesh.h>
#include <viewer/Viewer.h>


void printUsage();

int main (int argc, char** argv)
{
    std::string infile;
    std::string outfile;
    //bool have_outfile= false;
  
    // parse command line
    if ( argc == 2 ) {
        infile = argv[1]; 
    } 
    else if ( argc == 3 ) {
        infile  = argv[1]; 
        outfile = argv[2];
    }
    else {
        printUsage();
        exit(0);
    }
    

    Mesh* mesh = Mesh::loadMesh( infile );
    if ( !mesh ) {
        std::cerr << " ERROR - failed to load mesh \'" << infile << "\'"
                  << std::endl;
        exit(0);
    }

    Viewer::instance().addMesh( mesh );
    Viewer::instance().view( argc, argv );

    return 0;
} 

void printUsage()
{
    std::cerr << "\nUsage - ./mview  infile\n\n";
}
