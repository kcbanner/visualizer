uniform float time;

void main(void)
{
   vec4 v = vec4(gl_Vertex);
   vec3 n = vec3(gl_Normal);
   
   v.xyz = v.xyz + n*sin(time);
   
   gl_FrontColor = gl_Color;
   gl_Position = gl_ModelViewProjectionMatrix * v;
}
