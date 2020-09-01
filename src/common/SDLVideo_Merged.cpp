//
// the asm code's "interface" uses 640x400 screen space coordinates (mostly)
// and either 12 bit color (most rasterizing function) or palette entries
//
// double buffer hack was introduced because asm code assumes "copy" buffer
// swapping behaviour which is not the case for most gl implementations (especially in
// fullscreen mode) resulting in flickering menu's and non-flight screens this may 
// also be causing some of the fsaa anomalies (?)
//
// some array allocations are wastefull, but we are counting system ram by the gigs
// these days (2006) :-)
//
// why opengl 2.0 and glsl? because i was too lazy to implement dependent texture
// lookups using the "standard" extension thingies... otherwise the shaders are pretty
// trivial, and "backporting" to opengl 1.2 shouldn't be that big of a deal
//
// the libraries used are SDL v1.2.10, SDL_mixer v1.2.7 and glew v1.3.4
//
// the original sources were glffe v2.8a7 and aniso v0.9c tried to mark most of the changes 
// in the asm code but diff then against the originals to be sure...
//
// you might find some straight copy/pastes from nehe and other opengl tutorials
// i think i checked, and it should be okay to have them here
// 

#include <SDL.h>
#include <gl/glew.h>

#include <stdlib.h>
#include <string.h>
#include "ffecfg.h"
#include "ffeapi.h"

#include "sdlutil.h"
#include "std-pipeline.h"

extern "C" int asmmain (int argc, char **argv);

int main (int argc, char **argv)
{
	// Nick
	NickMain();

	return asmmain (argc, argv);
}

//////////////////
// glsl shaders //
//////////////////
GLuint	vs_tex;
GLuint	fs_tex;	   
GLuint	sp_tex;
GLuint	fs_flat;
GLuint	sp_flat;
GLuint	fs_char;
GLuint	sp_char;
GLuint	fs_true;
GLuint	sp_true;

////////////////////////////////////
// texture managment and surfaces //
////////////////////////////////////
static unsigned int pTextures[8192];
static unsigned int pLut;
static unsigned int pFont;
static unsigned int pBitmaps[8192];
static unsigned int pPal;
static bool			bReplacedBitmap[8192];
static SDL_Surface*	pScreen = 0;
static UCHAR		pPalTexture[256*5];		// Nick - Make bigger 3 -> 5
static UCHAR		pPalTable[3][64];

///////////////////
// configurables //
///////////////////
int	g_CfgFullscreen		= 0;
int	g_CfgAspectFix		= 0;
int	g_CfgFSWidth		= 640;
int	g_CfgFSHeight		= 400;
int	g_CfgFSbpp			= 16;
int	g_CfgWinWidth		= 640;
int	g_CfgWinHeight		= 400;
int	g_CfgFilterFont		= 0;
int	g_CfgFontWidth		= 8;
int g_CfgReplaceConsole	= 0; 
int g_CfgScannerOffsetX	= 0;
int g_CfgScannerOffsetY	= 0;

////////////////////////
// console replaement //
////////////////////////
unsigned int		pConTextures[10];
sMouseClickRemap	clickRemapIcon[40];
int					nNumPositions;
sBmpPosition*		posReplacement;
sBmpPosition		posReplacePylon[8];
sBmpReplacement		bmpLUT[400];
void ResetVideoSetPalValue ();


/////////////
// globals //
/////////////
int		g_nWidth;
int		g_nHeight;
int		g_nCorrectedWidth;
int		g_nCorrectedHeight;
bool	g_bDirtyBuffer;
int		g_nDoubleBufferHack;

////////////////////////////
// font managment related //
////////////////////////////
extern "C" int		ASMDrawChar(int c, int color, int pos, int posy, int miny, int maxy);
extern "C" UCHAR*	ASMGetBuffer();
static int			pCharAdvanceWidth[256];
static UCHAR		asmFont[256*256];

///////////////
// trig math //
///////////////
#include <math.h>
const float		fPi = 3.1415926f;
static float	sinLUT[256];
static float	cosLUT[256];

#include "shaders.h"

/////////////////////
// More Globals ;) //
/////////////////////

USHORT SDLTextureColors[256];
UCHAR buffer[1024*1024]; 
UCHAR lutBuffer[256*4];
static long* dblBufHack_bmp;
static int nPendingChars = 0;
static int nPendingBmps = 0;

const int		pylonXpos[8] = {  14,  33,   9,  38,   1,  43,   1,  36 };
const int		pylonYpos[8] = { 165, 164, 167, 166, 168, 169, 180, 179 };
bool	bCombatMfd;

struct CHARPARAMS {
	int		c;
	int		posx;
	int		posy;
	long	palcol;
};

static CHARPARAMS pPendingChar[2048];

static int nPendingStalks = 0;

struct STALKPARAMS {
	float		posx;
	float		posy;
	float		size;
	long	palcol;
};

static STALKPARAMS pPendingStalk[128];

struct BLITBMPPARAMS {
	int posx;
	int posy;
	long* bmp;
	int minx;
	int miny;
	int maxx;
	int maxy;
	int bmpnum;
};

static BLITBMPPARAMS pPendingBmp[512];

static long pal_count = 0;

/////////////////////
// Functions	   //
/////////////////////

// Test
//#undef GLEW_VERSION_2_0
//#define GLEW_VERSION_2_0 0
int nColorTableWidth = 32;

bool Nic_glColorTableEXT(GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void* data)
{
	if (glColorTableEXT) 
	{
		glColorTableEXT(target, internalFormat, width, format, type, data);
		return true;
	} 
	else
	{
		GLfloat mapR[256], mapG[256], mapB[256], mapA[256];

		unsigned char *pData = (unsigned char *)data;
		for ( int i = 0; i != width; i++ )
		{
			mapR[i] = ((float)*(pData++)) / 255.0;
			mapG[i] = ((float)*(pData++)) / 255.0;
			mapB[i] = ((float)*(pData++)) / 255.0;
			mapA[i] = ((float)*(pData++)) / 255.0;
		}

	//	glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_R, width, mapR);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_G, width, mapG);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_B, width, mapB);
		glPixelMapfv(GL_PIXEL_MAP_I_TO_A, width, mapA);

		return false;
	}
}

static int GLVideoInit ()
{
	int i;

	// Nick: Bit more unnecessary clean up
	nPendingStalks = 0;
	nPendingBmps = 0;
	nPendingChars = 0;
	ZeroMemory(pPendingStalk, sizeof(STALKPARAMS)*128);
	ZeroMemory(pPendingBmp, sizeof(BLITBMPPARAMS)*512);
	ZeroMemory(pPendingChar, sizeof(CHARPARAMS)*2048);
	ZeroMemory(lutBuffer, 256*3);
	ZeroMemory(buffer, 1024*1024);
	ZeroMemory(SDLTextureColors, 512);
	dblBufHack_bmp = NULL;

	// set up trig luts
	for(int count=0; count<256; count++) {
		sinLUT[count] = (float)sin(2.0f*fPi/256.0f*(float)count);
		cosLUT[count] = (float)cos(2.0f*fPi/256.0f*(float)count);
	}

	g_bDirtyBuffer = false;
	g_nDoubleBufferHack = 0;

	// Nick
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	if (g_CfgFullscreen) {
		if (g_CfgFSbpp == 32) {
			SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 32);
		}
		else {
			SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 5);
			SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 6);
			SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 5);
			SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 16);
		}
	}
	
	SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 0);
	SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 0);
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

	if (!g_CfgFullscreen) pScreen = SDL_SetVideoMode (g_nWidth, g_nHeight, 0, SDL_OPENGL);
	else pScreen = SDL_SetVideoMode (g_nWidth, g_nHeight, 0, SDL_OPENGL | SDL_FULLSCREEN);
	if (pScreen == 0) return 0;

	// SDL might init to closest fitting resolution to what was desired
	g_nWidth	= pScreen->w;
	g_nHeight	= pScreen->h;
	// fix viewport to desired aspect ratio if needed
	g_nCorrectedWidth  = g_nWidth;
	g_nCorrectedHeight = g_nHeight;
	if( g_CfgAspectFix ) {
		if( g_nWidth*10 > g_nHeight*16 ) {
			g_nCorrectedWidth = g_nHeight*16/10;			
		} else if( g_nHeight*16 > g_nWidth*10 ) {
			g_nCorrectedHeight = g_nWidth*10/16;
		}
	} 

    glViewport ((g_nWidth-g_nCorrectedWidth)/2, (g_nHeight-g_nCorrectedHeight)/2, g_nCorrectedWidth, g_nCorrectedHeight);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
	glOrtho( 0.0, 640.0, 400.0, 0.0, -1.0, 1.0 );
	glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
	glDisable (GL_LIGHTING);
	glDisable (GL_CULL_FACE);
	glDisable (GL_DEPTH_TEST);

	if (GLEW_VERSION_2_0)
	{
		glDisable (GL_BLEND);
	}
	else
	{
		glDepthMask(0);
		glDisable(GL_BLEND);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glAlphaFunc(GL_GREATER, 0.1f);
	}

	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	glClear (GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapBuffers ();
	glClear (GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapBuffers ();
	
	for (i=0; i<8192; i++)
	{
		pTextures[i]		= 0;
		pBitmaps[i]			= 0;
		bReplacedBitmap[i]	= false;
	}

	glewInit();
	
	if (GLEW_VERSION_2_0)
	{
		fprintf(stderr, "INFO: OpenGL 2.0 supported, proceeding\n");
	}
	else
	{
		// Nick
	//	MessageBox(NULL, "OpenGL 2.0 not supported by your graphics card!", "Graphics Error", MB_ICONWARNING | MB_OK );
		fprintf(stderr, "INFO: OpenGL 2.0 not supported. Exit\n");
		fprintf(stderr, "INFO: GLEW_EXT_paletted_texture %s. Exit\n", GLEW_EXT_paletted_texture ? "supported" : "not supported");
		fprintf(stderr, "INFO: GLEW_ARB_texture_env_combine %s. Exit\n", GLEW_ARB_texture_env_combine ? "supported" : "not supported");
		fprintf(stderr, "INFO: GLEW_EXT_texture_env_combine %s. Exit\n", GLEW_EXT_texture_env_combine ? "supported" : "not supported");
	//	return EXIT_FAILURE;
	}

	if (GLEW_VERSION_2_0)
	{
		// load and compile shaders
		const char *vsSource = VERTEXSHADERCODE;//file2string("shader.vert");
		vs_tex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs_tex, 1, &vsSource, NULL);
		glCompileShader(vs_tex);
		printLog(vs_tex);
		
		const char *fsSource = TEXFRAGMENTSHADERCODE;//file2string("tex.frag");
		fs_tex = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs_tex, 1, &fsSource, NULL);
		glCompileShader(fs_tex);
		printLog(fs_tex);
 
		fsSource = FLATFRAGMENTSHADERCODE;//file2string("flat.frag");
		fs_flat = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs_flat, 1, &fsSource, NULL);
		glCompileShader(fs_flat);
		printLog(fs_flat);

		fsSource = CHARFRAGMENTSHADERCODE;//file2string("char.frag");
		fs_char = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs_char, 1, &fsSource, NULL);
		glCompileShader(fs_char);
		printLog(fs_char);
 
		fsSource = TRUEFRAGMENTSHADERCODE;//file2string("true.frag");
		fs_true = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs_true, 1, &fsSource, NULL);
		glCompileShader(fs_true);
		printLog(fs_true);

		sp_tex = glCreateProgram();
		glAttachShader(sp_tex, vs_tex);
		glAttachShader(sp_tex, fs_tex);
		glLinkProgram(sp_tex);
		printLog(sp_tex);

		sp_flat = glCreateProgram();
		glAttachShader(sp_flat, vs_tex);
		glAttachShader(sp_flat, fs_flat);
		glLinkProgram(sp_flat);
		printLog(sp_flat);
 
		sp_char = glCreateProgram();
		glAttachShader(sp_char, vs_tex);
		glAttachShader(sp_char, fs_char);
		glLinkProgram(sp_char);
		printLog(sp_char);
 
		sp_true = glCreateProgram();
		glAttachShader(sp_true, vs_tex);
		glAttachShader(sp_true, fs_true);
		glLinkProgram(sp_true);
		printLog(sp_true);
	}

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	////////////////
	// font setup //
	////////////////
	// where's the 'old' buffer
	UCHAR* asmBuff = ASMGetBuffer();
	memset(asmBuff,0,sizeof(UCHAR)*640*400);
	memset(asmFont,0,sizeof(UCHAR)*256*128);
	// render all characters to 'old internal' buffer and store andvance g_nWidth
	for(int cChar=0; cChar<256; cChar++)
	{
		int row = cChar/16;
		int col = cChar%16;
		pCharAdvanceWidth[cChar] = ASMDrawChar(cChar,0xff,col*16,row*16,0,200);
	}
	// read to bitmap from 'old internal' buffer
	for(int cRow = 0; cRow<256; cRow++) {
		memcpy(&(asmFont[cRow*256]),asmBuff,256);
		asmBuff+=320;
	}


	if (GLEW_VERSION_2_0)
	{
		// create font texture from bitmap
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &pFont);
		glBindTexture(GL_TEXTURE_2D, pFont);
		glTexImage2D(GL_TEXTURE_2D, 0, 1, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, asmFont);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	}
	else
	{
		// create font texture from bitmap
		glGenTextures(1, &pFont);
		glBindTexture(GL_TEXTURE_2D, pFont);

		int newSZ = 256 * 256;
		char *pNewBuffer = new char[ newSZ * 2 ];
		for( int i = 0; i < newSZ; i++ )
		{
			pNewBuffer[ i * 2 + 0 ] = asmFont[i];
			pNewBuffer[ i * 2 + 1 ] = asmFont[i];
		}
		glTexImage2D(GL_TEXTURE_2D, 0, 2, 256, 256, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, pNewBuffer);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		delete pNewBuffer;
	}
	//////////////////////////////
	// load replacement bitmaps //
	//////////////////////////////
	// load cabin background
	SDL_Surface* cabin = LoadBMP("96.jpg");	/// Nick BMP->JPEG
	if( cabin != NULL ) {
		if (GLEW_VERSION_2_0)
		{
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
		}
		glGenTextures(1, &pBitmaps[96]);
		glBindTexture(GL_TEXTURE_2D, pBitmaps[96]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, cabin->w, cabin->h, 0, GL_RGB, GL_UNSIGNED_BYTE, cabin->pixels);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		SDL_FreeSurface(cabin);
		bReplacedBitmap[96] = true;
	}

	if (GLEW_VERSION_2_0)
	{
		// load crosshair
		SDL_Surface* crosshair = LoadBMP("crosshair.png"); /// Nick BMP->PNG
		if( crosshair != NULL ) {
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1, &pBitmaps[97]);
			glBindTexture(GL_TEXTURE_2D, pBitmaps[97]);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, crosshair->w, crosshair->h, 0, GL_RGB, GL_UNSIGNED_BYTE, crosshair->pixels);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			SDL_FreeSurface(crosshair);
			bReplacedBitmap[97] = true;
		}

		// load drift marker
		SDL_Surface* driftmarker = LoadBMP("driftmarker.png");	// Nick PNG
		if( driftmarker != NULL ) {
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1, &pBitmaps[98]);
			glBindTexture(GL_TEXTURE_2D, pBitmaps[98]);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, driftmarker->w, driftmarker->h, 0, GL_RGB, GL_UNSIGNED_BYTE, driftmarker->pixels);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			SDL_FreeSurface(driftmarker);
			bReplacedBitmap[98] = true;
		}
		
		/*
		// Nick - Load Missile (346)
		driftmarker = LoadBMP("miss0.png");	// Nick PNG
		if( driftmarker != NULL ) {
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1, &pBitmaps[346]);
			glBindTexture(GL_TEXTURE_2D, pBitmaps[346]);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, driftmarker->w, driftmarker->h, 0, GL_RGB, GL_UNSIGNED_BYTE, driftmarker->pixels);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			SDL_FreeSurface(driftmarker);
			bReplacedBitmap[346] = true;
		}

		// Nick - Load Missile (279)
		driftmarker = LoadBMP("miss1.png");	// Nick PNG
		if( driftmarker != NULL ) {
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1, &pBitmaps[279]);
			glBindTexture(GL_TEXTURE_2D, pBitmaps[279]);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, driftmarker->w, driftmarker->h, 0, GL_RGB, GL_UNSIGNED_BYTE, driftmarker->pixels);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			SDL_FreeSurface(driftmarker);
			bReplacedBitmap[279] = true;
		}*/
	}

	// load title
	SDL_Surface* title = LoadBMP("title.jpg");	/// Nick BMP->JPEG
	if( title != NULL ) 
	{	
		if (GLEW_VERSION_2_0)
		{
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
		}
		glGenTextures(1, &pBitmaps[66]);
		glBindTexture(GL_TEXTURE_2D, pBitmaps[66]);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, title->w, title->h, 0, GL_RGB, GL_UNSIGNED_BYTE, title->pixels);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		SDL_FreeSurface(title);
		bReplacedBitmap[66] = true;
	}
	if(g_CfgReplaceConsole) {
		// load replacement bmps
		for(i=0; i<10; i++) {
			char fname[128];
			sprintf(fname,"console%d.png",i);	/// Nick BMP->PNG
			SDL_Surface* surface = LoadBMP(fname);
			if( surface != NULL ) 
			{
				if (GLEW_VERSION_2_0)
				{
					glActiveTexture(GL_TEXTURE1);
					glEnable(GL_TEXTURE_2D);
				}
				glGenTextures(1, &pConTextures[i]);
				glBindTexture(GL_TEXTURE_2D, pConTextures[i]);
				if(surface->format->BytesPerPixel==3) {
					glTexImage2D(GL_TEXTURE_2D, 0, 3, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels);
				} else {
					glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
				}
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				SDL_FreeSurface(surface);
			}
		}
	}
	if (GLEW_VERSION_2_0)
		glGenTextures	( 1, &pLut );
	pPal = 0;
	// Nick - How big do you think pPalTexture is ? ;)
	if ( !pal_count )
		memset(pPalTexture,0,4*256*sizeof(UCHAR));
	else
		ResetVideoSetPalValue();
	
	if (GLEW_VERSION_2_0)
		glUseProgram(sp_tex);
	
	return 1;
}

static void VideoInitWorker ()
{
	if (g_CfgFullscreen)
	{
		g_nWidth = g_CfgFSWidth;
		g_nHeight = g_CfgFSHeight;
	}
	else {
		g_nWidth = g_CfgWinWidth;
		g_nHeight = g_CfgWinHeight;
	}	
	
	if( GLVideoInit() == 0 )
	{
		exit (0);
	}
}

void SetColorFrom12bit( GLint colorLocation, long col ) 
{
	USHORT color = col & 0xFFFF;
	UCHAR red = ((color>>8)&0xf);
	UCHAR gre = ((color>>4)&0xf);
	UCHAR blu = ((color>>0)&0xf);
	if (GLEW_VERSION_2_0)
		glUniform4f ( colorLocation, (float)red / 16.0f, (float)gre / 16.0f, (float)blu / 16.0f , 1.0f);
	else
		glColor3f( (float)red / 16.0f, (float)gre / 16.0f, (float)blu / 16.0f );
}

void SetColorFromPal( GLint colorLocation, long palcol ) 
{
	palcol &=0xff;
	if (GLEW_VERSION_2_0)
		glUniform4f ( colorLocation, (float)pPalTexture[palcol*3+0] / 256.0f, (float)pPalTexture[palcol*3+1] / 256.0f, (float)pPalTexture[palcol*3+2] / 256.0f , 1.0f);
	else
	{
		glColor4f
		( 
			(float)pPalTexture[palcol*4 + 0] / 256.0f, 
			(float)pPalTexture[palcol*4 + 1] / 256.0f, 
			(float)pPalTexture[palcol*4 + 2] / 256.0f,
			(float)pPalTexture[palcol*4 + 3] / 256.0f 
		);
	}
}

extern "C" unsigned char DATA_PlayerState;

extern "C" void SDLClearUpperView (long col)
{	
	GLint colorLocation = 0;
	g_bDirtyBuffer = true;
	glClear (GL_COLOR_BUFFER_BIT);

	if( col == 819 ) // system menu background
	{
		g_nDoubleBufferHack = 1;
	}

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();

	SetColorFrom12bit( colorLocation, col );
	
	glBegin (GL_QUADS);

	glVertex2i( -1,		-1	);
	glVertex2i( 641,	-1	);
	glVertex2i( 641,	317	);		
	glVertex2i( -1,		317	);
	
	glEnd ();

}

extern "C" void SDLDrawBoxToBuf (int x1, int y1, int x2, int y2, long palcol)
{	
	GLint colorLocation = 0;

	g_bDirtyBuffer = true;

	if(y2<310) g_nDoubleBufferHack = 0;

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();

	SetColorFromPal( colorLocation, palcol );
	
	glBegin (GL_QUADS);
	
	glVertex2i( x1,		y1	);
	glVertex2i( x1,		y2+1);
	glVertex2i( x2+1,	y2+1);
	glVertex2i( x2+1,	y1	);
	
	glEnd ();
}

const float fMaxArcLength = 4;

extern "C" void SDLDrawCircle (SHORT p1[2], long radius, long col)
{	
	
	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;
	GLint colorLocation = 0;

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();

	SetColorFrom12bit( colorLocation, col );
	
	int numSegments = (int)( 2.0f * fPi * (float)radius / fMaxArcLength );
	int segArc = 256 / (numSegments+1) - 1;
	if( segArc < 1) segArc = 1;
	if( segArc > 64) segArc = 64;
	
	glBegin (GL_POLYGON);

	for(int nSeg=0; nSeg<256; nSeg+=segArc) {
		glVertex2f( (float)p1[0]+(float)radius*cosLUT[nSeg],	(float)p1[1]-(float)radius*sinLUT[nSeg] );
	}

	glEnd ();

}

/** 
** FUNC_001612 - RMG - Brendan
** 
*/ 
extern "C" void SDLDrawCircle2(SHORT p1[2], long d, long palcol) //(Sint16 p1[2], const long d, const long palcol) - Brendan
{ 
        int radius = (d / 2); 
        g_bDirtyBuffer = true; 
        g_nDoubleBufferHack = 0; 
        glUseProgram(sp_flat); 
        GLint colorLocation = glGetUniformLocation (sp_flat, "color"); 
        //SetColorFromPal( colorLocation, palcol ); 
		SetColorFrom12bit( colorLocation, palcol ); // - Brendan
        int numSegments = (int)( 2.0f * fPi * (float)radius / fMaxArcLength ); 
        int segArc = 256 / (numSegments+1) - 1; 
        if ( segArc < 1) segArc = 1; 
        if ( segArc > 64) segArc = 64; 
        glBegin (GL_LINE_LOOP); 
        for( int nSeg = 0; nSeg < 256; nSeg += segArc ) 
        { 
                //glVertex2f( (float)p1[0]+(float)radius*cosLUT[nSeg],    (float)p1[1]-(float)radius*sinLUT[nSeg] ); 
			glVertex2f( (float)p1[0]*2+(float)radius*cosLUT[nSeg], (float)p1[1]*2-(float)radius*sinLUT[nSeg] );  // Brendan
        } 
        glEnd (); 
} 


extern "C" void SDLDrawParticle (SHORT p1[2], long radius, long col)
{	
	// SDLDrawCircle(p1,radius,col);
	SDLDrawCircle2(p1,radius,col);	// - RMG
}

extern "C" void SDLDrawScanLine (int xpos, int ypos, int length, long col)
{	

	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;
	GLint colorLocation = 0;

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();

	SetColorFrom12bit( colorLocation, col );
	
	glBegin (GL_QUADS);

	glVertex2i( xpos,			ypos	);
	glVertex2i( xpos+length,	ypos	);
	glVertex2i( xpos+length,	ypos+1	);
	glVertex2i( xpos,			ypos+1	);
	
	glEnd ();

}

extern "C" void SDLDrawLine (SHORT p1[2], SHORT p2[2], long col)
{	
	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;
	GLint colorLocation = 0;

	if( (p1[1]==p2[1]) && (p1[1]<1) ) return; // helps to get rid of "residual" lines on the edges of letterboxed viewport in nav screen mode

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();
	
	SetColorFrom12bit( colorLocation, col );
	
	glBegin (GL_LINES);

	glVertex2i( p1[0], p1[1] );
	glVertex2i( p2[0], p2[1] );

	glEnd ();
}

extern "C" void SDLDrawTriangleInternal (SHORT p1[2], SHORT p2[2], SHORT p3[2], long col)
{	
	
	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;
	GLint colorLocation = 0;

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();

	SetColorFrom12bit( colorLocation, col );
	
	glBegin (GL_TRIANGLES);

	glVertex2i( p1[0], p1[1] );
	glVertex2i( p2[0], p2[1] );
	glVertex2i( p3[0], p3[1] );
	
	glEnd ();

}

extern "C" void SDLDrawQuadInternal (SHORT p1[2], SHORT p2[2], SHORT p3[2], SHORT p4[2], long col)
{	
	GLint colorLocation = 0;

	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();
	
	SetColorFrom12bit( colorLocation, col );
	
	glBegin (GL_QUADS);

	glVertex2i( p1[0], p1[1] );
	glVertex2i( p2[0], p2[1] );
	glVertex2i( p3[0], p3[1] );
	glVertex2i( p4[0], p4[1] );
	
	glEnd ();
}

struct SPANDATA {
	int numpoints;
	int xpoint[10];
};
struct SPANS {
 	SPANDATA stuff[316];
 	int needshclip;
	int ymin;
	int ymax;
};

extern "C" void SDLDrawSpans (SPANS* spans, long col)
{	
	GLint colorLocation = 0;

	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_flat, "color");
	}
	else
		Use_FLAT_Shader();

	SetColorFrom12bit( colorLocation, col );
	
	if(spans->ymax==-1) spans->ymax = 316;

	int y;
	int numpairs;
	for( y=spans->ymin; y<spans->ymax; y++ ) {
		numpairs= spans->stuff[y].numpoints/2;
		if(numpairs>0) {
			for(int i=0; i<numpairs*2-1; i++) {
				for(int j=i+1; j<numpairs*2; j++) {
					if(spans->stuff[y].xpoint[i]>spans->stuff[y].xpoint[j]) {
						int tmp = spans->stuff[y].xpoint[i];
						spans->stuff[y].xpoint[i] = spans->stuff[y].xpoint[j];
						spans->stuff[y].xpoint[j] = tmp;
					}
				}
			}
		}
	}

	int processedRows = spans->ymin;
	int currentRow = processedRows;
	int slabs = spans->stuff[currentRow].numpoints/2;
	while ( currentRow < spans->ymax ) {
		while ( (slabs == spans->stuff[currentRow].numpoints/2) ) 
		{
			currentRow++;
			if(currentRow == spans->ymax) break;
		}
		for(int cSlab=0; cSlab<slabs; cSlab++) {
			glBegin(GL_TRIANGLE_STRIP);
			int k,x1,x2;
			for(k=processedRows; k<currentRow; k++) {
				x1 = spans->stuff[k].xpoint[cSlab*2];
				x2 = spans->stuff[k].xpoint[cSlab*2+1] + 1;
				glVertex2i( x1, k );
				glVertex2i( x2, k );
			}
			glVertex2i( x1, k );
			glVertex2i( x2, k );
			glEnd();
		}
		processedRows = currentRow;
		slabs = spans->stuff[currentRow].numpoints/2;
	}
}

// Nick: Very HACKY!
unsigned short SDLTextureColors_old[8192][256] = {0};
//unsigned char TextureFlipFlop[8192] = {0};

bool MemCmp(unsigned short *a, unsigned short *b, int size, int threshold)
{
	int count = 0;
	for ( int i = 0; i != size; i++ )
		if ( a[i] != b[i] )
		{
			count++;
			if ( count > threshold )
				return false;
		}
	return true;
}

void CreateTexture_Old( int texnum, UCHAR *texels, int sizex, int sizey, UCHAR flag )
{	
    Use_TEX_Shader(flag);

    for( int u = 0; u < 256; u++ )
    {
        int u4 = 4 * u;
		lutBuffer[u4 + 0] = ((SDLTextureColors[u]>>8)&0xf)<<4;
		lutBuffer[u4 + 1] = ((SDLTextureColors[u]>>4)&0xf)<<4;
		lutBuffer[u4 + 2] = ((SDLTextureColors[u]>>0)&0xf)<<4;
        lutBuffer[u4 + 3] = u > 1 ? 255 : 0;
	}


	if( pTextures[texnum] == 0 ) 
    {
		glGenTextures	( 1, &pTextures[texnum] );
		glBindTexture	(GL_TEXTURE_2D, pTextures[texnum]);
		glTexImage2D	(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, sizex, sizey, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, texels);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} 
    else 
    {
		glBindTexture (GL_TEXTURE_2D, pTextures[texnum]);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	
	glEnable(GL_TEXTURE_2D);
	
	if ( !MemCmp(SDLTextureColors_old[texnum], SDLTextureColors, nColorTableWidth, 3) )
	{
		if ( !Nic_glColorTableEXT(GL_TEXTURE_2D, GL_RGBA, nColorTableWidth, GL_RGBA, GL_UNSIGNED_BYTE, lutBuffer) )
			glTexImage2D	(GL_TEXTURE_2D, 0, 4, sizex, sizey, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, texels);
		memcpy(SDLTextureColors_old[texnum], SDLTextureColors, nColorTableWidth*2);
	}
}

void CreateTexture(int texnum, UCHAR* texels, int sizex, int sizey, UCHAR flag) {
	
	glUseProgram(sp_tex);

	// Nick - int 3 on texnum
	if ( texnum >= 8192 )
		__asm int 3;

	if( pTextures[texnum] == 0 ) {
		glGenTextures	( 1, &pTextures[texnum] );
		glActiveTexture(GL_TEXTURE0);
		glBindTexture	(GL_TEXTURE_2D, pTextures[texnum]);
		glTexImage2D	(GL_TEXTURE_2D, 0, 1, sizex, sizey, 0, GL_RED, GL_UNSIGNED_BYTE, texels);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, pTextures[texnum]);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glEnable(GL_TEXTURE_2D);
	GLint textureLocation = glGetUniformLocation (sp_tex, "tex");
	glUniform1i ( textureLocation, 0 ); 

	for(int u=0;u<256;u++) {
		lutBuffer[3*u+0] = ((SDLTextureColors[u]>>8)&0xf)<<4;
		lutBuffer[3*u+1] = ((SDLTextureColors[u]>>4)&0xf)<<4;
		lutBuffer[3*u+2] = ((SDLTextureColors[u]>>0)&0xf)<<4;
	}
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, pLut);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, lutBuffer);
	glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glEnable(GL_TEXTURE_1D);
	GLint lutLocation = glGetUniformLocation (sp_tex, "lut");
	glUniform1i ( lutLocation, 1 ); 

	GLint traLocation = glGetUniformLocation (sp_tex, "tra");
	glUniform1i ( traLocation, flag );	
}

// Nick
// #define min(a,b) ((a)<(b)?(a):(b))
// #define max(a,b) ((a)>(b)?(a):(b))

extern "C" void SDLDrawTexTriangleInternal (long* texels, SHORT p1[2], SHORT p2[2], SHORT p3[2], SHORT t1[2], SHORT t2[2], SHORT t3[2], long texnum)
{	
	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;

	UCHAR flag	= *( ((UCHAR*)texels) - 8 );
	int sizey	= *( ((UCHAR*)texels) - 7 );
	int sizex	= *( (USHORT*) (((UCHAR*)texels)-6) );
	if(sizey==0) {
		sizey = 512;
	} 
	
	if (GLEW_VERSION_2_0)
		CreateTexture(texnum,(UCHAR*)texels,sizex,sizey,flag);
	else
		CreateTexture_Old(texnum,(UCHAR*)texels,sizex,sizey,flag);
	
	t1[0] = max(1,min(sizex-1,t1[0]));
	t2[0] = max(1,min(sizex-1,t2[0]));
	t3[0] = max(1,min(sizex-1,t3[0]));
	t1[1] = max(1,min(sizey-1,t1[1]));
	t2[1] = max(1,min(sizey-1,t2[1]));
	t3[1] = max(1,min(sizey-1,t3[1]));

	glBegin (GL_TRIANGLES);

	glMultiTexCoord2f (GL_TEXTURE0, ((float)t1[0]) / (float)sizex , ((float)t1[1]) / (float) sizey );
	glVertex2i(	p1[0], p1[1] );
	glMultiTexCoord2f (GL_TEXTURE0, ((float)t2[0]) / (float)sizex , ((float)t2[1]) / (float) sizey );
	glVertex2i(	p2[0], p2[1] );
	glMultiTexCoord2f (GL_TEXTURE0, ((float)t3[0]) / (float)sizex , ((float)t3[1]) / (float) sizey );
	glVertex2i(	p3[0], p3[1] );
	
	glEnd ();	
}

extern "C" void SDLDrawTexQuadInternal (long* texels, SHORT p1[2], SHORT p2[2], SHORT p3[2], SHORT p4[2], SHORT t1[2], SHORT t2[2], SHORT t3[2], SHORT t4[2], long texnum)
{
	g_bDirtyBuffer = true;
	g_nDoubleBufferHack = 0;

	UCHAR flag	= *( ((UCHAR*)texels) - 8 );
	int sizey	= *( ((UCHAR*)texels) - 7 );
	int sizex	= *( (USHORT*) (((UCHAR*)texels)-6) );
	if(sizey==0) {
		sizey = 512;
	} 

	if (GLEW_VERSION_2_0)
		CreateTexture(texnum,(UCHAR*)texels,sizex,sizey,flag);
	else
		CreateTexture_Old(texnum,(UCHAR*)texels,sizex,sizey,flag);
	
	t1[0] = max(1,min(sizex-1,t1[0]));
	t2[0] = max(1,min(sizex-1,t2[0]));
	t3[0] = max(1,min(sizex-1,t3[0]));
	t4[0] = max(1,min(sizex-1,t4[0]));
	t1[1] = max(1,min(sizey-1,t1[1]));
	t2[1] = max(1,min(sizey-1,t2[1]));
	t3[1] = max(1,min(sizey-1,t3[1]));
	t4[1] = max(1,min(sizey-1,t4[1]));

	glBegin (GL_QUADS);
	
	glMultiTexCoord2fARB (GL_TEXTURE0, ((float)t1[0]) / (float)sizex , ((float)t1[1]) / (float) sizey );
	glVertex2i(	p1[0], p1[1] );
	glMultiTexCoord2fARB (GL_TEXTURE0, ((float)t2[0]) / (float)sizex , ((float)t2[1]) / (float) sizey );
	glVertex2i(	p2[0], p2[1] );
	glMultiTexCoord2fARB (GL_TEXTURE0, ((float)t3[0]) / (float)sizex , ((float)t3[1]) / (float) sizey );
	glVertex2i(	p3[0], p3[1] );
	glMultiTexCoord2fARB (GL_TEXTURE0, ((float)t4[0]) / (float)sizex , ((float)t4[1]) / (float) sizey );
	glVertex2i(	p4[0], p4[1] );
	
	glEnd ();
	
}

void PreDoChars()
{
	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_char);
		glActiveTexture(GL_TEXTURE0);
	}

	glBindTexture(GL_TEXTURE_2D, pFont);

	if (!GLEW_VERSION_2_0)
		User_CHAR_Shader();

	if(g_CfgFilterFont) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glEnable(GL_TEXTURE_2D);
	
	if (GLEW_VERSION_2_0)
	{
		GLint textureLocation = glGetUniformLocation (sp_char, "tex");
		glUniform1i ( textureLocation, 0 );
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void PostDoChars()
{	
	glDisable(GL_BLEND);
}

void DoChar(int c, int posx, int posy, long palcol)
{	
	float s,t;

	int rw = c/16;
	int cl = c%16;
	GLint colorLocation = 0;

	s = (float) cl/16.0f;
	t = (float) rw/16.0f;

	if (GLEW_VERSION_2_0)
		colorLocation = glGetUniformLocation (sp_char, "color");
	SetColorFromPal( colorLocation, palcol );

	glBegin(GL_QUADS);
	
	glMultiTexCoord2fARB (GL_TEXTURE0, s,				t );
	glVertex2i   ( 2*posx,		2*posy		);
	glMultiTexCoord2fARB (GL_TEXTURE0, s+1.0f/32.0f,	t );
	glVertex2i   ( 2*posx+g_CfgFontWidth,	2*posy		);
	glMultiTexCoord2fARB (GL_TEXTURE0, s+1.0f/32.0f,	t+1.0f/28.5f );
	glVertex2i   ( 2*posx+g_CfgFontWidth,	2*posy+16	);
	glMultiTexCoord2fARB (GL_TEXTURE0, s,				t+1.0f/28.5f );
	glVertex2i   ( 2*posx,		2*posy+16	);
	
	glEnd();

}

void DrawPendingChars()
{
	if(nPendingChars==0) return;
	PreDoChars();
	for(int i=0; i<nPendingChars; i++)
	{
		CHARPARAMS p = pPendingChar[i];
		DoChar(p.c, p.posx, p.posy, p.palcol);
	}
	PostDoChars();
	if( g_nDoubleBufferHack == 0 ) nPendingChars = 0;
}

// returns character g_nWidth (scaled to 320x200)
extern "C" long SDLDrawChar (int c, int posx, int posy, long palcol, int miny)
{
	c &= 0xff;	
	
	if(posy < miny) return pCharAdvanceWidth[c];

	g_bDirtyBuffer = true;

	if(palcol==0xf0) palcol=0x11;
	if(palcol==0xf00) palcol=0x10;
	
	if(nPendingChars==2048) {
		printf("WARNING: Too many chars to cue!\n");
		return pCharAdvanceWidth[c];
	}

	pPendingChar[nPendingChars].c		= c;
	pPendingChar[nPendingChars].posx	= posx;
	pPendingChar[nPendingChars].posy	= posy;
	pPendingChar[nPendingChars].palcol	= palcol;
	
	nPendingChars++;
	
	return pCharAdvanceWidth[c];
}

void DoDrawStalk(float posx, float posy, float size, long palcol)
{
	GLint colorLocation = 0;

	if (GLEW_VERSION_2_0)
	{
		glUseProgram(sp_flat);
		colorLocation = glGetUniformLocation (sp_char, "color");
	}
	else
		Use_FLAT_Shader();
	
	SetColorFromPal( colorLocation, palcol );
	
	int radius = 3;

	glBegin( GL_LINES );

	glVertex2f( posx, posy );
	glVertex2f( posx, posy-size );
	
	glEnd();

	posy -= size;

	int numSegments = (int)( 2.0f * fPi * (float)radius / fMaxArcLength );
	int segArc = 256 / (numSegments+1) - 1;
	if( segArc < 1) segArc = 1;
	if( segArc > 64) segArc = 64;
	
	glBegin (GL_POLYGON);

	for(int nSeg=0; nSeg<256; nSeg+=segArc) {
		glVertex2f( (float)posx+(float)radius*cosLUT[nSeg],	(float)posy-(float)radius*sinLUT[nSeg] );
	}

	glEnd ();
}

void DrawPendingStalks()
{

	for(int i=0; i<nPendingStalks; i++)
	{
		STALKPARAMS p = pPendingStalk[i];
		DoDrawStalk(p.posx,p.posy,p.size,p.palcol);
	}
	if( g_nDoubleBufferHack == 0 ) nPendingStalks = 0;
}

extern "C" void SDLDrawScannerStalk (SHORT p1[2], int size, long palcol)
{
	g_bDirtyBuffer = true;

	if(nPendingStalks==128) {
		printf("WARNING: Too many stalks to cue!\n");
		return;
	}

	pPendingStalk[nPendingStalks].size		= (float)size/2.0f;
	pPendingStalk[nPendingStalks].posx		= (float)p1[0]/2.0f + (float)g_CfgScannerOffsetX; 
	pPendingStalk[nPendingStalks].posy		= (float)p1[1]/2.0f + (float)g_CfgScannerOffsetY;
	pPendingStalk[nPendingStalks].palcol	= palcol;
	
	nPendingStalks++;
}

void CreateBmpTexture(int bmpnum, UCHAR* texels, int sizex, int sizey, UCHAR flag, float &smax, float &tmax) 
{
	if( bReplacedBitmap[bmpnum] ) 
	{
		if(bmpnum==97 || bmpnum==98)
		{
			if (GLEW_VERSION_2_0)
				glUseProgram(sp_char);
			else
				User_CHAR_Shader();
		}
		else
		{
			if (GLEW_VERSION_2_0)
				glUseProgram(sp_true);
			else
				Use_TRUE_Shader();
		}
	} 
	else 
	{
		if (GLEW_VERSION_2_0)
			glUseProgram(sp_tex);
		else
			Use_TEX_Shader(flag);
	}

	int glsizex = 1;
	int glsizey = 1;

	if(bReplacedBitmap[bmpnum]) 
	{
		smax = 1.0f;
		tmax = 1.0f;
	} 
	else 
	{
		while( glsizex < sizex ) glsizex *= 2;
		while( glsizey < sizey ) glsizey *= 2;
		smax = ((float)sizex-0.5f) / (float)glsizex;
		tmax = ((float)sizey-0.5f) / (float)glsizey;
	}
	
	if( pBitmaps[bmpnum] == 0 ) 
	{
		glGenTextures	( 1, &pBitmaps[bmpnum] );

		if (GLEW_VERSION_2_0)
			glActiveTexture(GL_TEXTURE0);

		glBindTexture	(GL_TEXTURE_2D, pBitmaps[bmpnum]);

		for(int y=0; y<sizey; y++) 
		{
			memcpy( &(buffer[y*glsizex]), &(texels[y*sizex]), sizeof(UCHAR)*sizex );
		}

		if (GLEW_VERSION_2_0)
			glTexImage2D	(GL_TEXTURE_2D, 0, 1, glsizex, glsizey, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);
		else
			glTexImage2D	(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, glsizex, glsizey, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buffer);

		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

		if (!GLEW_VERSION_2_0)
		{
			if ( !Nic_glColorTableEXT(GL_TEXTURE_2D, GL_RGBA, nColorTableWidth, GL_RGBA, GL_UNSIGNED_BYTE, pPalTexture) )
				glTexImage2D(GL_TEXTURE_2D, 0, 4, glsizex, glsizey, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, buffer);
		}
	} 
	else 
	{
		if (GLEW_VERSION_2_0)
			glActiveTexture(GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, pBitmaps[bmpnum]);
		if( bReplacedBitmap[bmpnum] ) 
		{
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		}
	}

	glEnable(GL_TEXTURE_2D);



	if (GLEW_VERSION_2_0)
	{
		GLint textureLocation = glGetUniformLocation (sp_tex, "tex");
		glUniform1i ( textureLocation, 0 ); 
	}

	if( bReplacedBitmap[bmpnum] ) 
	{
		return;
	}

	if (GLEW_VERSION_2_0)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, pPal);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		glEnable(GL_TEXTURE_1D);
		GLint lutLocation = glGetUniformLocation (sp_tex, "lut");
		glUniform1i ( lutLocation, 1 ); 

		GLint traLocation = glGetUniformLocation (sp_tex, "tra");
		glUniform1i ( traLocation, flag );
	}
}

bool cliptex(int& x1, int& y1, int &x2, int& y2, float& s1, float& t1, float& s2, float &t2, 
			 int minx, int miny, int maxx, int maxy) 
{
	minx*=2;
	maxx*=2;
	miny*=2;
	maxy*=2;

	if(x2<minx) return false;
	if(y2<miny) return false;
	if(x1>maxx) return false;
	if(y1>maxy) return false;
	
	float sizex = s2-s1;
	float sizey = t2-t1;
	float isizex = (float)(x2-x1);
	float isizey = (float)(y2-y1);

	int tmp;

	if(x1<minx) {
		tmp = x1;
		x1 = minx;
		s1 += (float)(x1-tmp)/isizex*sizex;
	}

	if(x2>maxx) {
		tmp = x2;
		x2 = maxx;
		s2 -= (float)(tmp-x2)/isizex*sizex;
	}

	if(y1<miny) {
		tmp = y1;
		y1 = miny;
		t1 += (float)(y1-tmp)/isizey*sizey;
	}

	if(y2>maxy) {
		tmp = y2;
		y2 = maxy;
		t2 -= (float)(tmp-y2)/isizey*sizey;
	}

	return true;
}

void DoBlitIcon(int bmpnum, float gaugeFraction = 0.0f, int pylon = -1) {
	
	if( bCombatMfd && (bmpnum==163) ) bmpnum=178;  // always display red nav/comb switch icon if in combat mode
	
	if( bmpLUT[bmpnum].nMode < 0 ) 
	{
		if (GLEW_VERSION_2_0)
			glUseProgram(sp_true);
		else
			Use_TRUE_Shader();
	} 
	else 
	{
		if (GLEW_VERSION_2_0)
			glUseProgram(sp_char);
		else
			User_CHAR_Shader();
	}

	if (GLEW_VERSION_2_0)
		glActiveTexture(GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, pConTextures[bmpLUT[bmpnum].nTextureIndex]);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glEnable(GL_TEXTURE_2D);
	
	GLint textureLocation;

	if( bmpLUT[bmpnum].nMode < 0 ) 
	{
		if (GLEW_VERSION_2_0)
		{
			textureLocation = glGetUniformLocation (sp_true, "tex");
			glUniform1i ( textureLocation, 0 );
		}
	} 
	else 
	{
		GLint colorLocation = 0;
		if (GLEW_VERSION_2_0)
		{
			textureLocation = glGetUniformLocation (sp_char, "tex");
			glUniform1i ( textureLocation, 0 );
			colorLocation = glGetUniformLocation (sp_char, "color");
		}
		SetColorFrom12bit( colorLocation, bmpLUT[bmpnum].nMode );
	}

	int		x1,x2,y1,y2;
	float	s1,t1,s2,t2;

	x1 = posReplacement[bmpLUT[bmpnum].nDstPosIndex].left;
	y1 = posReplacement[bmpLUT[bmpnum].nDstPosIndex].top;
	x2 = posReplacement[bmpLUT[bmpnum].nDstPosIndex].right;
	y2 = posReplacement[bmpLUT[bmpnum].nDstPosIndex].bottom;

	s1 = (float)posReplacement[bmpLUT[bmpnum].nSrcPosIndex].left	/ 640.0f;
	s2 = (float)posReplacement[bmpLUT[bmpnum].nSrcPosIndex].right	/ 640.0f;
	t1 = (float)posReplacement[bmpLUT[bmpnum].nSrcPosIndex].top		/ 84.0f;
	t2 = (float)posReplacement[bmpLUT[bmpnum].nSrcPosIndex].bottom	/ 84.0f;

	if( bmpnum==273 || bmpnum==274 || bmpnum==275 )	// gages
	{
		y1 = (int)( (float)(y1-y2)*gaugeFraction+(float)y2);
		s1 = (s1-s2)*gaugeFraction+s2;
	}

	if( pylon != -1 ){
		int mod = 0;

		x1 = posReplacePylon[pylon].left;
		y1 = posReplacePylon[pylon].top+mod;
		x2 = posReplacePylon[pylon].right;
		y2 = posReplacePylon[pylon].bottom+mod;
	}

	if( bmpLUT[bmpnum].nMode > -2 ) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBegin (GL_QUADS);
	
	glMultiTexCoord2fARB (GL_TEXTURE0, s1 , t1 );
	glVertex2i( x1, y1+316 );
	glMultiTexCoord2fARB (GL_TEXTURE0, s2 , t1 );
	glVertex2i( x2, y1+316 );
	glMultiTexCoord2fARB (GL_TEXTURE0, s2 , t2 );
	glVertex2i( x2, y2+316 );
	glMultiTexCoord2fARB (GL_TEXTURE0, s1 , t2 );
	glVertex2i( x1, y2+316 );
	
	glEnd ();

	glDisable(GL_BLEND);
};


void DoBlitClip (int posx, int posy, long* bmp, int minx, int miny, int maxx, int maxy, int bmpnum) {

	if( bmpnum==281 ) return;  // rewind/forward thingy for orbital maps, yuck!
	
	int pylon = -1;
	if( g_CfgReplaceConsole ) {
	
		if( bCombatMfd ) {
			for(int cPylon=0; cPylon<8; cPylon++) {
				if( (posx==pylonXpos[cPylon]) && (posy==pylonYpos[cPylon]) ) {
					pylon = cPylon;
					break;
				}
			}
		}

		if ( bmpnum == 346 )
			DoBlitIcon(bmpnum,(float)(173-miny)/11.0f,pylon);
		else
		if( ((bmpnum<300) && (bmpLUT[bmpnum].nTextureIndex > -1)) ) //|| bmpnum == 346) 
		{
			DoBlitIcon(bmpnum,(float)(173-miny)/11.0f,pylon);
			return;
		}

		if(bmpnum>=254 && bmpnum<=261) return; // rotating scanner disc, yuck!
	}

	UCHAR* texels = ((UCHAR*)bmp)+8;

	UCHAR flag	= *( texels - 8 );
	int sizey	= *( texels - 7 );
	int sizex	= *( (USHORT*) (texels-6) );
	if(sizey==0) {
		sizey = 512;
	} 
	
	float s1,t1,s2,t2;
	s1=t1=0.0f;
	CreateBmpTexture(bmpnum,(UCHAR*)texels,((flag>1)?2:1)*sizex,((flag>1)?2:1)*sizey,(flag&0x1),s2,t2);
	
	int x1 = posx*2;
	int y1 = posy*2;
	int x2 = x1+sizex*2;
	int y2 = y1+sizey*2;
	if(maxx<0) maxx=640;
	if(maxy<0) maxy=400;

	if( !cliptex(x1,y1,x2,y2,s1,t1,s2,t2,minx,miny,maxx,maxy) ) return; 
	
	if( pylon != -1 ){
		x1 = posReplacePylon[pylon].left;
		y1 = posReplacePylon[pylon].top+316;
		x2 = posReplacePylon[pylon].right;
		y2 = posReplacePylon[pylon].bottom+316;
	}

	if( ((bmpnum==97) || (bmpnum==98) || (bmpnum==66)) && bReplacedBitmap[bmpnum] ) 
	{
		GLint colorLocation = 0;
		if (GLEW_VERSION_2_0)
			colorLocation = glGetUniformLocation (sp_char, "color");
		SetColorFromPal( colorLocation, 15 );
	} 

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin (GL_QUADS);
	
	glMultiTexCoord2fARB (GL_TEXTURE0, s1 , t1 );
	glVertex2i( x1, y1 );
	glMultiTexCoord2fARB (GL_TEXTURE0, s2 , t1 );
	glVertex2i( x2, y1 );
	glMultiTexCoord2fARB (GL_TEXTURE0, s2 , t2 );
	glVertex2i( x2, y2 );
	glMultiTexCoord2fARB (GL_TEXTURE0, s1 , t2 );
	glVertex2i( x1, y2 );
	
	glEnd ();

	glDisable(GL_BLEND);
	if( g_CfgReplaceConsole && (bmpnum!=96) && (posy>155) && (bmpLUT[bmpnum].nTextureIndex>-2) ) {
		bmpLUT[bmpnum].nTextureIndex = -2;
		fprintf(stdout,"No replacement for bitmap index %d\n",bmpnum);
		fflush(stdout);
	}
}

void BlitPendingBmps()
{
	for(int i=0; i<nPendingBmps; i++)
	{
		BLITBMPPARAMS p = pPendingBmp[i];
		DoBlitClip(p.posx, p.posy, p.bmp, p.minx, p.miny, p.maxx, p.maxy, p.bmpnum);
	}
	if( g_nDoubleBufferHack == 0 ) nPendingBmps = 0;
}

extern "C" void SDLBlitClip (int posx, int posy, long* bmp, int minx, int miny, int maxx, int maxy, int bmpnum)
{
	g_bDirtyBuffer = true;

	if( (bmpnum==96) )	// cabin background
	{	
		DoBlitClip(posx, posy, bmp, minx, miny, maxx, maxy, bmpnum);
		if( (minx==0) && (maxx==-1) && (miny==0) && (maxy==-1) ) {
			dblBufHack_bmp		= bmp;
			g_nDoubleBufferHack = 2;
		}
		return;
	}

	if( bmpnum==268 ) bCombatMfd = true;
	if( (bmpnum==239) || (bmpnum==238) ) bCombatMfd = false;

	if(nPendingBmps==512) {
		printf("WARNING: Too many bmps to cue!\n");
		return;
	}

	pPendingBmp[nPendingBmps].posx		= posx;
	pPendingBmp[nPendingBmps].posy		= posy;
	pPendingBmp[nPendingBmps].bmp		= bmp;
	pPendingBmp[nPendingBmps].minx		= minx;
	pPendingBmp[nPendingBmps].miny		= miny;
	pPendingBmp[nPendingBmps].maxx		= maxx;
	pPendingBmp[nPendingBmps].maxy		= maxy;
	pPendingBmp[nPendingBmps].bmpnum	= bmpnum;

	nPendingBmps++;
}



void VideoInit (void)
{
	CfgStruct cfg;
	int pPalBase[3] = { 0, 0, 0 };
	int step, i, j, col;

	CfgOpen (&cfg, __CONFIGFILE__);
	CfgFindSection (&cfg, "VIDEO");
	CfgGetKeyVal (&cfg, "RedBase", pPalBase);
	CfgGetKeyVal (&cfg, "GreenBase", pPalBase+1);
	CfgGetKeyVal (&cfg, "BlueBase", pPalBase+2);

	CfgGetKeyVal (&cfg, "startfullscreen", &g_CfgFullscreen);
	CfgGetKeyVal (&cfg, "aspectfix", &g_CfgAspectFix);
	CfgGetKeyVal (&cfg, "fswidth", &g_CfgFSWidth);
	CfgGetKeyVal (&cfg, "fsheight", &g_CfgFSHeight);
	CfgGetKeyVal (&cfg, "fsprefbpp", &g_CfgFSbpp);

	CfgGetKeyVal (&cfg, "winwidth", &g_CfgWinWidth);
	CfgGetKeyVal (&cfg, "winheight", &g_CfgWinHeight);

	CfgGetKeyVal (&cfg, "filterfont", &g_CfgFilterFont);
	CfgGetKeyVal (&cfg, "fontwidth", &g_CfgFontWidth);

	CfgFindSection (&cfg, "CONSOLE");
	for(i=0; i<400; i++) bmpLUT[i].nTextureIndex = -1;
	CfgGetKeyVal( &cfg, "replaceconsole", &g_CfgReplaceConsole );
	nNumPositions = 0;
	if(g_CfgReplaceConsole) {
		char tmp[128];
		for(i=0;i<40;i++) {
			sprintf(tmp,"clickremap%dtop",i);
			CfgGetKeyVal( &cfg, tmp, &clickRemapIcon[i].top );
			sprintf(tmp,"clickremap%dbottom",i);
			CfgGetKeyVal( &cfg, tmp, &clickRemapIcon[i].bottom );
			sprintf(tmp,"clickremap%dleft",i);
			CfgGetKeyVal( &cfg, tmp, &clickRemapIcon[i].left );
			sprintf(tmp,"clickremap%dright",i);
			CfgGetKeyVal( &cfg, tmp, &clickRemapIcon[i].right );
			sprintf(tmp,"clickremap%dx",i);
			CfgGetKeyVal( &cfg, tmp, &clickRemapIcon[i].to_x );
			sprintf(tmp,"clickremap%dy",i);
			CfgGetKeyVal( &cfg, tmp, &clickRemapIcon[i].to_y );
		}
		CfgGetKeyVal( &cfg, "numpositions", &nNumPositions );
		posReplacement = new sBmpPosition[nNumPositions];
		for(i=0;i<nNumPositions;i++) {
			sprintf(tmp,"iconpos%dtop",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacement[i].top );
			sprintf(tmp,"iconpos%dbottom",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacement[i].bottom );
			sprintf(tmp,"iconpos%dleft",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacement[i].left );
			sprintf(tmp,"iconpos%dright",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacement[i].right );
		}
		for(i=0;i<8;i++) {
			sprintf(tmp,"pylonpos%dtop",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacePylon[i].top );
			sprintf(tmp,"pylonpos%dbottom",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacePylon[i].bottom );
			sprintf(tmp,"pylonpos%dleft",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacePylon[i].left );
			sprintf(tmp,"pylonpos%dright",i);
			CfgGetKeyVal( &cfg, tmp, &posReplacePylon[i].right );
		}
		for(i=0;i<400;i++) {
			sprintf(tmp,"bmplut%dtexture",i);
			CfgGetKeyVal( &cfg, tmp, &bmpLUT[i].nTextureIndex );
			sprintf(tmp,"bmplut%dmode",i);
			CfgGetKeyVal( &cfg, tmp, &bmpLUT[i].nMode );
			sprintf(tmp,"bmplut%dsrcpos",i);
			CfgGetKeyVal( &cfg, tmp, &bmpLUT[i].nSrcPosIndex );
			sprintf(tmp,"bmplut%ddstpos",i);
			CfgGetKeyValDef( &cfg, tmp, &bmpLUT[i].nDstPosIndex , bmpLUT[i].nSrcPosIndex);
		}
		CfgGetKeyVal (&cfg, "scanneroffsetx", &g_CfgScannerOffsetX);
		CfgGetKeyVal (&cfg, "scanneroffsety", &g_CfgScannerOffsetY);
	}

	CfgClose (&cfg);

	for (i=0; i<3; i++) 
	{
		col = pPalBase[i] << 8;
		step = ((64<<8) - col) / 63;

		for (j=0; j<64; j++) {
			int tcol = col >> 8;
			if (tcol < 0) tcol = 0;
			pPalTable[i][j] = (UCHAR)tcol;
			col += step;
		}
	}

	VideoInitWorker();
}

void VideoCleanup (void)
{
	if (GLEW_VERSION_2_0)
	{
		// Nick: Below clean up almost definitely isn't necessary, but oh well.
		glDetachShader(sp_tex, vs_tex);
		glDetachShader(sp_tex, fs_tex);
		glDetachShader(sp_flat, vs_tex);
		glDetachShader(sp_flat, fs_flat);
		glDetachShader(sp_char, vs_tex);
		glDetachShader(sp_char, fs_char);
		glDetachShader(sp_true, vs_tex);
		glDetachShader(sp_true, fs_true);

		glDeleteShader(vs_tex);
		glDeleteShader(fs_tex);
		glDeleteShader(sp_tex);
		glDeleteShader(fs_flat);
		glDeleteShader(sp_flat);
		glDeleteShader(fs_char);
		glDeleteShader(sp_char);
		glDeleteShader(fs_true);
		glDeleteShader(sp_true);

		glDeleteTextures(8192, pBitmaps);
		glDeleteTextures(10, pConTextures);
		glDeleteTextures(8192, pTextures);
		glDeleteTextures(1, &pFont);
		glDeleteTextures(1, &pLut);
		glDeleteTextures(1, &pPal);
		glUseProgram(0);
	}

	// Nick
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

extern int SDLMsgHandler (void);

extern "C" void VideoBlitDud (UCHAR *pData, long x, long y, long w, long h, long jump)
{
	SDLMsgHandler();
	return;
}

extern "C" void VideoBlit (UCHAR *pData, long x, long y, long w, long h, long jump)
{
	SDLMsgHandler();							// handle messages here
	if (w != 640) {
		return;									// flip only once per frame
	}
	if( g_bDirtyBuffer ) {
		if( DATA_PlayerState == 0x2c ) {		// clear bottom view in death anims
			SDLDrawBoxToBuf(0,316,640,400,0);
		}
		BlitPendingBmps();
		DrawPendingChars();
		DrawPendingStalks();
		SDL_GL_SwapBuffers ();
		if( g_nDoubleBufferHack > 0 )
		{
			if( DATA_PlayerState == 0x2c ) {	// clear bottom view in death anims
				SDLDrawBoxToBuf(0,316,640,400,0);
			}
			if( g_nDoubleBufferHack == 1) {
				SDLClearUpperView( 819 );		// system menu
			} else if( g_nDoubleBufferHack == 2) {
				DoBlitClip( 0, 0, dblBufHack_bmp, 0, 0, -1, -1, 96 );
			}
			g_nDoubleBufferHack = 0;
			BlitPendingBmps();
			DrawPendingChars();
			DrawPendingStalks();
			SDL_GL_SwapBuffers ();
		}
		g_bDirtyBuffer = false;
	}
	return;
}


extern "C" void VideoMaskedBlit (UCHAR *pData, long x, long y, long w, long h, long jump)
{
	
}

extern "C" void VideoReverseBlit (UCHAR *pData, long x, long y, long w, long h, long jump)
{

}

void ResetVideoSetPalValue ()
{
	if (GLEW_VERSION_2_0)
	{
		for ( long palindex = 0; palindex != pal_count; palindex++ )
		{
			if(pPal==0) {
			//	if( palindex == 127 ) 
				{
					glGenTextures( 1, &pPal );
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_1D, pPal);
					glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pPalTexture);
					glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
					glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glEnable(GL_TEXTURE_1D);
					
				}	
			}
		}
	}
}

extern "C" void VideoSetPalValue (long palindex, UCHAR *pVal)
{
	int pCol[4];
	pal_count = 0;

	pCol[0] = pPalTable[0][pVal[0]] << 2;
	pCol[1] = pPalTable[1][pVal[1]] << 2;
	pCol[2] = pPalTable[2][pVal[2]] << 2;

	if (GLEW_VERSION_2_0)
	{
		pPalTexture[palindex*3+0] = (UCHAR)pCol[0];
		pPalTexture[palindex*3+1] = (UCHAR)pCol[1];
		pPalTexture[palindex*3+2] = (UCHAR)pCol[2];

		if(pPal==0) {
			if( palindex == 127 ) {
				glGenTextures( 1, &pPal );
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_1D, pPal);
				glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, pPalTexture);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glEnable(GL_TEXTURE_1D);
				
			}	
		}
	}
	else
	{
		const long p4 = palindex * 4;

		pPalTexture[p4 + 0] = (UCHAR)pCol[0];
		pPalTexture[p4 + 1] = (UCHAR)pCol[1];
		pPalTexture[p4 + 2] = (UCHAR)pCol[2];
		pPalTexture[p4 + 3] = palindex > 1 ? 255 : 0;
	}

	pal_count++;
}

extern "C" long VideoPointerExclusive (void)
{
	return 0;
}

extern "C" void VideoPointerEnable (void) 
{
	SDL_WM_GrabInput (SDL_GRAB_OFF);
	SDL_ShowCursor (SDL_ENABLE);
}

extern "C" void VideoPointerDisable (void) 
{
	SDL_WM_GrabInput (SDL_GRAB_ON);
	SDL_ShowCursor (SDL_DISABLE);
}


void InputInit ();
void InputCleanup ();
void TimerInit ();
void TimerCleanup ();
void SoundInit ();
void SoundCleanup ();

extern "C" void DirInit ();
extern "C" void DirCleanup ();

extern "C" void SystemInit (void)
{
	int rval;
	rval = SDL_Init (SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (rval != 0) exit (0);

	TimerInit();
	DirInit();
	VideoInit();
	InputInit();
	SoundInit();
}

extern "C" void SystemCleanup (void)
{
	SoundCleanup();
	InputCleanup();
	VideoCleanup();
	DirCleanup();
	TimerCleanup();
	SDL_Quit ();
}

void VideoSwitchMode ()
{
	VideoCleanup ();
	g_CfgFullscreen ^= 1;
	VideoInitWorker ();
	return;
}

extern "C" void SDLBreakPoint (long param)
{
	// debug what you need
}

