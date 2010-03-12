/**
 *
 *    Image 3D class (implementation)
 *
 *  File: image3d.cc
 *
 *  Authors:
 *    Andre Maximo (andmax@cos.ufrj.br)
 *    Ricardo Farias (rfarias@ufrj.br)
 *    Cristiana Bentes (cristianabentes@gmail.com)
 *
 *  Last Update: Jan 04, 2007
 *
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
}

#include <fstream.h>

#include "ppm.h"

/// PPM Image

/// Read a line, skipping blank lines or comment
/// lines: lines started by "#"
void PPM::getNextLine( char *buf, FILE *f ) {

	do {

		fgets( buf, _BUF_SIZE-1 , f );

	} while( *buf == '#' || *buf == '\n' );

	int i = 0;
	while( *(buf+i) != '\n' ) i++;

	assert( *(buf+i) == '\n' );

	*(buf+i) = 0;

}

/// Read a ppm file named filename
/// @return true if success in reading
bool PPM::read() {

	char buf[_BUF_SIZE];

	FILE *f = fopen( filename, "r" );

	if( f == NULL )
		return false;

	getNextLine( buf, f ); // First line must be P2 or P5

	if( *buf != 'P' ) {
		fclose( f );
		return false;
	}

	switch( *( buf + 1 ) ) {

	case ASCII_PGM+'0': ff = ffascii; break;

	case RAW_PGM+'0': ff = ffpgm; break;
			
	case RAW_PPM+'0': ff = ffppm; break;

	default:
		fclose( f );
		return false;
	}
	
	getNextLine( buf, f ); // Second line: xdim ydim

	if( sscanf( buf, "%d %d", &dimX, &dimY ) != 2 ) {
		cerr << "Unexpected number of parameters for dimension X and Y." << endl;
		return false;
	}

	getNextLine( buf, f ); // Third line: number of colors

	if( sscanf( buf, "%d", &nC ) != 1 ) {
		cerr << "Unexpected number of parameters for number of colors." << endl;
		return false;
	}

        // Allocate space for data

	if( buffer ) delete buffer;

	buffer = ( uchar * )malloc( dimX * dimY  * sizeof( uchar ) );

	if( !buffer ) {
		cerr << "Could not allocate memory for ppm data.\n";
		return false;
	}

	// Reading data

	if( ff == ffpgm || ff == ffppm ) {
		
		fread( buffer, 1, dimX*dimY, f );

	} else {

		for( uint i = 0 ; i < dimX*dimY ; i++ ) {

			getNextLine( buf, f ); // Read data

			*( buffer + i ) = (uchar) atoi( buf );
			
		}

	}

	fclose(f);

	return true;

}

/// Write a ppm file named outFile
/// @arg outFile output file name
/// @arg outFF output file format
/// @return true if success in writing
bool PPM::write( char *outFile, file_format outFF ) {

	FILE *f = fopen( outFile, "w" );

	if( f == NULL ) {
		cerr << "Could not save ppm file." << endl;
		return false;
	}


	if( outFF == ffppm ) {
			
		fprintf( f, "P6\n" );
		fprintf( f, "#File created by Ricardo Farias/Cristiana Bentes\n" );
		fprintf( f, "%u %u\n", dimX, dimY );
		fprintf( f, "255\n" );
		fwrite( buffer, 1, dimX*dimY*nB, f );

	}

	if( outFF == ffpgm ) {
			
		fprintf( f, "P5\n" );
		fprintf( f, "#File created by Ricardo Farias/Cristiana Bentes\n" );
		fprintf( f, "%u %u\n", dimX, dimY );
		fprintf( f, "255\n" );
		fwrite( buffer, 1, dimX*dimY*nB, f );

	}

	if ( outFF == ffascii ) {

		fprintf( f, "P2\n" );
		fprintf( f, "#File created by Ricardo Farias/Cristiana Bentes\n" );
		fprintf( f, "%u %u\n", dimX, dimY );
		fprintf( f, "255\n" );

		for( uint i = 0 ; i < dimX*dimY*nB ; i++ ) {
				
			fprintf( f, "%d\n", *( buffer + i )  );

		}
			
	}
		
	fclose(f);

	return true;

}

/// Flip ppm image
/// @arg fd which direction it will be flipped
void PPM::flip( flip_direction fd ) {

	uint x, y, idx1, idx2;
	uchar c0, c1;

	for( y = 0 ; y < ((fd == fdHorizontal)?dimY:dimY/2) ; y++ ) {

		for( x = 0 ; x < ((fd == fdHorizontal)?dimX/2:dimX) ; x++ ) {

			idx1 = idxPos(x, y);

			if (fd == fdHorizontal)
				idx2 = idxPos(dimX-x-1, y);
			else
				idx2 = idxPos(x, dimY-y-1);

			c0 = buffer[ idx1 ];
			c1 = buffer[ idx2 ];

			buffer[ idx1 ] = c1;
			buffer[ idx2 ] = c0;

		}

	}

}
