#version 120

uniform sampler2D sampler_;

varying vec3 fragColor;
varying vec2 fragTexCoords;

void main(void) {
    gl_FragColor = texture2D(sampler_, vec2(fragTexCoords.x, -fragTexCoords.y)) * vec4(fragColor, 1);
}
