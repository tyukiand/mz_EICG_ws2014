#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

attribute vec3 a_position;
attribute vec3 a_normal;

uniform mat4 uMVMat;
uniform mat4 uPMat;
uniform mat3 uNMat;

varying vec3 Normal;
varying vec3 Position;

void main() {
    //übergebe Variablen an Fragment shader
    Normal = normalize(uNMat * a_normal);
    Position =vec3( uMVMat * vec4(a_position,1.0) );
    gl_Position = uPMat * uMVMat * vec4(a_position,1.0);
}
