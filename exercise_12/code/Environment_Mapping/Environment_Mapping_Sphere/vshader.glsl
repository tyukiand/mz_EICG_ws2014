#ifdef GL_ES
// Set default precision to medium
precision highp int;
precision highp float;
#endif

attribute vec3 a_position;
attribute vec3 a_normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat3 normal_matrix;


#if 1   // SET TO 1 for per Vertex-EM, 0 for per-Frag-EM

varying vec3 Reflection;

void main() {
    vec4 world_pos = model_matrix *vec4(a_position,1.0);
    vec4 eye_pos = view_matrix *world_pos;
    vec4 clip_pos =projection_matrix *eye_pos;

    vec3 Normal = normalize(normal_matrix * a_normal);
    Reflection = reflect(normalize(eye_pos.xyz), Normal);

    gl_Position = clip_pos;
}

#else

varying vec3 Normal;
varying vec3 Position;

void main() {
    vec4 world_pos = model_matrix *vec4(a_position,1.0);
    vec4 eye_pos = view_matrix *world_pos;
    vec4 clip_pos =projection_matrix *eye_pos;

    Normal = normalize(normal_matrix * a_normal);
    Position = eye_pos.xyz;

    gl_Position = clip_pos;
}


#endif
