uniform float time;
uniform float amplitude;

uniform float low_freq_max;
uniform float high_freq;
uniform float low_freq;

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;

const float PI = 3.1415926535;

void main(void)
{
    vec4 v = vec4(gl_Vertex);
    vec3 origin = vec3(0.0, 0.0, -40.0);

    // Surface Waves
    //v.yz = v.yz + gl_Normal.yz*sin(high_freq/1000.0*time + v.x*2.0);
    //v.yz = v.yz + gl_Normal.xz*sin(low_freq/100.0*time + v.x*0.5);
    //v.x = v.x + 2.0*gl_Normal.x*sin(low_freq/100.0*time + v.x);
    //v.yz = v.yz + gl_Normal.yz*sin(smoothstep(0.0, low_freq_max, low_freq)*time + v.x*2.0);
    v.yz = v.yz + gl_Normal.yz*sin(1.5*smoothstep(0.0, 1000.0, low_freq)*time + v.x);
    v.x = v.x + gl_Normal.x*sin(0.8*smoothstep(6000.0, 7500.0, high_freq)*time + v.x*4.0);

    // Pulse
    v.xyz = v.xyz + amplitude*gl_Normal;

    // Lighting
    normal = normalize(gl_NormalMatrix * gl_Normal);
    lightDir = normalize(vec3(gl_LightSource[0].position));
    halfVector = normalize(gl_LightSource[0].halfVector.xyz);

    diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
    ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient * smoothstep(0.0, 20.0, amplitude*length(v.xyz-gl_Normal.xyz)/5.0);
    ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;

    diffuse = diffuse + (sin(low_freq/200*time + v.x/8.0)+1)*0.1;


    gl_TexCoord[0]  = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * v;
}
