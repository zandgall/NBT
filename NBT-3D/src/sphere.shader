#shader vertex
#version 400 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 anormal;

varying vec2 TexCoord;
uniform mat4 view, transform, perspective;

void main() {
	TexCoord = uv;
	gl_Position = perspective * view * transform * vec4(position, 1.0);
	gl_Position = gl_Position.xyww;
}

#shader fragment
#version 400 core

varying vec2 TexCoord;

out vec4 glColor;
uniform sampler2D image;

void main() {
	glColor = texture2D(image, TexCoord);
	glColor.w = 1;
}