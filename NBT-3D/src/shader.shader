#shader vertex
#version 400 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 anormal;

varying vec2 TexCoord;
varying vec3 pos;
varying vec3 norm;
uniform mat4 view, perspective, transform;

void main() {
	TexCoord = uv;
	norm = normalize(anormal); 
	gl_Position = perspective * view * transform * vec4(position, 1.0);
	pos = gl_Position.xyz;
}

#shader fragment
#version 400 core

struct MTL {
	vec3 Ka;
	vec3 Kd;
	sampler2D mapKd;
	bool mapKdExists;
	vec3 Ks;
	vec3 Ke;
	float Ns;
	float d;
};

varying vec2 TexCoord;
varying vec3 pos;
varying vec3 norm;

out vec4 glColor;
uniform sampler2D hdri;
uniform sampler2D hdrd;
uniform MTL material;

const vec3 viewDir = vec3(0, 0, 1);
uniform vec3 nViewDir;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
	vec2 uv = vec2(atan(v.x, -v.z), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	uv.y = 1 - uv.y;
	return uv;
}

void main() {
	vec3 lightDir = normalize(vec3(-10, 10, 0) - pos);
	//vec3 norm = normalize(normal);
	vec3 reflectDir = reflect(-lightDir, norm);
	vec3 reflected = texture2D(hdri, SampleSphericalMap(reflect(nViewDir, norm))).xyz;
	vec3 reflectedd = texture2D(hdrd, SampleSphericalMap(reflect(nViewDir, norm))).xyz;
	float mult = max(dot(norm, lightDir) + length(reflectedd), -0.5)/2.0 + 0.5;

	vec3 diff = (material.mapKdExists ? texture2D(material.mapKd, TexCoord).xyz : material.Kd) * mult;
	vec3 spec = (material.Ks) * pow(max(dot(viewDir, reflectDir), 0.0), material.Ns) + (reflected*pow(length(reflected) / 32.f, material.Ns/450.f));
	glColor.xyz = diff + spec + material.Ke * 2.0;
	//glColor.xyz = nViewDir;
	glColor.w = material.d;
}