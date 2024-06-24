#version 420

uniform float time;
//layout(binding=0) uniform matrices {
//    mat4 transformNormal;
//    mat4 transformWithProjection;
//};
uniform mat4 transformNormal;
uniform mat4 transformWithProjection;

in vec3 position;
in vec3 normal;
in vec2 texCoords;

out vec3 fragNormal;
out vec2 fragTexCoords;

void main(void) {
    fragNormal = mat3(transformNormal) * normal;
    fragTexCoords = texCoords;
    vec3 movement = vec3(0.1 * sin(time * 10), 0.05 * sin(time * 50), 0.1 * cos(time * 10));
    gl_Position = transformWithProjection * vec4(position + movement, 1);
}
