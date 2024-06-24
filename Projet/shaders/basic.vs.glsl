#version 120

uniform float time;

attribute vec2 position;
attribute vec3 color;
attribute vec2 texCoords;

varying vec3 fragColor;
varying vec2 fragTexCoords;

void main(void) {
    fragColor = color;
    fragTexCoords = texCoords;
    vec2 movement = vec2(0.1 * sin(3. * time), 0.1 * cos(1.5 * time));
    gl_Position = vec4(position + movement, 0, 1);
}
