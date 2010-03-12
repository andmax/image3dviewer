/**
 *
 *    Bin class (implementation)
 *
 *  File: bin.cc
 *
 *  Authors:
 *    Andre Maximo (andmax@cos.ufrj.br)
 *    Ricardo Farias (rfarias@ufrj.br)
 *    Cristiana Bentes (cristianabentes@gmail.com)
 *
 *  Last Update: May 03, 2007
 *
 */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
}

#include <fstream>

#include "bin.h"

/// Bin Image

/// Gets the corresponded indexed value on the buffer
/// @arg idx index of the value inside the buffer
/// @return agregated value by the number of Bytes
uint Bin::getValue( uint idx ) {

	uint highB, lowB, value;

	if ( numBytes == 1 )

		value = (uint)buffer[ idx ];

	else {	// considering only 2 Bytes

		highB = (uint)buffer[ idx+0 ];
		lowB  = (uint)buffer[ idx+1 ];

		if (endian == 1) // little endian
			value = (highB + 256*lowB);

		if (endian == 2) // big endian
			value = (lowB + 256*highB);
	}

	return value;

}

/// Gets one byte value for the indicated buffer position
/// @arg idx index of the value inside the buffer
/// @return one byte agregated value
uchar Bin::getByteValue( uint idx ) {

	uint value;
	double ratio;

	value = getValue( idx );

	ratio = (value - minValue) / (double)(maxValue - minValue);

	return (uchar)floor( 255 * ratio );

}

/// Gets a pgm slice from the binary file
/// @arg sliceNum number of the slice to be returned
/// @arg sd slice direction to be taken
/// @arg outFF output file format
/// @return pgm pointer to the pgm image of that slice
PGM *Bin::getSlice( uint sliceNum, slice_direction sd, file_format outFF ) {

	//-- this function isn't correct:
	// 1* The PGM image returned must have the data
	// buffer even for more than 1 number of bytes;
	// 2* The returned image dimensions must be
	// always larger in x than y.

	uint idx, nC = 255; // fixed number of colors is 255
	uchar *tmpBuf;
	PGM* pgm;

	switch( sd ) {

	case sdXY:

		pgm = new PGM( dimX, dimY, nC );
		tmpBuf = new uchar[dimX*dimY];

		for( uint i = 0 ; i < dimX ; i++ ) {

			for( uint j = 0 ; j < dimY ; j++ ) {

				idx = idxPos( i, j, sliceNum );

				tmpBuf[ pgm->idxPos(i, j) ] = getByteValue( idx );

			}

		}

		break;

	case sdYZ:

		pgm = new PGM( dimY, dimZ, nC );
		tmpBuf = new uchar[dimY*dimZ];

		for( uint i = 0 ; i < dimY ; i++ ) {

			for( uint j = 0 ; j < dimZ ; j++ ) {

				idx = idxPos( sliceNum, i, j );

				tmpBuf[ pgm->idxPos(i, j) ] = getByteValue( idx );

			}

		}

		break;

	case sdXZ:

		pgm = new PGM( dimX, dimZ, nC );
		tmpBuf = new uchar[dimX*dimZ];

		for( uint i = 0 ; i < dimX ; i++ ) {

			for( uint j = 0 ; j < dimZ ; j++ ) {

				idx = idxPos( i, sliceNum, j );

				tmpBuf[ pgm->idxPos(i, j) ] = getByteValue( idx );

			}

		}

		break;

	default:
		return NULL;

	}

	pgm->setFileName( filename );
	pgm->setFileFormat( ffpgm ); // fixed file format is pgm
	pgm->setBuffer( tmpBuf );

	return pgm;

}

/// Gets the size of a slice in some direction
/// @arg sd one slide direction
/// @return size of one slice
uint Bin::getSliceSize( slice_direction sd ) {

	switch( sd ) {

	case sdXY:

		return dimX * dimY * numBytes;

	case sdYZ:

		return dimY * dimZ * numBytes;

	case sdXZ:

		return dimX * dimZ * numBytes;

	}

	return 0;

}

/// Read a bin file (with header) named filename
/// @return true if success in reading
bool Bin::read() {

	version = (float)HEADER_VERSION;

	ifstream inF( filename );

	if (inF.fail()) return false;

	// Prepare and read header buffer
	char header_buffer[ HEADER_SIZE ];
	for( uint i = 0 ; i < HEADER_SIZE ; i++ )
		header_buffer[ i ] = inF.get();

	int ni = sscanf( header_buffer, "%f %d %d %d %d %d",
			 &version, &dimX, &dimY, &dimZ, &numBytes, &endian );

	if (ni == 5) // number of items successfully read
		endian = 1; // for backward compatibility

	buffer = (uchar *)malloc( getDataSize() * sizeof(uchar) );

	uint count = 0, value;
	maxValue = 0;
	minValue = (uint)(pow(256, numBytes) - 1);

	while( count < getDataSize() && !inF.eof() ) {

		buffer[ count ] = (uchar)inF.get();

		if ( numBytes == 1 )

			value = getValue( count );

		else { // considering only 2 Bytes

			if ( (count % 2) == 1 ) // 2 Bytes readed
				value = getValue( count - 1 );

		}

		if ( value < minValue )
			minValue = value;

		if ( value > maxValue )
			maxValue = value;

		count++;

	}

	inF.close();

	return true;

}

/// Write a bin file (with header) named outFile
/// @arg outFile output file name
/// @return true if success in writing
bool Bin::write( char *outFile ) {

	// Prepare header buffer
	char header_buffer[ HEADER_SIZE ];
	for( uint i = 0 ; i < HEADER_SIZE ; i++ ) 
		header_buffer[i] = ' ';

	sprintf( header_buffer, "%5.2f %5d %5d %5d %5d %5d",
		 version, dimX, dimY, dimZ, numBytes, endian );

	ofstream outF( outFile );

	if (outF.fail()) return false;

	// Save the header
	for( uint i = 0 ; i < HEADER_SIZE ; i++ )
		outF << header_buffer[i];

	// Save the body
	for( uint i = 0 ; i < getDataSize() ; i++ )
		outF << buffer[i];

	outF.close();

	return true;

}
