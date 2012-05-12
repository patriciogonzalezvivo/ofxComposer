uniform sampler2DRect tex0;
uniform sampler2DRect tex1;

void main()
{
	vec4 color = texture2DRect(tex0, gl_TexCoord[0].st);
	gl_FragColor = color;
}