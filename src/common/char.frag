/* Fragment shader */

uniform sampler2D tex;
uniform vec4	  color;
	
void main()
{
	float	shade	= texture2D( tex, gl_TexCoord[0].st ).r;
	gl_FragColor	= color * shade;
}

