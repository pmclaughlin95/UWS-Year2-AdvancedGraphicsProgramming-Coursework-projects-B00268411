#version 330 core

uniform sampler2D image;

out vec4 FragmentColor;

layout(location = 0) out vec3 color;

void main(void)
{
	FragmentColor = texture2D( image, vec2(gl_FragCoord)/1024.0 );
}
