
const char VERTEXSHADERCODE[] = "\
	void main(void)\n\
	{\n\
		gl_TexCoord[0] = gl_MultiTexCoord0;\n\
		gl_TexCoord[1] = gl_MultiTexCoord1;\n\
		gl_Position = ftransform();\n\
	}\n\n";

const char CHARFRAGMENTSHADERCODE[] = "\
	uniform sampler2D tex;\n\
	uniform vec4	  color;\n\
	void main()\n\
	{\n\
		float	shade	= texture2D( tex, gl_TexCoord[0].st ).r;\n\
		gl_FragColor	= color * shade;\n\
	}\n\n";

const char FLATFRAGMENTSHADERCODE[] = "\
	uniform vec4	  color;\n\
	void main()\n\
	{\n\
		gl_FragColor	= color;\n\
	}\n\n";

const char TEXFRAGMENTSHADERCODE[] = "\
	uniform sampler2D tex;\n\
	uniform sampler1D lut;\n\
	uniform int		  tra;\n\
	void main()\n\
	{\n\
		float	shade	= texture2D( tex, gl_TexCoord[0].st ).r + 0.001;\n\
		if( (shade<0.003) && (tra>0) ) discard;\n\
		gl_FragColor	= texture1D( lut, shade );\n\
	}\n\n";

const char TRUEFRAGMENTSHADERCODE[] = "\
	uniform sampler2D tex;\n\
	void main()\n\
	{\n\
		gl_FragColor	=  texture2D( tex, gl_TexCoord[0].st );\n\
	}\n\n";

