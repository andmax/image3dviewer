/**
 *
 *    Bin class (header)
 *
 *  File: bin.h
 *
 *  Authors:
 *    Andre Maximo (andmax@cos.ufrj.br)
 *    Ricardo Farias (rfarias@ufrj.br)
 *    Cristiana Bentes (cristianabentes@gmail.com)
 *
 *  Last Update: May 03, 2007
 *
 */

#include "pgm.h"

/// Bin Image

class Bin : public image3d
{

public:

	/// Constructor
	/// Set and read a bin file named f
	/// @arg f bin file name
	Bin( char *f ) : image3d() {
		ff = ffbin;
		filename = f;
		read();
	}

	/// Gets the corresponded indexed value on the buffer
	/// @arg idx index of the value inside the buffer
	/// @return agregated value by the number of Bytes
	uint getValue( uint idx );

	/// Gets one byte value for the indicated buffer position
	/// @arg idx index of the value inside the buffer
	/// @arg i, j, k local buffer index (i,j,k) matrix-like position
	/// @return one byte agregated value
	uchar getByteValue( uint idx );
	uchar getByteValue( uint i, uint j, uint k ) {
		return getByteValue( idxPos(i, j, k) );
	}

	/// Gets a pgm slice from the binary file
	/// @arg sliceNum number of the slice to be returned
	/// @arg sd slice direction to be taken
	/// @arg outFF output file format
	/// @return pgm pointer to the pgm image of that slice
	PGM *getSlice( uint sliceNum, slice_direction sd = sdXY, file_format outFF = ffpgm );

        /// Gets the size of a slice in some direction
	/// @arg sd one slide direction
        /// @return size of one slice
	uint getSliceSize( slice_direction sd );

	/// Gets the number of bytes of each datum
	/// @return number of bytes
	inline uint getNumBytes( void ) { return numBytes; }

	/// Gets endian format
	/// @return 1=little ; 2=big endian
	inline uint getEndian( void ) { return endian; }

	/// Gets the Data Size (Bin can have more than 1 byte)
	/// @return size of the entire binary
	inline uint getDataSize( void ) { return numBytes*dimX*dimY*dimZ; }

	/// Gets minimum value
	/// @return minimum value of the entire binary
	inline uint getMinValue( void ) { return minValue; }

	/// Gets maximum value
	/// @return maximum value of the entire binary
	inline uint getMaxValue( void ) { return maxValue; }

	/// Read a bin file (with header) named filename
	/// @return true if success in reading
	bool read();

	/// Write a bin file (with header) named outFile
	/// @arg outFile output file name
	/// @return true if success in writing
	bool write(char *outFile);
	bool write() { return write ( filename ); }

	/// Gets index position in the buffer (Bin can have more than 1 byte)
	/// @arg i, j, k local buffer index (i,j,k) matrix-like position
	/// @return vector position index in the buffer
	inline uint idxPos(uint i, uint j) { return (i + j*dimX)*numBytes; }
	inline uint idxPos(uint i, uint j, uint k) { return (i + (j + k*dimY)*dimX)*numBytes; }


private:

	float version; ///< Binary file version
	uint numBytes; ///< Number of Bytes for each element
	uint endian; ///< 1=Little ; 2=Big Endian format
	uint minValue, maxValue; ///< Minimum and Maximum value in buffer
};
