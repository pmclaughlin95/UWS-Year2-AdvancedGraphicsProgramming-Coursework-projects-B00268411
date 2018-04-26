// Phong fragment shader phong-tex.frag matched with phong-tex.vert
#version 330

// Some drivers require the following
precision highp float;

struct lightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct spotLightStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 position;

};

uniform	vec4 spotLightPosition;				// Position in eye coords.
uniform vec3 spotlightDirection;			// Normalized direction of the spotlight
//uniform float spotlightExponent;			// Angular attenuation exponent
//uniform float spotlightCutoff;			// Cutoff angle (between 0 and 90)

struct materialStruct
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform lightStruct light;

uniform spotLightStruct spotLight;

in vec4 Vposition;
in vec4 slPosition;

uniform materialStruct material;
uniform sampler2D textureUnit0;

uniform float attConst;
uniform float attLinear;
uniform float attQuadratic;


in vec3 ex_N;
in vec3 ex_V;
in vec3 ex_L;

in vec2 ex_TexCoord;
in float ex_D;
layout(location = 0) out vec4 out_Color;
 
void main(void) {
    
	//Spotlight calculations
	vec4 totalLight;

	//Finds the vector from the spotlight to the vertex
	vec3 L = normalize(slPosition.xyz - Vposition.xyz);
	float distToLight = length(L);
	L = normalize(L);

	//The angle between spotlight direction and vertex
	float cosDir = dot(-L, spotlightDirection);

	float spotEffect = smoothstep(0.978 , 0.985 ,cosDir);

	totalLight = vec4(spotEffect) * (material.ambient + material.diffuse + material.specular);

	totalLight = totalLight * max(dot(normalize(ex_N),normalize(-spotlightDirection)),0);



	// Ambient intensity
	vec4 ambientI = light.ambient * material.ambient;

	// Diffuse intensity
	vec4 diffuseI = light.diffuse * material.diffuse;
	diffuseI = diffuseI * max(dot(normalize(ex_N),normalize(ex_L)),0);

	// Specular intensity
	// Calculate R - reflection of light
	vec3 R = normalize(reflect(normalize(-ex_L),normalize(ex_N)));

	vec4 specularI = light.specular * material.specular;
	specularI = specularI * pow(max(dot(R,ex_V),0), material.shininess);

	float attenuation=1.0f/(attConst + attLinear * ex_D + attQuadratic * ex_D*ex_D);
	vec4 tmp_Color = (diffuseI + specularI)*texture(textureUnit0, ex_TexCoord);
	//Attenuation does not affect transparency
	vec4 litColour = vec4(tmp_Color.rgb *attenuation, tmp_Color.a);
	vec4 amb=min(ambientI,vec4(1.0f));
		
	out_Color=min(litColour+amb*texture(textureUnit0, ex_TexCoord),vec4(1.0f));//Here attenuation does not affectambient
	
	
	



}