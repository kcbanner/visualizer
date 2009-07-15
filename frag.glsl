varying vec4 diffuse,ambient;
varying vec3 normal,lightDir,halfVector;
   
void main(void)
{
   vec3 n, halfV;
   float intensity, hv_intensity;

   vec4 color = ambient;
   
   n = normalize(normal);
   intensity = max(dot(n,lightDir),0.0);

   if (intensity > 0.0)
   {
      color += diffuse*intensity;
      halfV = normalize(halfVector);
      hv_intensity = max(dot(n,halfV),0.0);
      color += gl_FrontMaterial.specular*gl_LightSource[0].specular*pow(hv_intensity, gl_FrontMaterial.shininess);
   }

   gl_FragColor = color;
}
