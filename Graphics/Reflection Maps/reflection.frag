/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

// These definitions agree with the ObjectIds enum in scene.h
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     wallId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;

// Computes unit vector N (for lighting calc)
	in vec3 normalVec;
 
// Computes unit vectors L and V (for lighting calc)
	in vec3  lightVec, eyeVec;

// Values describing the surface
	uniform vec3 diffuse;     // Kd
	uniform vec3 specular;  // Ks
	uniform float shininess; // alpha exponent
// Values describing the scene's light
	uniform vec3 Light;       // Ii
	uniform vec3 Ambient;  // Ia 

in vec2 texCoord;
in vec3 worldPos;

in vec4 shadowCoord;

uniform sampler2D shadowMap;

uniform int objectId;

void main()
{

//if (teapotId==objectId) discard;
    vec3 Kd = diffuse;   
    
    if (objectId==groundId || objectId==seaId) {
        ivec2 uv = ivec2(floor(200.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd *= 0.9; }
    
	float lightDepth,pixelDepth;
	vec2 shadowIndex=shadowCoord.xy/shadowCoord.w;	
	
	vec3 N = normalize(normalVec);
	
	vec3 L = normalize(lightVec);
	vec3 V = normalize(eyeVec);	
	
	
	vec3 H = normalize(L+V);
	float LN = max(dot(L,N),0.0);
	float HN = max(dot(H,N),0.0);
	float LH = max(dot(H,L),0.0);
	float VN = max(dot(V,N),0.0);
	
	vec3 Ks= specular;
	
	vec3 F = Ks + (vec3(1)-Ks)*pow((1-LH),5.0);
	float G = 1/(pow(LH,2.0));

	// The lighting calculation ...
	float D = (shininess+2)*(pow(HN,shininess))/(2.0*3.14);
	
	vec3 BRDF=(Kd/3.14)+((F*G*D)/4.0);
	
	//Shadow calculation
	if(shadowCoord.w>0)
	{
		if(shadowIndex.x>=0 && shadowIndex.x<=1 && shadowIndex.y>=0 && shadowIndex.y<=1)
		{
			lightDepth=texture(shadowMap,shadowIndex).w;
			pixelDepth=shadowCoord.w/100;
			
			//if true then in shadow,apply shadow values
			if(pixelDepth>lightDepth+0.001)//offset added here to remove shadow acne
			{
				gl_FragColor.xyz = Ambient * Kd;
			}
			else // apply normal lighting
			{
				gl_FragColor.xyz = (Ambient*Kd)+(Light*LN*BRDF);
			}
			
		}
		else
		{
			gl_FragColor.xyz = (Ambient*Kd)+(Light*LN*BRDF);
		}
	}
	else
	{
		gl_FragColor.xyz = (Ambient*Kd)+(Light*LN*BRDF);
	}
	
	//gl_FragColor.xyz=vec3(1,0,0)
}
