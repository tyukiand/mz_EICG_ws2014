#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

struct LightProperties {
    vec3 ambient; //light’s contribution toambient light
    vec3 diffuse; //diffuse color of light
    vec3 specular; //diffuse color of light
    vec3 position; //location of light 
    vec3 spotAngle;
};

struct MaterialProperties {
    vec3 emission; //light produced by the material
    vec3 ambient; //what part of ambient light is reflected
    vec3 diffuse; //what part of diffuse light is scattered
    vec3 specular; //what part of specular light is scattered
    float shininess; //exponent for sharpening specular reflection
};

uniform LightProperties Light;
uniform MaterialProperties Material;
uniform vec4 uColor;
uniform vec3 attCoe;

uniform vec3 spotDirection;
uniform bool spotOn;

varying vec3 Normal;
varying vec3 Position;

void main()
{
    vec3 EyeDirection = normalize(-Position);
    

          //compute per-fragment direction, halfVector, and attenuation
        vec3 lightDirection =Light.position - vec3(Position);
        float lightDistance =length(lightDirection);
        lightDirection =lightDirection /lightDistance;
        vec3  halfVector =normalize(lightDirection +EyeDirection);

        float att = 1.0 / (attCoe[2]*lightDistance*lightDistance+ attCoe[1]*lightDistance+attCoe[0]);

        //Blinn Lighting
        float diffuse = max(0.0, dot(Normal, lightDirection));
        float specular = max(0.0, dot(Normal, halfVector));

        if(diffuse ==0.0)
            specular =0.0;
        else
            specular =pow(specular, Material.shininess);

        vec3 scatteredLight =Light.ambient * Material.ambient + Light.diffuse * Material.diffuse *diffuse ;
        vec3 reflectedLight =Light.specular * Material.specular *specular;

        //spot

        float spotfactor=1.0;
        if (spotOn){
            float lightAngle=degrees(acos(dot(-lightDirection, -lightDirection)));
            if (lightAngle > Light.spotAngle/2. || dot(spotDirection,Normal)>0)
                spotfactor=0.;
        }
        vec3 rgb=min(spotfactor*Material.emission + uColor.rgb *scatteredLight * att +reflectedLight * att, vec3(1.0));

        gl_FragColor =vec4(rgb, uColor.a);

}
