#version 140
out vec4 fragColor;
uniform vec4 geometryColor;
void main() {
    fragColor = geometryColor;
    fragColor.a = 0.4;
}
