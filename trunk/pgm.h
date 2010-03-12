/**
 *
 *    PGM class (header)
 *
 *  File: pgm.h
 *
 *  Authors:
 *    Andre Maximo (andmax@cos.ufrj.br)
 *    Ricardo Farias (rfarias@ufrj.br)
 *    Cristiana Bentes (cristianabentes@gmail.com)
 *
 *  Last Update: May 03, 2007
 *
 */

#include "image3d.h"

/// PGM Image

class PGM : public image3d
{

public:

	/// Constructor (empty image)
	/// @arg dx pgm dimension x
	/// @arg dy pgm dimension y
	/// @arg nc number of colors
	PGM( uint dx, uint dy, uint nc = 256 ) : image3d() {
		dimX = dx;
		dimY = dy;
		dimZ = 1; ///< only 1 slice
		nC = nc; ///< 256 colors
	}

	/// Constructor (read image from file)
	/// Set and read a pgm file named f
	/// @arg f pgm file name
	PGM( char *f ) : image3d() {
		dimZ = 1; ///< only 1 slice
		nC = 256; ///< 256 colors
		filename = f;
		read(); ///< in read the file format is setted
	}

	/// Constructor (create image from buffer)
	/// @arg dx pgm dimension x
	/// @arg dy pgm dimension y
	/// @arg nc number of colors
	PGM( uchar *b ) : image3d() {
		dimZ = 1; ///< only 1 slice
		nC = 256; ///< 256 colors
		buffer = b;
	}

	/// Read a pgm file named filename
	/// @return true if success in reading
	bool read();

	/// Write a pgm file named outFile
	/// @arg outFile output file name
	/// @arg outFF output file format
	/// @return true if success in writing
	bool write(char *o, file_format outFF = ffpgm);
	bool write() { return write ( filename, ff ); }

	/// Flip pgm image
	/// @arg fd which direction it will be flipped
	void flip( flip_direction fd );


private:

	/// Read a line, skipping blank lines or comment
	/// lines: lines started by "#"
	void getNextLine( char *buf, FILE *f );

};
