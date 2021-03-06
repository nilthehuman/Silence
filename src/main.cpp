/*
 * Copyright 2016 Dániel Arató
 *
 * This file is part of Silence.
 *
 * Silence is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Silence is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Silence.  If not, see <http://www.gnu.org/licenses/>.
 */

// The main function. Parses command line flags, reads the input scene file and runs the actual tracer
// Part of Silence, an experimental rendering engine

#include <iostream>
#include <limits>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <ctime>

#include "core/camera.h"
#include "core/renderer.h"
#include "core/scene.h"

#ifdef COMPILE_WITH_GUI
#include "gui/gui.h"
#include "parser/parsemotions.h"
#endif

#include "parser/parsescene.h"

using namespace Silence;

const std::string VERSION = "pre-alpha";

struct flags Silence::modeFlags;

struct arguments {
    char*  progname;
    int    depth;
    int    level;
    double cutoff;
    double gamma;
    char*  sceneFilename;
    char*  outFilename;
#ifdef COMPILE_WITH_GUI
    bool   gui;
    int    fps;
    char*  motionsFilename;
    bool   hud;
#endif
};

void help( std::string progname )
{
    std::cout << "usage: " << progname << " SCENE_FILENAME [OPTIONS]" << std::endl << std::endl;
    std::cout << "Command line options:" << std::endl;
    std::cout << "  -d, --depth DEPTH   Set the maximal depth (length) of any path (default 6)" << std::endl;
    std::cout << "  -l, --level LEVEL   Show only an exact level of the tree (unset by default)" << std::endl;
    std::cout << "  -c, --cutoff LIMIT  Stop following Zones with less intensity than LIMIT (unset by default)" << std::endl;
    std::cout << "  -g, --gamma EXP     Set the exponent for post-mortem gamma correction (default 1.0)" << std::endl;
    std::cout << "  -o, --out FILENAME  Set the filename for the output image (default image.ppm)" << std::endl;
#ifdef COMPILE_WITH_GUI
    std::cout << "      --gui           Start interactive graphical interface instead of outputting to file" << std::endl;
    std::cout << "  -f, --fps FPS       Set the framerate for the graphical interface (default 10)" << std::endl;
    std::cout << "  -m, --motions FILEN Set the file describing how the surfaces move" << std::endl;
    std::cout << "      --hud           Show performance info in top left corner" << std::endl;
#endif
    std::cout << "  -h, --help          Print this help message and quit" << std::endl;
    std::cout << "      --version       Print version information and quit" << std::endl;
    std::cout << "  -v, --verbose       Show progress and print troubleshooting info while running" << std::endl << std::endl;
    std::cout << "Exit status:" << std::endl;
    std::cout << "  0  if OK" << std::endl;
    std::cout << "  1  if arguments are unparseable" << std::endl;
    std::cout << "  2  if input file is unreadable" << std::endl;
    std::cout << "  3  if input file is unparseable" << std::endl;
    std::cout << "  4  if output file is unwriteable" << std::endl;
    exit(0);
}

void version()
{
    std::cout << "Silence " << VERSION << std::endl;
    std::cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>." << std::endl;
    std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
    std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
    exit(0);
}

void usage( std::string progname )
{
    std::cerr << "usage: " << progname << " SCENE_FILENAME [-v|--verbose] [--depth MAX_DEPTH_OF_PATHS]" << std::endl;
    std::cerr << "  [--level LEVEL] [--cutoff LIMIT] [--gamma GAMMA] [--out IMAGE_FILENAME]" << std::endl;
#ifdef COMPILE_WITH_GUI
    std::cerr << "  [--gui]" << std::endl;
#endif
    exit(1);
}

void die( int exitCode, std::string message )
{
    std::cerr << "error: " << message << std::endl;
    exit( exitCode );
}

// Read and process the command line arguments to the program
void parseArgs( int argc, char* argv[], struct arguments* args )
{
    args->progname = argv[0];
#ifdef COMPILE_WITH_GUI
    if ( std::string(args->progname).find("gui") != std::string::npos )
        args->gui = true;
#endif
    for ( int i = 1; i < argc; ++i )
    {
        if( !strcmp(argv[i], "-d") || !strcmp(argv[i], "--depth") )
        {
            if ( argc <= ++i )
                usage( args->progname );
            args->depth = atoi( argv[i] );
            if ( !args->depth )
                usage( args->progname );
        }
        else if( !strcmp(argv[i], "-l") || !strcmp(argv[i], "--level") )
        {
            if ( argc <= ++i )
                usage( args->progname );
            args->level = atoi( argv[i] );
            if ( args->level < 0 )
                usage( args->progname );
        }
        else if( !strcmp(argv[i], "-c") || !strcmp(argv[i], "--cutoff") )
        {
            if ( argc <= ++i )
                usage( args->progname );
            args->cutoff = atof( argv[i] );
            if ( !args->cutoff < 0 )
                usage( args->progname );
        }
        else if( !strcmp(argv[i], "-g") || !strcmp(argv[i], "--gamma") )
        {
            if ( argc <= ++i )
                usage( args->progname );
            args->gamma = atof( argv[i] );
            if ( !args->gamma )
                usage( args->progname );
        }
        else if( !strcmp(argv[i], "-o") || !strcmp(argv[i], "--out") )
        {
            if ( argc <= ++i )
                usage( args->progname );
            args->outFilename = argv[i];
        }
#ifdef COMPILE_WITH_GUI
        else if( !strcmp(argv[i], "--gui") )
        {
            args->gui = true;
        }
        else if( !strcmp(argv[i], "-f") || !strcmp(argv[i], "--fps") )
        {
            if ( argc <= ++i )
                usage( args->progname );
            args->fps = atoi( argv[i] );
            if ( !args->fps )
                usage( args->progname );
        }
        else if( !strcmp(argv[i], "-m") || !strcmp(argv[i], "--motions") )
        {
            if ( argc <= ++i )
                usage( args->progname );
            args->motionsFilename = argv[i];
        }
        else if( !strcmp(argv[i], "--hud") )
        {
            args->hud = true;
        }
#endif
        else if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
        {
            help( args->progname );
        }
        else if( !strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose") )
        {
            modeFlags.verbose = true;
        }
        else if( !strcmp(argv[i], "--version") )
        {
            version();
        }
        else if( argv[i][0] == '-' )
        {
            usage( args->progname );
        }
        else
        {
            if( args->sceneFilename )
                usage( args->progname );
            args->sceneFilename = argv[i];
        }
    }
    if( !args->sceneFilename )
        usage( args->progname );
#ifdef COMPILE_WITH_GUI
    if( args->gui )
    {
        if( strcmp( args->outFilename, "image.ppm" ) )
            std::cerr << "main: warning: starting in GUI mode, disregarding --out setting." << std::endl;
    }
    else
    {
        if( args->motionsFilename )
            std::cerr << "main: warning: starting in CLI mode, disregarding --motions setting." << std::endl;
        if( args->hud )
            std::cerr << "main: warning: starting in CLI mode, disregarding --hud setting." << std::endl;
    }
#endif
    if( args->depth - 1 < args->level )
    {
        std::cerr << "main: you requested to see --level " << args->level << " of the render but --depth is set to " << args->depth << "." << std::endl;
        std::cerr << "main: (Note that levels start at 0.)" << std::endl;
        usage( args->progname );
    }
}

Camera* camera = NULL;
void cleanup()
{
    delete camera->getScene();
    delete camera;
}

int main( int argc, char* argv[] )
{
    // Set mode flags to default values
    modeFlags.verbose = false;

    // Parse command line arguments
    struct arguments args;
    args.depth           =  6;
    args.level           = -1;
    args.cutoff          =  0;
    args.gamma           =  1;
    args.sceneFilename   = NULL;
    args.outFilename     = (char*)"image.ppm";
#ifdef COMPILE_WITH_GUI
    args.gui             = false;
    args.fps             = 10;
    args.motionsFilename = NULL;
    args.hud             = false;
#endif
    parseArgs( argc, argv, &args );
    if( modeFlags.verbose )
    {
        std::cerr << "main: arguments: ";
        std::cerr << "depth = " << args.depth << ", level = " << args.level << ", cutoff = " << args.cutoff << ", gamma = " << args.gamma;
#ifdef COMPILE_WITH_GUI
        if( !args.gui )
#endif
            std::cerr << ", outFilename = " << args.outFilename;
        std::cerr << std::endl;
    }

    // Process scene description input
    if( modeFlags.verbose )
    {
        if( !strcmp(args.sceneFilename, "-") )
            std::cerr << "main: reading scene description from standard input..." << std::endl;
        else
            std::cerr << "main: reading scene file '" << args.sceneFilename << "'..." << std::endl;
    }
    std::istream* is;
    std::ifstream ifs;
    if( !strcmp(args.sceneFilename, "-") )
    {
        is = &std::cin;
    }
    else
    {
        ifs.open( args.sceneFilename );
        if( !ifs.is_open() )
            die( 2, "cannot read file at '" + std::string(args.sceneFilename) + "'" );
        is = &ifs;
    }
    try {
        camera = parseScene( *is );
    }
    catch( const std::string& e ) {
        die( 3, e );
    }
    if( ifs.is_open() )
        ifs.close();
    if( modeFlags.verbose )
        std::cerr << "main: input scene file read successfully." << std::endl;

#ifdef COMPILE_WITH_GUI
    std::vector< Motion* > motions;
    if( args.motionsFilename )
    {
        // Process motions description input
        if( modeFlags.verbose )
        {
            if( !strcmp(args.motionsFilename, "-") )
                std::cerr << "main: reading motions description from standard input..." << std::endl;
            else
                std::cerr << "main: reading motions file '" << args.motionsFilename << "'..." << std::endl;
        }
        if( !strcmp(args.motionsFilename, "-") )
        {
            is = &std::cin;
        }
        else
        {
            ifs.open( args.motionsFilename );
            if( !ifs.is_open() )
                die( 2, "cannot read file at '" + std::string(args.motionsFilename) + "'" );
            is = &ifs;
        }
        try {
            motions = parseMotions( *is, camera->getScene() );
        }
        catch( const std::string& e ) {
            die( 3, e );
        }
        if( ifs.is_open() )
            ifs.close();
        if( modeFlags.verbose )
            std::cerr << "main: input motions file read successfully." << std::endl;
    }

    if ( args.gui )
    {
        if ( modeFlags.verbose )
            std::cerr << "main: creating GUI." << std::endl;
        GUI gui( camera );
        gui.initialize( &argc, argv );
        gui.setup( args.depth, args.level, args.cutoff, args.gamma, (int)(1000.0 / args.fps), args.hud, motions );
        atexit( &cleanup );
        if ( modeFlags.verbose )
            std::cerr << "main: starting the renderer." << std::endl;
        gui.run();
    }
#endif

    if( strcmp(args.outFilename, "-") )
    {
        // Check if target file is writable before starting calculations
        if( modeFlags.verbose )
            std::cerr << "main: checking if output file is writable... ";
        std::ofstream ofs;
        ofs.open( args.outFilename );
        if( !ofs.is_open() )
            die( 4, "cannot write file at '" + std::string(args.outFilename) + "'" );
        ofs.close();
        if( modeFlags.verbose )
            std::cerr << "OK." << std::endl;
    }

    // Render image
    if ( modeFlags.verbose )
        std::cerr << "main: starting the renderer." << std::endl;
    const time_t start = std::time( NULL );
    std::srand( start );
    Renderer renderer( camera->getScene() );
    renderer.addCamera( camera );
    renderer.render( 0, args.depth, args.level, args.cutoff, args.gamma );
    if ( modeFlags.verbose )
    {
        const time_t end = std::time( NULL );
        const int m =      difftime(end, start) / 60;
        const int s = (int)difftime(end, start) % 60;
        std::cerr << "main: rendering took ";
        if ( m ) std::cerr << m << " minute(s) and " ;
        std::cerr << s << " second(s)." << std::endl;
    }

    if( strcmp(args.outFilename, "-") )
    {
        // Dump results into file
        std::ofstream ofs;
        ofs.open( args.outFilename );
        while( !ofs.is_open() )
        {
            std::cerr << "cannot write file at '" << args.outFilename << "'; please make the file writable and press Return." << std::endl;
            std::cin.ignore();
            ofs.open( args.outFilename );
        }
        camera->writePixels( ofs );
        ofs.close();
        if ( modeFlags.verbose )
            std::cerr << "main: image written to '" << args.outFilename << "'" << std::endl;
    }
    else
    {
        // Dump results to standard output
        camera->writePixels( std::cout );
        if ( modeFlags.verbose )
            std::cerr << "main: image written to standard output" << std::endl;
    }

    cleanup();
    return 0;
}

