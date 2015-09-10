#ifdef GL_ES
// Set default precision to medium
precision highp int;
precision highp float;
#endif

uniform sampler2D texture;

#if 1   // SET TO 1 for per Vertex-EM, 0 for per-Frag-EM

varying vec3 Reflection;

void main()
{
    vec2 texCoord;
    vec3 r = normalize(Reflection);
    float m = 2.0 * sqrt( r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0) );
    texCoord.s = r.x/m + 0.5;
    texCoord.t = r.y/m + 0.5;

    gl_FragColor = texture2D( texture, texCoord);


}


#else

varying vec3 Normal;
varying vec3 Position;

void main()
{
    vec2 texCoord;
    vec3 r = reflect(normalize(Position),normalize(Normal));
    float m = 2.0 * sqrt( r.x*r.x + r.y*r.y + (r.z+1.0)*(r.z+1.0) );
    texCoord.s = r.x/m + 0.5;
    texCoord.t = r.y/m + 0.5;

    gl_FragColor = texture2D( texture, texCoord);


}


#endif



