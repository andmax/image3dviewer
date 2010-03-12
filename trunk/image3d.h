/**
 *
 *    Image 3D class (header)
 *
 *  File: image3d.h
 *
 *  Authors:
 *    Andre Maximo (andmax@cos.ufrj.br)
 *    Ricardo Farias (rfarias@ufrj.br)
 *    Cristiana Bentes (cristianabentes@gmail.com)
 *
 *  Last Update: May 03, 2007
 *
 */

/// --------------------------------------   Definitions   -----------------------------------------

#include <iostream>

using namespace std;

#define _BUF_SIZE 500
#define ASCII_PGM 2
#define RAW_PGM 5
#define RAW_PPM 6
#define HEADER_SIZE 200
#define HEADER_VERSION 1.0

/// Type definitions

typedef unsigned int uint;
typedef unsigned char uchar;

/// Enumerations

enum file_format { ffnone, ffppm, ffpgm, ffascii, ffbin };

enum flip_direction { fdHorizontal, fdVertical };

enum slice_direction { sdXY, sdYZ, sdXZ };

///--------------------------------------   Image 3D   ---------------------------------------

class image3d {

public:

	/// Default Constructor
	image3d() : dimX(0), dimY(0), dimZ(0), nC(0), nB(1), ff(ffnone),
		filename(NULL), buffer(NULL) { }

	/// Destructor
	virtual ~image3d() {
		/// virtual destructor:
		///   image3d descendants will be responsible
		///   to delete their own allocated pointers
		/// the only two pointers are:
		///   filename -- can not be deleted, it isn't allocated
		///   buffer -- must be deleted if it is allocated
		if (buffer) delete [] buffer;
	}

	/// Virtual abstract functions
	/// Its descendants classes must implement it
	virtual bool read() = 0;
	virtual bool write() = 0;

	/// Gets informations about the image
	/// @arg i global buffer index 
	/// @arg x local buffer index (x coordinate)
	/// @arg y local buffer index (y coordinate)
	/// @return any information about the image
	inline uchar *getBuffer( void ) { return buffer; };
	inline uint getDimX( void ) { return dimX; }
	inline uint getDimY( void ) { return dimY; }
	inline uint getDimZ( void ) { return dimZ; }
	inline uint getDataSize( void ) { return dimX*dimY*dimZ; }
	inline uint getMaxColor( void ) { return nC; }
	inline uint getNumByte( void ) { return nB; }
	inline uchar getByte( int i ) { return *(buffer+i); }
	inline uchar getByte( int x, int y ) { return *(buffer+(x+y*dimX)); }
	inline char *getFileName( void ) { return filename; }
	inline file_format getFF( void ) { return ff; }

	/// Sets a specific byte in the image
	/// @arg i global buffer index 
	/// @arg x local buffer index (x coordinate)
	/// @arg y local buffer index (y coordinate)
	/// @arg c byte to be set
	inline void setByte( int i, uchar c ) { *(buffer+i) = c; }
	inline void setByte( int x, int y, uchar c ) { *(buffer+x+y*dimX) = c; }
	inline void setFileFormat( file_format newFF ) { ff = newFF; }
	inline void setFileName( char *f ) { filename = f; }
	inline void setBuffer( uchar *buf ) { buffer = buf; }
	inline void setNumByte( uint nb ) { nB = nb; }

	/// Gets index position in the buffer
	/// @arg i, j, k local buffer index (i,j,k) matrix-like position
	/// @return vector position index in the buffer
	inline uint idxPos(uint i, uint j) { return i + j*dimX; }
	inline uint idxPos(uint i, uint j, uint k) { return i + (j + k*dimY)*dimX; }

	/// Gets the maximum slice number in any slice direction
	/// @arg sd slice direction
	/// @return maximum slice number
	inline uint maxSliceNum(slice_direction sd) {
		return ((sd==sdXY)?(dimZ):((sd==sdYZ)?(dimX):(dimY)));
	}


protected:

	uint dimX, dimY, dimZ, nC, nB; ///< Dimensions X, Y, Z, number of colors and bytes
	file_format ff; ///< Image file format
	char *filename; ///< Name of the image file
	uchar *buffer; ///< Image buffer

};
