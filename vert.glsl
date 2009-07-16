uniform float time;
uniform float amplitude;

uniform float high_freq;
uniform float low_freq;

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;

const float PI = 3.1415926535;

void main(void)
{
   vec4 v = vec4(gl_Vertex);

   // Surface Waves
   v.yz = v.yz + gl_Normal.yz*sin(high_freq/1000*time + v.x*2.0);
   v.yz = v.yz + gl_Normal.xz*sin(low_freq/100*time + v.x*0.5);
   v.x = v.x + gl_Normal.x*sin(low_freq/100*time + v.z);

   // Pulse
   v.xyz = v.xyz + amplitude*gl_Normal;

   // Lighting
   normal = normalize(gl_NormalMatrix * gl_Normal);
   lightDir = normalize(vec3(gl_LightSource[0].position));
   halfVector = normalize(gl_LightSource[0].halfVector.xyz);
   diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse * length(v.xyz-gl_Normal.xyz)/4;
   ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
   ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;

   gl_TexCoord[0]  = gl_MultiTexCoord0;
   gl_Position = gl_ModelViewProjectionMatrix * v;
}
