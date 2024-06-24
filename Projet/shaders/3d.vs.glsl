#version 420

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
    gl_Position = transformWithProjection * vec4(position, 1);
}
