// textured.frag
#version 330

// Some drivers require the following
precision highp float;

uniform sampler2D textureUnit0;

in vec2 ex_TexCoord;
layout(location = 0) out vec4 out_Color;
 
void main(void) {
    
	// Fragment colour
	out_Color = texture(textureUnit0, ex_TexCoord);
	vec4 texel = texture(textureUnit0, ex_TexCoord);
     if(texel.a < 0.5)                 //checks if any texel has an alpha value below 0.5
    discard;                          //if so, remove the texel
    out_Color = texel; 
}