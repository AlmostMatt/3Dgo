uniform sampler2D color_texture;
uniform sampler2D normal_texture;

// New bumpmapping
varying vec3 lightVec;
varying vec3 halfVec;
varying vec3 eyeVec;

void main (void)
{
	// lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
	//vec3 normal = texture2D (normal_texture, gl_TexCoord[0].st).rgb;
	vec3 normal = (2.0 * texture2D (normal_texture, gl_TexCoord[0].st).rgb) - 1.0;
	normal = normalize (normal);
    //normal.z = - normal.z;
	
    lightVec = normalize(lightVec);
	// compute diffuse lighting
	float lamberFactor= max (dot (lightVec, normal), 0.0) ;
	vec4 diffuseMaterial = 0.0;
	vec4 diffuseLight  = 0.0;
	
	// compute specular lighting
	vec4 specularMaterial ;
	vec4 specularLight ;
	float shininess ;
  
	// compute ambient
	vec4 ambientLight = gl_LightSource[0].ambient;	
	
	//gl_FragColor = vec4(1.0);
	
    gl_FragColor = ambientLight;
    //gl_FragColor.rgb = 0.5 * (normal - vec3(1.0));//ambientLight;
    //gl_FragColor.rgb = (0.5 * normal) + 0.5;
    
	if (lamberFactor > 0.0)
	{
		diffuseMaterial = texture2D (color_texture, gl_TexCoord[0].st);
		diffuseLight  = gl_LightSource[0].diffuse;
		
		// In doom3, specular value comes from a texture 
		specularMaterial =  vec4(1.0)  ;
		specularLight = gl_LightSource[0].specular;
		shininess = pow (max (dot (halfVec, normal), 0.0), 2.0)  ;
		gl_FragColor +=	diffuseMaterial * diffuseLight * lamberFactor ;
		//gl_FragColor +=	specularMaterial * specularLight * shininess ;				
	}
}
