uniform float time;
uniform float amplitude;

void main(void)
{
   vec4 v = vec4(gl_Vertex);
   vec3 n = vec3(gl_Normal);
   
   //v.xyz = v.xyz + n*amplitude;
   v.xyz = v.xyz + n.xyz*sin(v.y+time)+n.xyz*0.5;
   
   gl_FrontColor = gl_Color;
   gl_Position = gl_ModelViewProjectionMatrix * v;
}
