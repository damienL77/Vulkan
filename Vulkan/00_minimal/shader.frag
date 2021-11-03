#version 450

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec4 normal  = texture(normalSampler, fragTexCoord) * 2.0f;
	normal .r -= 1.0f;
	normal .g -= 1.0f;
	normal .b -= 1.0f;
	
	normal  =  normalize(normal.rgb);
	
    outColor = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0f);// * texture(normalSampler, fragTexCoord);
}