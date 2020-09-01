#include "std-pipeline.h"
#include <GL/glew.h>

void Use_TEX_Shader( int flag )
{    
    // RGB Stage!!!
    //
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_RGB_ARB, 
		GL_REPLACE
	);

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_RGB_ARB, 
		//GL_CONSTANT_ARB
        GL_TEXTURE
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_RGB_ARB, 
		GL_SRC_COLOR
	);

    if( flag != 0 )
    {        
        glEnable(GL_ALPHA_TEST);
    }
    else 
    {
        glDisable(GL_ALPHA_TEST);
    }

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_ALPHA_ARB, 
		GL_REPLACE
	);
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_ALPHA_ARB, 
		GL_TEXTURE
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_ALPHA_ARB, 
		GL_SRC_ALPHA
	);
}

void Use_FLAT_Shader() // complete, but need glColor command!
{
    glDisable(GL_ALPHA_TEST);
    // RGB Stage!!!
    // inColor
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_RGB_ARB, 
		GL_REPLACE
	);

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_RGB_ARB, 
		GL_PRIMARY_COLOR_ARB
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_RGB_ARB, 
		GL_SRC_COLOR
	);
    // Alpha Stage
    // inColor
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_ALPHA_ARB, 
		GL_REPLACE
	);
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_ALPHA_ARB, 
		GL_PRIMARY_COLOR_ARB
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_ALPHA_ARB, 
		GL_SRC_ALPHA
	);
}

void Use_TRUE_Shader() // complete, but you forget alpha stage!
{
    glDisable(GL_ALPHA_TEST);
    // RGB Stage!!!
    // Tex(RGB)
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_RGB_ARB, 
		GL_REPLACE
	);

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_RGB_ARB, 
		GL_TEXTURE
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_RGB_ARB, 
		GL_SRC_COLOR
	);

    // Alpha Stage !!!

    // Tex(Alpha)
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_ALPHA_ARB, 
		GL_REPLACE
	);

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_ALPHA_ARB, 
		GL_TEXTURE
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_ALPHA_ARB, 
		GL_SRC_ALPHA
	);
}

void User_CHAR_Shader()
{
    glDisable(GL_ALPHA_TEST);
    // RGB Stage !!!
    // Tex(Alpha) * inColor

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_RGB_ARB, 
		GL_MODULATE
	);

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_RGB_ARB, 
		GL_TEXTURE
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_RGB_ARB, 
		GL_SRC_COLOR
	);

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE1_RGB_ARB, 
		GL_PRIMARY_COLOR_ARB
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND1_RGB_ARB, 
		GL_SRC_COLOR
	);

    // Alpha Stage !!!

    // Tex(Alpha)
    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_COMBINE_ALPHA_ARB, 
		GL_REPLACE
	);

    glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_SOURCE0_ALPHA_ARB, 
		GL_TEXTURE
	);
	glTexEnvi
	(
		GL_TEXTURE_ENV, 
		GL_OPERAND0_ALPHA_ARB, 
		GL_SRC_ALPHA
	);
}