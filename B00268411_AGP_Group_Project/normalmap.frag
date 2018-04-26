// Phong fragment shader phongEnvMap.frag matched with phong-tex.vert
#version 330

// Some drivers require the following
precision highp float;

struct lightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct materialStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform lightStruct light;
uniform materialStruct material;

//uniform samplerCube cubeMap;
uniform sampler2D texMap;
uniform sampler2D normalMap;

uniform float attConst;
uniform float attLinear;
uniform float attQuadratic;

in vec3 ex_N;
in vec3 ex_V;
in vec3 ex_L;
in vec2 ex_TexCoord;
in float ex_D;


in vec3 eyeTan;
in vec3 lightTan;

layout(location = 0) out vec4 out_Color;

void main(void) {

// TO DO: Check I havent messed up lighting calculations!!!
	vec3 N = normalize( (texture( normalMap, ex_TexCoord ).rgb-0.5) * 2 );

	// Ambient intensity
	vec4 ambientI = light.ambient * material.ambient;

	// Diffuse intensity
	vec4 diffuseI = light.diffuse * material.diffuse;
	diffuseI = diffuseI * max(dot(N,normalize(lightTan)),0);

	// Specular intensity
	// Calculate R - reflection of light
	vec3 R = normalize(reflect(-lightTan,N));

	vec4 specularI = light.specular * material.specular;
	specularI = specularI * pow(max(dot(R,-eyeTan),0), material.shininess); // + or - eyeTan??

	// Fragment colour
	float attenuation = attConst + attLinear * ex_D + attQuadratic * ex_D*ex_D;
	vec4 litColor = vec4( (ambientI + (diffuseI + specularI)/attenuation).rgb, 1.0f );

	out_Color = texture(texMap, ex_TexCoord) * litColor;
	//out_Color = litColor;
}