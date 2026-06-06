#version 450 // Use GLSL 4.5

// array of triangle that fills screen
vec2 positions[3] = vec2[3]( //
    vec2(3.0, -1.0),    // p1
    vec2(-1.0, -1.0),     // p2
    vec2(-1.0, 3.0)     // p3
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
