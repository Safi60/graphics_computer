#version 420

struct Light {
    vec3 direction;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

struct Material {
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
};

uniform float time;
uniform sampler2D sampler_;
uniform Light light;
uniform Material material;
uniform float shininess;
uniform vec3 view;

in vec3 fragNormal;
in vec2 fragTexCoords;

out vec4 color;

vec3 ambient() {
    return light.ambientColor * material.ambientColor;
}

vec3 diffuse(vec3 n, vec3 l) {
    return max(0.0, dot(n, l)) * light.diffuseColor * material.diffuseColor;
}

vec3 specular(vec3 n, vec3 l) {
    if (dot(n, l) <= 0)
        return vec3(0);
    vec3 h = normalize(l + view);
    return max(0.0, pow(dot(n, h), shininess)) * light.specularColor * material.specularColor;
}

void main(void) {
    vec3 n = normalize(fragNormal);
    vec3 l = -light.direction;
    float blink = 0.5 + 0.5 * sin(time * 7);
    color = texture2D(sampler_, vec2(fragTexCoords.x, -fragTexCoords.y)) * vec4(ambient() + diffuse(n, l) + specular(n, l), 0) * vec4(blink, blink, blink, 1);
}
