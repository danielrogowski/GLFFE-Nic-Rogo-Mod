// utility functions for sdlvideo.cpp
#include <SDL_image.h>

SDL_Surface *LoadBMP(char *filename)
{
	Uint8 *rowhi, *rowlo;
	Uint8 *tmpbuf, tmpch;
	SDL_Surface *image, *alpha;
	int i, j;

	// SDL_image
	// IMG_Load
	// Nick

	image = IMG_Load(filename); // Nick was SDL_LoadBMP
	if ( image == NULL ) {
		fprintf(stderr, "Unable to load %s: %s\n", filename, SDL_GetError());
		return(NULL);
	}

	// Nick - Convert to ensure correct
	SDL_PixelFormat RGBFormat = {0};
	RGBFormat.palette = 0; RGBFormat.colorkey = 0; RGBFormat.alpha = 255;
	RGBFormat.BitsPerPixel = 24; RGBFormat.BytesPerPixel = 3;
	RGBFormat.Rloss = 16; RGBFormat.Gloss = 8; RGBFormat.Bloss = 0; RGBFormat.Aloss = 8;	
	RGBFormat.Rmask = 0x00FF0000; RGBFormat.Gmask = 0x0000FF00; RGBFormat.Bmask = 0x000000FF;
	/*
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	RGBFormat.Rmask = 0xFF000000; RGBFormat.Rshift = 0; RGBFormat.Rloss = 0;
	RGBFormat.Gmask = 0x00FF0000; RGBFormat.Gshift = 8; RGBFormat.Gloss = 0;
	RGBFormat.Bmask = 0x0000FF00; RGBFormat.Bshift = 16; RGBFormat.Bloss = 0;
	RGBFormat.Amask = 0x000000FF; RGBFormat.Ashift = 24; RGBFormat.Aloss = 0;
	#else
	RGBFormat.Rmask = 0x000000FF; RGBFormat.Rshift = 24; RGBFormat.Rloss = 16;
	RGBFormat.Gmask = 0x0000FF00; RGBFormat.Gshift = 16; RGBFormat.Gloss = 8;
	RGBFormat.Bmask = 0x00FF0000; RGBFormat.Bshift = 8; RGBFormat.Bloss = 0;
	RGBFormat.Amask = 0xFF000000; RGBFormat.Ashift = 0; RGBFormat.Aloss = 8;
	#endif
*/
	SDL_Surface *conv = SDL_ConvertSurface(image, &RGBFormat, SDL_SWSURFACE);
	SDL_FreeSurface(image);
	image = conv;

	if ( image->format->BitsPerPixel != 24 ) {
		fprintf(stderr, "not a 24 bitmap\n", filename, SDL_GetError());
		return(NULL);
	}

	/* GL surfaces are upsidedown and RGB, not BGR :-) */
	tmpbuf = (Uint8 *)malloc(image->pitch);
	if ( tmpbuf == NULL ) {
		fprintf(stderr, "Out of memory\n");
		return(NULL);
	}
	rowhi = (Uint8 *)image->pixels;
	rowlo = rowhi + (image->h * image->pitch) - image->pitch;
	for ( i=0; i<image->h/2; ++i ) {
		for ( j=0; j<image->w; ++j ) {
			tmpch = rowhi[j*3];
			rowhi[j*3] = rowhi[j*3+2];
			rowhi[j*3+2] = tmpch;
			tmpch = rowlo[j*3];
			rowlo[j*3] = rowlo[j*3+2];
			rowlo[j*3+2] = tmpch;
		}
		//memcpy(tmpbuf, rowhi, image->pitch);
		//memcpy(rowhi, rowlo, image->pitch);
		//memcpy(rowlo, tmpbuf, image->pitch);
		rowhi += image->pitch;
		rowlo -= image->pitch;
	}
	free(tmpbuf);

	char alphafname[128];
	char tmp[128];
	sprintf(tmp,"%s",filename);
	tmp[strlen(tmp)-4]='\0';
	sprintf(alphafname,"%s_alpha.png",tmp);		// Nick was bmp

	alpha = IMG_Load(alphafname);   // Nick was SDL_LoadBMP
	if ( alpha == NULL ) {
		fprintf(stderr, "%s alpha mask not loaded: %s\n", filename, SDL_GetError());
		return(image);
	}

	if ( alpha->format->BitsPerPixel != 8 ) {
		fprintf(stderr, "not a 8 alpha bitmap\n", alphafname, SDL_GetError());
		return(image);
	}

	if ( (alpha->w!=image->w) || (alpha->h!=image->h) ) {
		fprintf(stderr, "%s image and alpha bitmap size mismatch, ignoring alpha\n", alphafname);
		return(image);
	}

	SDL_Surface *surface;
    Uint32 rmask, gmask, bmask, amask;

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
	#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
	#endif

    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, image->w, image->h, 32, rmask, gmask, bmask, amask);
	if( surface == NULL ) {
		fprintf(stderr, "could not create 32 bit surface for %s, ignoring alpha\n", filename);
		return(image);
	}

	Uint8* rowres = (Uint8 *)surface->pixels;
	Uint8* rowimg = (Uint8 *)image->pixels;
	Uint8* rowalp = (Uint8 *)alpha->pixels;
	for ( i=0; i<image->h; ++i ) {
		for ( j=0; j<image->w; ++j ) {
			memcpy( &(rowres[4*j]), &(rowimg[3*j]), 3*sizeof(Uint8));
			rowres[4*j+3] = rowalp[j];
		}
		rowres += surface->pitch;
		rowimg += image->pitch;
		rowalp += alpha->pitch;
	}

	SDL_FreeSurface(image);		// Nick instead of SDL_free
	return(surface);
}

void printLog(GLuint obj)
{
	int infologLength = 0;
	char infoLog[1024];
 
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);
 
	if (infologLength > 0)
		printf("%s\n", infoLog);
}

char *file2string(const char *path)
{
	FILE *fd;
	long len,
		 r;
	char *str;
 
	if (!(fd = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return NULL;
	}
 
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
 
	printf("File '%s' is %ld long\n", path, len);
 
	fseek(fd, 0, SEEK_SET);
 
	if (!(str = (char*)malloc(len * sizeof(char))))
	{
		fprintf(stderr, "Can't malloc space for '%s'\n", path);
		return NULL;
	}
 
	r = fread(str, sizeof(char), len, fd);
 
	str[r - 1] = '\0'; /* Shader sources have to term with null */
 
	fclose(fd);
 
	return str;
}
