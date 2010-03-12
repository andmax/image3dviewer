/**
 *  Image 3D Viewer application program
 *
 *  File: main.cc
 *
 *  Author: Andre Maximo - Date: Jan 23, 2006
 *
 *  Revision: 4.0
 *
 */

/**
 *  TO-DO LIST:
 *

(o) WITHOUT MAXIMIZING: glDrawPixels -> TEXTURE
(o) OUT OF PROPORTION DATA
(o) OFF <-> NFF CONVERTER
(o) GPU Shader Tex3D
(o) APPLY TRANSFER FUNCTION
(o) VERTEX ARRAY

 *
 */

/// --------------------------------------   Definitions   -----------------------------------------

#include <GL/GLee.h>

extern "C" {
//#include "../../lcgtk/glslKernel/GLee.h"
#include <GL/glut.h> // gl-utility library
}

#include <sstream>

#include "bin.h"

#define CLEFT   -0.2 // cube 3d image
#define CRIGHT   1.2
#define CBOTTOM -0.2
#define CTOP     1.2
#define CNEAR   -2.0
#define CFAR     3.0
#define CBORDER  0.4

using namespace std;

#define QLEFT    0.0 // quad 2d image
#define QRIGHT   1.0
#define QBOTTOM  0.0
#define QTOP     1.0
#define QNEAR    1.0
#define QFAR     0.0

#define DEFPOS 50

/// ---------------------------------   Global Variables   --------------------------------------

/// Slice windows variables

static int xyWindow, yzWindow, xzWindow;

static const char titleXYWin[30] = "Image3D Viewer (XY)";
static const char titleYZWin[30] = "Image3D Viewer (YZ)";
static const char titleXZWin[30] = "Image3D Viewer (XZ)";

static int winWidth[3], winHeight[3];

static bool showInfo[3] = {true, true, true};
static bool showHelp[3] = {false, false, false};

static bool xyVis = true, yzVis = false, xzVis = false;

static bool flipHor[3] = {false, false, false};
static bool flipVer[3] = {false, false, false};

static uint sliceNum[3] = {0, 0, 0};
static slice_direction sliceDir; // current slice direction

static uint sliceX[3], sliceY[3];

static int currWin; // current window

/// Volume window variables

#ifdef _DWIN4

static int vol3DWindow;
static const char titleVol3DWin[30] = "Volume 3D Viewer";
static int vol3DWidth = 512, vol3DHeight = 512;

static int buttonPressed = -1; ///< button state

static GLfloat oldx = 0.0, oldy = 0.0, xangle = 0.0, yangle = 0.0;
static int xmouse, ymouse;

static bool showFPS = true; // show fps flag
static bool show3DHelp = false; // show 3d window help

static bool vol3DVis = false; // 3d window is visible

static bool rotate3D = false; // always rotating state

static GLuint volumeTex; // 3d texture
static uint volumeWidth, volumeHeight, volumeDepth, volumeSize;

#endif

/// 2d and 3d images

static Bin *binImage;
static PGM *pgmImage[3]; // one pgm image for each slice window

/// ----------------------------------   Functions   -------------------------------------

/// Kill application

bool killApp( bool closeAll = false ) { // destroy all windows if its all invisible

#ifdef _DWIN4

	if ( (!xyVis && !yzVis && !xzVis && !vol3DVis) || (closeAll) ) {

		glutDestroyWindow(xyWindow);
		glutDestroyWindow(yzWindow);
		glutDestroyWindow(xzWindow);
		glutDestroyWindow(vol3DWindow);

		return true;

	}

#else

	if ( (!xyVis && !yzVis && !xzVis) || (closeAll) ) {

		glutDestroyWindow(xyWindow);
		glutDestroyWindow(yzWindow);
		glutDestroyWindow(xzWindow);

		return true;

	}

#endif

	return false;

}

/// ------------------------------------   Slice   ---------------------------------------

/// Update slice direction

void updateSlice( void ) {

	currWin = glutGetWindow();

	if (currWin == xyWindow)
		sliceDir = sdXY;

	if (currWin == yzWindow)
		sliceDir = sdYZ;

	if (currWin == xzWindow)
		sliceDir = sdXZ;

}

/// Build PGM image

void buildPGM( void ) {

	if (pgmImage[sliceDir])
		delete pgmImage[sliceDir];

	pgmImage[sliceDir] =
		binImage->getSlice( sliceNum[sliceDir], sliceDir );

	if (flipHor[sliceDir])
		pgmImage[sliceDir]->flip( fdHorizontal );

	if (flipVer[sliceDir])
		pgmImage[sliceDir]->flip( fdVertical );

	if (!pgmImage[sliceDir]) {
		cerr << "Can't cut slice pgm image\n";
		killApp(true);
		return;
	}

	sliceX[sliceDir] = pgmImage[sliceDir]->getDimX();
	sliceY[sliceDir] = pgmImage[sliceDir]->getDimY();

}

/// Reshape
/// @arg w, h new window width and height

void reshape( int w, int h ) {

	updateSlice();

	double currPropW, currPropH, currProp;
	int newW=w, newH=h;

	/// Proportions
	currPropW = w / (double)sliceX[sliceDir];
	currPropH = h / (double)sliceY[sliceDir];
	currProp  = (currPropW > currPropH) ? currPropW : currPropH;

	/// New locked proportion size
	newW = (int)(currProp*sliceX[sliceDir]);
	newH = (int)(currProp*sliceY[sliceDir]);

	/// Its keep proportion, but not resize the window
	if ((newW == w) && (newH == h))
		glViewport(0, 0, winWidth[sliceDir] = w, winHeight[sliceDir] = h);
	else
		glutReshapeWindow(newW, newH);

}

/// OpenGL Write
/// @arg x, y raster position
/// @arg str string to write

void glWrite( GLdouble x, GLdouble y, char *str ) {

	// You should call glColor* before glWrite;
	// And the font color is also affected by:
	//   texture, lighting, fog...
	glRasterPos2d(x, y);
	for (char *s = str; *s; s++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);

}

/// Slice Information

void sliceInfo( void ) {

	ostringstream oss1;

	oss1 << ((sliceDir == sdXY)?"XY":((sliceDir == sdXZ)?"XZ":("YZ")))
	     << " : " << (sliceNum[sliceDir]+1) << " / " << (binImage->maxSliceNum(sliceDir))
	     << " : " << sliceX[sliceDir] << "x" << sliceY[sliceDir]
	     << ((flipHor[sliceDir])?" : Hor":"") << ((flipVer[sliceDir])?" : Ver":"");

	glColor3f(0.0, 0.0, 1.0);
	glWrite(QLEFT + 0.1, QBOTTOM + 0.05, (char*) oss1.str().c_str());
}

/// Help Information

void helpInfo( void ) {

	ostringstream oss[9];

	oss[0] << "??? Key bind HELP ???" << endl;
	oss[1] << " (s,?) show/hide slice/help information" << endl;
	oss[2] << " (w) write current slice in pgm format" << endl;
	oss[3] << " (p) keep proportion" << endl;
	oss[4] << " (h/v) flip slice horizontal/vertical" << endl;
#ifdef _DWIN4
	oss[5] << " (1,2,3,4) open view: XY, YZ, XZ, 3D" << endl;
#else
	oss[5] << " (1,2,3) open view: XY, YZ, XZ" << endl;
#endif
	oss[6] << " (up/down) next/previous slice" << endl;
	oss[7] << " (pgUp/pgDown) next/previous 10 slices" << endl;
	oss[8] << " (q,x/esc) q - quit ; x/esc - close" << endl;


	glColor3f(0.0, 1.0, 0.0);

	for (uint i=0; i<9; i++)
		glWrite(QLEFT + 0.1, (QTOP - 0.05) - i/10.0, (char*) oss[i].str().c_str());

}

/// Display

void display( void ) {

	updateSlice();

	glClear(GL_COLOR_BUFFER_BIT);

	glRasterPos2i(0, 0);

	glDrawPixels( sliceX[sliceDir], sliceY[sliceDir],
		      GL_LUMINANCE, GL_UNSIGNED_BYTE,
		      pgmImage[sliceDir]->getBuffer() );

	if (showInfo[sliceDir])
		sliceInfo();

	if (showHelp[sliceDir])
		helpInfo();

	glutSwapBuffers();

}

/// Special keyboard
/// @arg key keyboard special key hitted
/// @arg x, y pixel position when hit

void special( int key, int x, int y ) {

	updateSlice();

	switch(key) {
	case GLUT_KEY_UP: // next slice
		if (sliceNum[sliceDir] >= (binImage->maxSliceNum(sliceDir)-1))
			return;
		sliceNum[sliceDir]++;
		buildPGM();
		break;
	case GLUT_KEY_DOWN: // previous slice
		if (sliceNum[sliceDir] == 0)
			return;
		sliceNum[sliceDir]--;
		buildPGM();
		break;
	case GLUT_KEY_PAGE_UP: // next 10 slices
		if (sliceNum[sliceDir] >= (binImage->maxSliceNum(sliceDir))-1)
			return;
		if (sliceNum[sliceDir] <= (binImage->maxSliceNum(sliceDir))-11)
			sliceNum[sliceDir] += 10;
		else
			sliceNum[sliceDir] = (binImage->maxSliceNum(sliceDir))-1;
		buildPGM();
		break;
	case GLUT_KEY_PAGE_DOWN: // previous 10 slices
		if (sliceNum[sliceDir] == 0)
			return;
		if (sliceNum[sliceDir] >= 10)
			sliceNum[sliceDir] -= 10;
		else
			sliceNum[sliceDir] = 0;
		buildPGM();
		break;
	default: // any other special key
		cerr << "No key bind for " << key
		     << " in (" << x << ", " << y << ")" << endl;
		return;
	}

	glutPostRedisplay();

}

/// Keyboard
/// @arg key keyboard key hitted
/// @arg x, y pixel position when hit

void keyboard( unsigned char key, int x, int y ) {

	ostringstream oss;
	double currPropW, currPropH, currProp;
	int newW, newH;

	updateSlice();

	switch(key) {
	case '?': // show help
		showHelp[sliceDir] = !showHelp[sliceDir];
		break;
	case 's': case 'S': // show information
		showInfo[sliceDir] = !showInfo[sliceDir];
		break;
	case 'p': case 'P': // keep proportion
		currPropW = winWidth[sliceDir] / (double)sliceX[sliceDir];
		currPropH = winHeight[sliceDir] / (double)sliceY[sliceDir];
		currProp  = (currPropW > currPropH) ? currPropW : currPropH;
		newW = (int)(currProp*sliceX[sliceDir]);
		newH = (int)(currProp*sliceY[sliceDir]);
		glutReshapeWindow(newW, newH);
		break;
	case 'w': case 'W': // write pgm file image
		oss << binImage->getFileName() << "."
		    << ((sliceDir == sdXY)?"XY":((sliceDir == sdXZ)?"XZ":("YZ")))
		    << "." << (sliceNum[sliceDir]+1) << ".pgm";
		cout << "Writing pgm file: " << oss.str() << " ... ";
		pgmImage[sliceDir]->flip( fdVertical ); // opengl y axis problem...
		pgmImage[sliceDir]->write( (char*) oss.str().c_str() );
		pgmImage[sliceDir]->flip( fdVertical ); // ...get back to original
		cout << "done!" << endl;
		return;
	case 'h': case 'H': // flip horizontally
		pgmImage[sliceDir]->flip( fdHorizontal );
		flipHor[sliceDir] = !flipHor[sliceDir];
		break;
	case 'v': case 'V': // flip vertically
		pgmImage[sliceDir]->flip( fdVertical );
		flipVer[sliceDir] = !flipVer[sliceDir];
		break;
	case '1': // show window-1 view XY
		xyVis = !xyVis;
		glutSetWindow(xyWindow);
		if (xyVis) glutShowWindow();
		else glutHideWindow();
		if (killApp()) return;
		updateSlice();
		buildPGM();
		break;
	case '2': // show window-2 view YZ
		yzVis = !yzVis;
		glutSetWindow(yzWindow);
		if (yzVis) glutShowWindow();
		else glutHideWindow();
		if (killApp()) return;
		updateSlice();
		buildPGM();
		break;
	case '3': // show window-3 view XZ
		xzVis = !xzVis;
		glutSetWindow(xzWindow);
		if (xzVis) glutShowWindow();
		else glutHideWindow();
		if (killApp()) return;
		updateSlice();
		buildPGM();
		break;
#ifdef _DWIN4
	case '4': // show window-4 3d view
		vol3DVis = !vol3DVis;
		glutSetWindow(vol3DWindow);
		if (vol3DVis) glutShowWindow();
		else glutHideWindow();
		if (killApp()) return;
		break;
#endif
	case 'q': case 'Q': // quit application
		killApp(true);
		return;
	case 'x': case 'X': case 27: // close window
		if (currWin == xyWindow) xyVis = false;
		if (currWin == yzWindow) yzVis = false;
		if (currWin == xzWindow) xzVis = false;
#ifdef _DWIN4
		if (currWin == vol3DWindow) vol3DVis = false;
#endif
		if (killApp()) return;
		glutHideWindow();
		return;
	default: // any other key
		cerr << "No key bind for " << key
		     << " in (" << x << ", " << y << ")" << endl;
		return;
	}

	glutPostRedisplay();

}

/// ------------------------------------   Volume 3D   ----------------------------------------

#ifdef _DWIN4

/// Build 3D Texture

void build3DTexture( void ) {

	GLubyte *volumeBuffer;

	volumeWidth = binImage->getDimX() + 2; // 2 border limits
	volumeHeight = binImage->getDimY() + 2;
	volumeDepth = binImage->getDimZ() + 2;

	volumeSize = volumeWidth * volumeHeight * volumeDepth;

// 	volumeBuffer = new GLubyte[ volumeSize * 4 ]; // 1B

 	volumeBuffer = new GLubyte[ volumeSize * 2 ]; // 1B

	if (!volumeBuffer)
		return;

	cout << "* 3D Texture Allocating: " << (GLuint)(((volumeSize*2)/1000000.0)*100.0)/100.0 << " MB! "
	     << "Building..." << flush;

	uint idx;
	GLubyte b;
	for (uint i=0; i<volumeWidth; i++) {

		for (uint j=0; j<volumeHeight; j++) {

			for (uint k=0; k<volumeDepth; k++) {

				idx = i + (j + k*volumeHeight)*volumeWidth;

				if ( (i==0) || (j==0) || (k==0) || (i==volumeWidth-1) ||
				     (j==volumeHeight-1) || (k==volumeDepth-1) ) {

					/// Build a zero-border on texture to
					/// correct clampping
					b = 0;


				} else {
	
					b = binImage->getByteValue(i-1, j-1, k-1);

				}

				if( b < 50 ) b = 0; // test

// 				volumeBuffer[ 0 + idx*4 ] = b;
// 				volumeBuffer[ 1 + idx*4 ] = b;
// 				volumeBuffer[ 2 + idx*4 ] = b;
// 				volumeBuffer[ 3 + idx*4 ] = b;

				//cout << " " << idx << flush;

				if( idx >= volumeSize ) { std::cerr << "!error! texture index out-of-bounds!\n"; return; }

				volumeBuffer[ 0 + idx*2 ] = b;
				volumeBuffer[ 1 + idx*2 ] = b;

			}
		}
	}

	cout << "done! Setting..." << flush;

	glGenTextures(1, &volumeTex);
	glBindTexture(GL_TEXTURE_3D, volumeTex);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE8_ALPHA8, volumeWidth, volumeHeight,
		     volumeDepth, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, volumeBuffer);

	cout << "done!" << endl;

}

/// Reshape
/// @arg w, h new window width and height

void vol3DReshape( int w, int h ) {

    glViewport( 0, 0, vol3DWidth=w, vol3DHeight=h );

}

/// Help Information

void vol3DInfo( void ) {

	ostringstream oss[6];

	oss[0] << "??? Key bind HELP ???" << endl;
	oss[1] << " (?) show/hide 3D help information" << endl;
	oss[2] << " (1,2,3,4) open view: XY, YZ, XZ, 3D" << endl;
	oss[3] << " (f) show/hide FPS information" << endl;
	oss[4] << " (r) rotate 3D model" << endl;
	oss[5] << " (q,x/esc) q - quit ; x/esc - close" << endl;

	glColor3f(0.0, 1.0, 0.0);

	for (uint i=0; i<6; i++)
		glWrite(-0.3, 1.3 - i/10.0, (char*) oss[i].str().c_str());

}

/// FPS Information

void fpsInfo( double fps ) {

	ostringstream oss1;

	oss1 << "FPS: " << fps;

	glColor3f(0.0, 0.0, 1.0);

	glWrite(CLEFT + 0.1, CBOTTOM + 0.05, (char*) oss1.str().c_str());

}

/// Display

void vol3DDisplay( void ) {

	static uint numFrames = 0;
	static int dtBegin = 0, dtEnd = 0;
	static double frameTime = 0.0, elapsedTime = 0.0, fps = 0.0;

	static const GLdouble cbX[4] = { 0.0 - CBORDER, 1.0 + CBORDER, 1.0 + CBORDER, 0.0 - CBORDER };
	static const GLdouble cbY[4] = { 0.0 - CBORDER, 0.0 - CBORDER, 1.0 + CBORDER, 1.0 + CBORDER };

	dtBegin = glutGet(GLUT_ELAPSED_TIME); // start counting time

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glTranslated(0.5, 0.5, 0.5);

	glRotatef(oldx+xangle, 1.0, 0.0, 0.0);
	glRotatef(oldy+yangle, 0.0, 1.0, 0.0);

	glTranslated(-0.5, -0.5, -0.5);

	glEnable(GL_BLEND);

	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, volumeTex);

	glBegin(GL_QUADS); // try to use vertex array...

	uint nz = volumeDepth;

	for (uint z=0; z<nz; z++) {

		for (uint i=0; i<4; i++) {

			GLdouble dz = (1.0 + 2*(CBORDER))/(GLdouble)nz;

			glTexCoord3d( cbX[i], cbY[i],
				      (GLdouble)(z*dz - CBORDER) );

			glVertex3d( cbX[i], cbY[i],
				    (GLdouble)(z*dz - CBORDER) );

		}

	}

	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_3D);

	glLoadIdentity();

	if (showFPS)
		fpsInfo(fps);

	if (show3DHelp)
		vol3DInfo();

	glutSwapBuffers();

	dtEnd = glutGet(GLUT_ELAPSED_TIME); // stop counting time

	frameTime = (dtEnd - dtBegin) / 1000.0; // 1 frame time in seconds

	elapsedTime += frameTime;

	numFrames++;

	if (elapsedTime >= 2.0) {

		fps = (GLuint)((numFrames/elapsedTime)*100.0)/100.0;

		elapsedTime = 0.0;

		numFrames = 0;

	}

}

/// Keyboard
/// @arg key keyboard key hitted
/// @arg x, y pixel position when hit

void vol3DKeyboard( unsigned char key, int x, int y ) {

	switch(key) {
	case 'f': case 'F': // Show 3D help information
		showFPS = !showFPS;
		break;
	case '?': // Show 3D help information
		show3DHelp = !show3DHelp;
		break;
	case 'r': case 'R': // rotate 3D model
		rotate3D = !rotate3D;
		return;
	case '1': case '2': case '3': case '4': // same to the slice keyboard
	case 'q': case 'Q': case 'x': case 'X': // switch cases
	case 27:
		keyboard(key, x, y);
		return;
	default: // any other key
		cerr << "No key bind for " << key
		     << " in (" << x << ", " << y << ")" << endl;
		return;
	}

	glutPostRedisplay();

}

/// Mouse
/// @arg button pressed or released
/// @arg state of the button
/// @arg x, y window coordinate of the event

void vol3DMouse( GLint button, GLint state, GLint x, GLint y ) {

	buttonPressed = button;
	xmouse = x;
	ymouse = y;

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {

		oldx += xangle;
		oldy += yangle;
		if (oldx > 360.0) oldx -= 360.0;
		if (oldx < 0.0) oldx += 360.0;
		if (oldy > 360.0) oldy -= 360.0;
		if (oldy < 0.0) oldy += 360.0;
		xangle = 0.0;
		yangle = 0.0;

	}

}

/// Mouse Motion
/// @arg x, y current mouse position

void vol3DMotion( GLint x, GLint y ) {

	if (buttonPressed == GLUT_LEFT_BUTTON) {

		yangle = -(x - xmouse) * 360 / (GLfloat) vol3DWidth;
		xangle = -(y - ymouse) * 180 / (GLfloat) vol3DHeight;

		glutPostRedisplay();

	}



}

/// Idle

void vol3DIdle( void ) {

	if (rotate3D) {

		xangle += 0.1;
		yangle += 0.1;

		glutPostRedisplay();

	}

}

#endif // defined window-4

/// -------------------------------------   Main   -----------------------------------------

/// OpenGL Window Initialization

void openglWinInit( void ) {

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glShadeModel(GL_FLAT); // smoothless
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(QLEFT, QRIGHT, QBOTTOM, QTOP, QNEAR, QFAR);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);

}

int main(int argc, char** argv) {

	/// Initial set up
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	if( argc < 2 ) {

		cerr << "Usage: " << argv[0] << " file.bin" << endl;
		return 1;

	}

	binImage = new Bin( argv[1] );
	if (!binImage) {
		cerr << "Can't load binary image\n";
		return 1;
	}

	cout << "*** Binary image information ***" << endl
	     << "* Resolution: " << binImage->getDimX() << " x "
	     << binImage->getDimY() << " x " << binImage->getDimZ() << endl
	     << "* Dataset size: "
	     << ((uint)((binImage->getDataSize()/1000000.0)*100.0)/100.0) << " M" << endl
	     << "* Number of bytes: " << binImage->getNumBytes() << endl
	     << "* " << ((binImage->getEndian()==1)?"Little":"Big") << " endian" << endl
	     << "* Minimum value: " << binImage->getMinValue() << endl
	     << "* Maximum value: " << binImage->getMaxValue() << endl;

	/// XY Window
	sliceDir = sdXY;
	buildPGM();
	glutInitWindowSize(sliceX[sliceDir], sliceY[sliceDir]);
	glutInitWindowPosition(DEFPOS, DEFPOS);
	xyWindow = glutCreateWindow(titleXYWin);

	openglWinInit();

	if (!xyVis)
		glutHideWindow();

	/// YZ Window
	sliceDir = sdYZ;
	buildPGM();
	glutInitWindowSize(sliceX[sliceDir], sliceY[sliceDir]);
	glutInitWindowPosition(2*DEFPOS+sliceX[sliceDir], DEFPOS);
	yzWindow = glutCreateWindow(titleYZWin);

	openglWinInit();

	if (!yzVis)
		glutHideWindow();

	/// XZ Window
	sliceDir = sdXZ;
	buildPGM();
	glutInitWindowSize(sliceX[sliceDir], sliceY[sliceDir]);
	glutInitWindowPosition(2*DEFPOS+sliceX[sliceDir], 2*DEFPOS+sliceY[sliceDir]);
	xzWindow = glutCreateWindow(titleXZWin);

	openglWinInit();

	if (!xzVis)
		glutHideWindow();

#ifdef _DWIN4

	/// Volume 3D Window
	glutInitWindowSize(vol3DWidth, vol3DHeight);
	glutInitWindowPosition(DEFPOS, 2*DEFPOS+sliceY[sliceDir]);
	vol3DWindow = glutCreateWindow(titleVol3DWin);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(CLEFT, CRIGHT, CBOTTOM, CTOP, CNEAR, CFAR);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutReshapeFunc(vol3DReshape);
	glutDisplayFunc(vol3DDisplay);
	glutKeyboardFunc(vol3DKeyboard);
	glutMouseFunc(vol3DMouse); // UI mouse
	glutMotionFunc(vol3DMotion); // UI mouse motion

	glutIdleFunc(vol3DIdle);

	if (!vol3DVis)
		glutHideWindow();

	build3DTexture();

#endif

	/// Concluding
	glutSetWindow(xyWindow);

	cout << "Finished!" << endl;

	glutMainLoop();

	if (binImage)
		delete binImage;

	for (uint i=0; i<3; i++)
		if (pgmImage[i])
			delete pgmImage[i];

	killApp(true);

	return 0;

}
