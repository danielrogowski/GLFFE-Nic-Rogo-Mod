/* Fragment shader */

uniform sampler2D tex;
uniform sampler1D lut;
uniform int		  tra;
	
void main()
{
	float	shade	= texture2D( tex, gl_TexCoord[0].st ).r + 0.001;
	if( (shade<0.003) && (tra>0) ) discard;
	//vec4	color	= vec4(shade,shade,shade,1.)*16.0;
	vec4	color	= texture1D( lut, shade );
	gl_FragColor	= color;
}

