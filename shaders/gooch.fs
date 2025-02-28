#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

//Constants, passed in directly from C++
uniform vec3 viewPos;
uniform vec3 objectColour;
uniform float rough;
uniform vec3 lightPositions[2];
uniform vec3 lightIntensities[2];
uniform sampler2D texture_diffuse1; //This is used for texture mapping, you can ignore it in A#2

void main()
{
	//For the second part of assignment#2, I believe you will be implementing Gooch shading, if so it goes here!
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}