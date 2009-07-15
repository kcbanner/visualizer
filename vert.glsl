uniform float time;
uniform float amplitude;
uniform float speed;

varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;

void main(void)
{
   vec4 v = vec4(gl_Vertex);

   // Surface Waves    
   v.xyz = v.xyz + amplitude*gl_Normal*sin(v.x + speed*time);
   v.xyz = v.xyz + amplitude*gl_Normal*sin(v.y + speed*time);
   v.xyz = v.xyz + amplitude*gl_Normal*sin(v.z + speed*time);
 
   // Pulse
   v.xyz = v.xyz + amplitude*gl_Normal*sin(speed*time);

   // Lighting     
   normal = normalize(gl_NormalMatrix * gl_Normal);
   lightDir = normalize(vec3(gl_LightSource[0].position));
   halfVector = normalize(gl_LightSource[0].halfVector.xyz);
   diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
   ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
   ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;

   gl_Position = gl_ModelViewProjectionMatrix * v;
}
