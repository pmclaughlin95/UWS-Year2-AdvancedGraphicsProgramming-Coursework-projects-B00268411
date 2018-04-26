// phong-tex.vert
// Vertex shader for use with a Phong or other reflection model fragment shader
// Calculates and passes on V, L, N vectors for use in fragment shader, phong2.frag
#version 330

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 lightPosition;

uniform vec4 spotLightPosition;
//uniform mat3 normalmatrix;

in  vec3 in_Position;
in  vec3 in_Normal;
out vec3 ex_N;
out vec3 ex_V;
out vec3 ex_L;

out vec4 Vposition;
out vec4 slPosition;

in vec2 in_TexCoord;
out vec2 ex_TexCoord;
out float ex_D;


void main(void) {

	// vertex into eye coordinates
	vec4 vertexPosition = modelview * vec4(in_Position,1.0);



	


	// Find V - in eye coordinates, eye is at (0,0,0)
	ex_V = normalize(-vertexPosition).xyz;


	// surface normal in eye coordinates
	// taking the rotation part of the modelview matrix to generate the normal matrix
	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	ex_N = normalize(normalmatrix * in_Normal);


	ex_L = normalize(lightPosition.xyz - vertexPosition.xyz);			// L - to light source from vertex

	vec3 ex_DL =normalize(spotLightPosition.xyz - vertexPosition.xyz);	



	ex_TexCoord = in_TexCoord;

	ex_D = distance(vertexPosition,lightPosition);						//Distance used for attenuation in the frag shader

	//spotlight
	Vposition = vertexPosition;
	slPosition = spotLightPosition;



    gl_Position = projection * vertexPosition;

	

}