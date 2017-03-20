// Block Party - @P_Malin
//#define WALK

//#define COLOR_PASTEL
//#define COLOR_VIVID
#define COLOR_SUNSET

#ifdef COLOR_PASTEL

vec3 gSunColor = vec3(1.0, 0.9, 0.1) * 10.0;  

vec3 gSkyTop =  vec3( 0.1, 0.2, 0.8 ) * 4.0;
vec3 gSkyBottom = vec3( 0.5, 0.8, 1.0 ) * 5.0;

float gFogDensity = 0.01;

vec3 gFloorColor = vec3(0.8, 1.0, 0.8);
vec3 gCubeColor = vec3(1.0, 0.8, 0.8);
float gExposure = 1.0;

float gCubeColorRandom = 0.4;

#endif

#ifdef COLOR_VIVID

vec3 gSunColor = vec3(1.0, 0.9, 0.1) * 10.0;  

vec3 gSkyTop =  vec3( 0.1, 0.2, 0.8 ) * 0.5;
vec3 gSkyBottom = vec3( 0.5, 0.8, 1.0 ) * 1.5;

float gFogDensity = 0.05;

vec3 gFloorColor = vec3(0.4, 1.0, 0.4);
vec3 gCubeColor = vec3(1.0, 0.1, 1.0);
float gExposure = 1.0;

float gCubeColorRandom = 0.9;

#endif

#ifdef COLOR_SUNSET
vec3 gSunColor = vec3(1.0, 0.2, 0.1) * 10.0;  

vec3 gSkyTop = vec3( 1.0, 0.8, 0.5 ) * 0.5;
vec3 gSkyBottom =  vec3( 0.8, 0.2, 0.1 ) * 1.5;

float gFogDensity = 0.05;

vec3 gFloorColor = vec3(1.0, 0.5, 0.5);
vec3 gCubeColor = vec3(1.0, 0.5, 1.0);
float gExposure = 1.0;

float gCubeColorRandom = 0.5;

#endif

#define MOVE_OUTWARDS

float fAOAmount = 0.8;
float gFloorHeight = -1.0;
float g_cameraFar = 1000.0;

#define PI radians( 180.0 )


vec3 GetSunDir()
{
  	return normalize( vec3( 1.0, 0.3, -0.5 ) );
}


void GetQuadInfo( const float vertexIndex, out vec2 quadVertId, out float quadId )
{
    float twoTriVertexIndex = mod( vertexIndex, 6.0 );
    float triVertexIndex = mod( vertexIndex, 3.0 );
  
    if 		( twoTriVertexIndex < 0.5 ) quadVertId = vec2( 0.0, 0.0 );
    else if	( twoTriVertexIndex < 1.5 )	quadVertId = vec2( 1.0, 0.0 );
    else if ( twoTriVertexIndex < 2.5 )	quadVertId = vec2( 0.0, 1.0 );
    else if ( twoTriVertexIndex < 3.5 )	quadVertId = vec2( 1.0, 0.0 );
    else if ( twoTriVertexIndex < 4.5 )	quadVertId = vec2( 1.0, 1.0 );
    else 								quadVertId = vec2( 0.0, 1.0 );

    quadId = floor( vertexIndex / 6.0 );
}


void GetQuadTileInfo( const vec2 quadVertId, const float quadId, const vec2 vDim, out vec2 vQuadTileIndex, out vec2 vQuadUV )
{
    vQuadTileIndex.x = floor( mod( quadId, vDim.x ) );
    vQuadTileIndex.y = floor( quadId / vDim.x );

  	vQuadUV.x = floor(quadVertId.x + vQuadTileIndex.x);
    vQuadUV.y = floor(quadVertId.y + vQuadTileIndex.y);

    vQuadUV = vQuadUV * (1.0 / vDim);
}


void GetQuadTileInfo( const float vertexIndex, const vec2 vDim, out vec2 vQuadTileIndex, out vec2 vQuadUV )
{
  	vec2 quadVertId;
  	float quadId;
	GetQuadInfo( vertexIndex, quadVertId, quadId );  
  	GetQuadTileInfo( quadVertId, quadId, vDim, vQuadTileIndex, vQuadUV );   
}


void GetMatrixFromZY( const vec3 vZ, const vec3 vY, out mat3 m )
{
   vec3 vX = normalize( cross( vY, vZ ) );
   vec3 vOrthoY = normalize( cross( vZ, vX ) );
   m[0] = vX;
   m[1] = vOrthoY;
   m[2] = vZ;
}


void GetMatrixFromZ( vec3 vZAxis, out mat3 m )
{
  	vec3 vZ = normalize(vZAxis);
   	vec3 vY = vec3( 0.0, 1.0, 0.0 );
  	if ( abs(vZ.y) > 0.99 )
    {
       vY = vec3( 1.0, 0.0, 0.0 );
    }
  	GetMatrixFromZY( vZ, vY, m );
}


struct SceneVertex
{
  	vec3 vWorldPos;
  	vec3 vColor;
  	float fAlpha;
};


float GetCosSunRadius()
{
  return 0.01;
}


float GetSunIntensity()
{  	
  	return 0.001;
}


vec3 GetSkyColor( vec3 vViewDir )
{
	return mix( gSkyBottom, gSkyTop, max( 0.0, vViewDir.y ) );
}


vec3 GetBackdropColor( vec3 vViewDir )
{
  	float VdotL = dot( normalize(vViewDir), GetSunDir() );
  
  	VdotL = clamp( VdotL, 0.0, 1.0 );
  
  	float fShade = 0.0;

  	fShade = acos( VdotL ) * (1.0 / PI);
  
  	float fCosSunRadius = GetCosSunRadius();
  
  	fShade = max( 0.0, (fShade - fCosSunRadius) / (1.0 - fCosSunRadius) );    
  
  	fShade = GetSunIntensity() / max( 0.0001, pow(fShade, 1.5) );
  
  	vec3 vColor = vec3(0.0);
    vColor += GetSkyColor( vViewDir );
      
    vColor += vec3( fShade * gSunColor );
    return vColor;
}


#define g_backdropSegments 			32.0
#define g_backdropSlices 			16.0
#define g_backdropQuads 			( g_backdropSegments * g_backdropSlices )
#define g_backdropVertexCount 		( g_backdropQuads * 6.0 )


void GenerateBackdropVertex( const float vertexIndex, const vec3 vCameraPos, out SceneVertex outSceneVertex )
{
    vec2 vBackdropDim = vec2( g_backdropSegments, g_backdropSlices );
    
  	vec2 vQuadTileIndex;
    vec2 vUV;  
  	GetQuadTileInfo( vertexIndex, vBackdropDim, vQuadTileIndex, vUV );

    float fSlicePos = 0.0;
  
  	float fSunMeshPinch = 5.0;
  
  	if (vUV.y > 0.0)
    {
      	float t = pow( vUV.y, fSunMeshPinch );
  		float fCosSunRadius = GetCosSunRadius();
      	fSlicePos = fCosSunRadius + t * (1.0- fCosSunRadius);
    }
    
  	vec3 vSpherePos;
  	float fElevation = fSlicePos * PI;
  	vSpherePos.z = cos( fElevation );

  	float fHeading = vUV.x * PI * 2.0;
  	float fSliceRadius = sqrt( 1.0 - vSpherePos.z * vSpherePos.z );
  	vSpherePos.x = sin( fHeading ) * fSliceRadius;
  	vSpherePos.y = cos( fHeading ) * fSliceRadius;
  
	mat3 m;
  
  	GetMatrixFromZ( GetSunDir(), m );
    
  	vec3 vLocalSpherePos = m * vSpherePos;

  	float fBackdropDistance = g_cameraFar; 
  	vec3 vWorldSpherePos = vLocalSpherePos * fBackdropDistance;

  	vWorldSpherePos += vCameraPos;
  
    outSceneVertex.vWorldPos = vWorldSpherePos;
      
  	outSceneVertex.vColor = GetBackdropColor( vLocalSpherePos );

  	outSceneVertex.fAlpha = 1.0;
}  


#define g_cubeFaces					6.0
#define g_cubeVerticesPerFace		( g_cubeFaces * 2.0 * 3.0 )
#define g_cubeVertexCount 			( g_cubeVerticesPerFace * g_cubeFaces )

//                   6          7
//                    +----------+
//                   /|         /|
//                2 / |       3/ |
//                 +----------+  |
//                 |  |       |  |
//      Y   Z      | 4|       | 5|
//                 |  +-------|--+
//      ^ /        | /        | /
//      |/        0|/        1|/
//      +--> X     +----------+
  
vec3 GetCubeVertex( float fVertexIndex )
{
	vec3 fResult = vec3( 1.0 );
  
  	float f = fVertexIndex / 8.0;
  	if ( fract( f * 4.0 ) < 0.5 )
    {
    	fResult.x = -fResult.x;
    }
  
  	if ( fract( f * 2.0 ) < 0.5 )
    {
    	fResult.y = -fResult.y;
    }

  	if ( fract( f ) < 0.5 )
    {
    	fResult.z = -fResult.z;
    }
  
  	return fResult;
}


void GetCubeVertex( const float vertexIndex, const mat3 mRot, const vec3 vTrans, out vec3 vWorldPos, out vec3 vWorldNormal )
{
  	float fFaceIndex = floor( vertexIndex / g_cubeFaces );

  	vec3 v0, v1, v2, v3;
  
  	if ( fFaceIndex < 0.5 )
    {
      	v0 = GetCubeVertex( 0.0 );
      	v1 = GetCubeVertex( 2.0 );
      	v2 = GetCubeVertex( 3.0 );
      	v3 = GetCubeVertex( 1.0 );
    }
  	else if ( fFaceIndex < 1.5 )
    {
      	v0 = GetCubeVertex( 5.0 );
      	v1 = GetCubeVertex( 7.0 );
      	v2 = GetCubeVertex( 6.0 );
      	v3 = GetCubeVertex( 4.0 );
    }
  	else if ( fFaceIndex < 2.5 )
    {
      	v0 = GetCubeVertex( 1.0 );
      	v1 = GetCubeVertex( 3.0 );
      	v2 = GetCubeVertex( 7.0 );
      	v3 = GetCubeVertex( 5.0 );
    }
  	else if ( fFaceIndex < 3.5 )
    {
      	v0 = GetCubeVertex( 4.0 );
      	v1 = GetCubeVertex( 6.0 );
      	v2 = GetCubeVertex( 2.0 );
      	v3 = GetCubeVertex( 0.0 );
    }
  	else if ( fFaceIndex < 4.5 )
    {
      	v0 = GetCubeVertex( 2.0 );
      	v1 = GetCubeVertex( 6.0 );
      	v2 = GetCubeVertex( 7.0 );
      	v3 = GetCubeVertex( 3.0 );
    }
  	else
    {
      	v0 = GetCubeVertex( 1.0 );
      	v1 = GetCubeVertex( 5.0 );
      	v2 = GetCubeVertex( 4.0 );
      	v3 = GetCubeVertex( 0.0 );
    }
  
  	v0 = v0 * mRot + vTrans;
  	v1 = v1 * mRot + vTrans;
  	v2 = v2 * mRot + vTrans;
  	v3 = v3 * mRot + vTrans;
  
  	float fFaceVertexIndex = mod( vertexIndex, 6.0 );
  
  	if ( fFaceVertexIndex < 0.5 )
    {
	  	vWorldPos = v0;
    }
  	else if ( fFaceVertexIndex < 1.5 )
    {
	  	vWorldPos = v1;
    }
  	else if ( fFaceVertexIndex < 2.5 )
    {
	  	vWorldPos = v2;
    }
  	else if ( fFaceVertexIndex < 3.5 )
    {
	  	vWorldPos = v0;
    }
  	else if ( fFaceVertexIndex < 4.5 )
    {
	  	vWorldPos = v2;
    }
  	else
    {
	  	vWorldPos = v3;
    }
  
  	vWorldNormal = normalize( cross( v1 - v0, v2 - v0 ) );  
}


vec3 GetSunLighting( const vec3 vNormal )
{
  	vec3 vLight = -GetSunDir();
  
  	float NdotL = max( 0.0, dot( vNormal, -vLight ) );
 	  
  	return gSunColor * NdotL;
}


vec3 GetSunSpec( const vec3 vPos, const vec3 vNormal, const vec3 vCameraPos )
{
  	vec3 vLight = -GetSunDir();

  	vec3 vView = normalize( vCameraPos - vPos );
  
  	vec3 vH = normalize( vView - vLight );
  
  	float NdotH = max( 0.0, dot( vNormal, vH ) );
  	float NdotL = max( 0.0, dot( vNormal, -vLight ) );
 
  	float f = mix( 0.01, 1.0, pow( 1.0 - NdotL, 5.0 ) );
  
  	return gSunColor * pow( NdotH, 20.0 ) * NdotL * f * 4.0;
}


vec3 GetSkyLighting( const vec3 vNormal )
{
  	vec3 vSkyLight = normalize( vec3( -1.0, -2.0, -0.5 ) );
  
  	float fSkyBlend = vNormal.y * 0.5 + 0.5;
 
  	return mix( gSkyBottom, gSkyTop, fSkyBlend );
}


void GenerateCubeVertex( const float vertexIndex, const mat3 mRot, const vec3 vTrans, const vec3 vCubeCol, const float fStage, const vec3 vCameraPos, out SceneVertex outSceneVertex )
{  
  	vec3 vNormal;
  
	GetCubeVertex( vertexIndex, mRot, vTrans, outSceneVertex.vWorldPos, vNormal );
  
  	outSceneVertex.vColor = vec3( 0.0 );
  
  	outSceneVertex.fAlpha = 1.0;  
  
  	float h = outSceneVertex.vWorldPos.y - gFloorHeight;
  	if ( fStage < 0.5 )
    {
	    outSceneVertex.vColor += GetSkyLighting( vNormal );
      	outSceneVertex.vColor *= mix( 1.0, fAOAmount, clamp( h, 0.0, 1.0 ) );
      
  		outSceneVertex.vColor += GetSunLighting( vNormal );
      
      	outSceneVertex.vColor *= vCubeCol;      
      
      	outSceneVertex.vColor += GetSunSpec( outSceneVertex.vWorldPos, vNormal, vCameraPos );
    }
  	else
    {
      	vec3 vSunDir = GetSunDir();
      	outSceneVertex.vWorldPos.x += h * -vSunDir.x;
      	outSceneVertex.vWorldPos.z += h * -vSunDir.z;
      	outSceneVertex.vWorldPos.y = gFloorHeight;
      
        outSceneVertex.vColor += GetSkyLighting( normalize(vec3(1.0, 1.0, 0.0)) );

      	outSceneVertex.vColor *= gFloorColor;
    }  
}


void ApplyFog( const vec3 vCameraPos, inout SceneVertex sceneVertex )
{
  	vec3 vViewOffset = sceneVertex.vWorldPos - vCameraPos;
  	float fDist = length( vViewOffset );
  
  	vec3 vViewDir = normalize( vViewOffset );
  
  	float fFogBlend = exp2( fDist * -gFogDensity );
  
  	vec3 vFogColor = GetBackdropColor( vViewDir );
    
  	sceneVertex.vColor = mix( vFogColor, sceneVertex.vColor, fFogBlend );
}


#define g_floorTileX 16.0
#define g_floorTileY 16.0
#define g_floorTileCount ( g_floorTileX * g_floorTileY )
#define g_floorVertexCount ( g_floorTileCount * 6.0 )

void GenerateFloorVertex( const float vertexIndex, out SceneVertex outSceneVertex )
{
  	vec2 vDim = vec2( g_floorTileX, g_floorTileY );
  	vec2 vQuadTileIndex;
  	vec2 vQuadUV;
  
	GetQuadTileInfo( vertexIndex, vDim, vQuadTileIndex, vQuadUV );  

  	outSceneVertex.vWorldPos.xz = (vQuadUV * 2.0 - 1.0) * 1000.0;
  	outSceneVertex.vWorldPos.y = gFloorHeight - 0.01;
  	outSceneVertex.fAlpha = 1.0;
  	outSceneVertex.vColor = vec3(0.0);

  	vec3 vNormal = vec3( 0.0, 1.0, 0.0 );
  	outSceneVertex.vColor += GetSkyLighting( vNormal );
  	outSceneVertex.vColor += GetSunLighting( vNormal );
  
  	outSceneVertex.vColor *= gFloorColor;
}


// hash function from https://www.shadertoy.com/view/4djSRW
float hash(float p)
{
	vec2 p2 = fract(vec2(p * 5.3983, p * 5.4427));
    p2 += dot(p2.yx, p2.xy + vec2(21.5351, 14.3137));
	return fract(p2.x * p2.y * 95.4337);
}


mat3 RotMatrixY( float fAngle )
{
    float s = sin( fAngle );
    float c = cos( fAngle );
  	
    return mat3( c, 0.0, s, 
                         0.0, 1.0, 0.0,
                         -s, 0.0, c );
  
}


mat3 RotMatrixZ( float fAngle )
{
    float s = sin( fAngle );
    float c = cos( fAngle );
  	
    return mat3( c, s, 0.0, 
                 -s, c, 0.0,
                 0.0, 0.0, 1.0 );
  
}

void GetCubePosition( float fCubeId, out mat3 mCubeRot, out vec3 vCubeOrigin, out vec3 vCubeCol )
{  
  	float fSeed = fCubeId;
  	float fPositionBase = fCubeId;
#ifdef MOVE_OUTWARDS
  	fSeed -= floor(time);
  	fPositionBase += mod(time, 1.0);
#endif  
  	float fSize = hash(fSeed * 1.234);
  	fSize = fSize * fSize;

  	vCubeOrigin = vec3( 0.0, 0.0, 0.0 );

	
    float r = sqrt(fPositionBase) * 1.987;
  	float fTheta = r * (0.3 * PI * 2.0);//log2(r) * 10.0;
    vCubeOrigin.x = sin(fTheta) * r;
    vCubeOrigin.z = cos(fTheta) * r;
  
    float fScale = fSize * 0.5 + 0.5;
  
#ifdef MOVE_OUTWARDS
  	fScale *= clamp(r , 0.0, 1.0);
#endif  
  
  	float roll = 0.0;
#ifdef WALK
  	roll = r * PI * 2.0 * fScale;
#endif
  
  	mCubeRot = RotMatrixZ(roll);
  	mCubeRot *= RotMatrixY(fTheta);
    mCubeRot *= fScale;
  	
  	float fMinY = 10.0;
  	
  	float f = 0.0;
  	for( int i=0; i<8; i++)
    {
      	vec3 vert = GetCubeVertex(f) * mCubeRot;
	  	fMinY = min( fMinY, vert.y );
      	f+= 1.0;
    }
  
  	vCubeOrigin.y = gFloorHeight;
  	vCubeOrigin.y += -fMinY;
  
  	float jump = 0.0;

  	float o = 10.0 / 240.0;
  	float v = 0.0;
    float a = 0.011;
  
  	for(int i=0; i<10; i++)
    {
        float off = 0.1;
        float spread = 0.3;      
        float speed  = 0.002;
      
        float snd = texture2D(sound, vec2(off + spread * fSize, r * speed + o)).a;

      	snd = snd * snd * (1.0 + 0.5 * (1.0 - fSize));
      
      	o = o - (1.0 / 240.0);
      
      	v = v - 0.1;
      	a = a * 1.5;
      	v += snd * a;
      	jump = jump + v;
      
      	if( jump < 0.0)
        {
          	v = 0.0;
          	jump = 0.0;
        }
    }

  	vCubeOrigin.y += jump * 6.0;
  
  	vec3 vRandCol;
  	vRandCol.x = hash( fSeed );
  	vRandCol.y = hash( fSeed * 1.234);
  	vRandCol.z = hash( fSeed * 2.345);
  
  	vCubeCol = mix( gCubeColor, vRandCol, gCubeColorRandom );
}


void main()
{ 
  	SceneVertex sceneVertex;
  
  	vec2 vMouse = mouse;
  
  	float fov = 1.5;
  
  	float fMouseX = (vMouse.x * 0.5 + 0.5);
  	fMouseX = fMouseX * fMouseX;
  
  	float animTime = time;
  
  	float orbitAngle = animTime * 0.3456 + 4.0;
  	float elevation = -0.02 + (sin(animTime * 0.223 - PI * 0.5) * 0.5 + 0.5) * 0.5;
  	float fOrbitDistance = 5.0 + (cos(animTime * 0.2345) * 0.5 + 0.5 ) * 40.0;
  
  	vec3 vCameraTarget = vec3( 0.0, 1.0, 0.0 );
  	vec3 vCameraPos = vCameraTarget + vec3( sin(orbitAngle) * cos(elevation), sin(elevation), cos(orbitAngle) * cos(elevation) ) * fOrbitDistance;
  	vec3 vCameraUp = vec3( 0.1, 1.0, 0.0 );
  
  	if( false )
    {
      vCameraPos = vec3( 10.0, 8.0, 30.0 );
      vCameraTarget = vec3( 0.0, 0.0, 0.0 );
      vCameraUp = vec3( 0.0, 1.0, 0.0);
    }
  
  	vec3 vCameraForwards = normalize(vCameraTarget - vCameraPos);
  
  	mat3 mCamera;
    GetMatrixFromZY( vCameraForwards, normalize(vCameraUp), mCamera );
  
  	float vertexIndex = vertexId;
  
  
  	if ( vertexIndex >= 0.0 && vertexIndex < g_backdropVertexCount )
    {
    	GenerateBackdropVertex( vertexIndex, vCameraPos, sceneVertex );
    }
  	vertexIndex -= g_backdropVertexCount;
  
  	if ( vertexIndex >= 0.0 && vertexIndex < g_floorVertexCount )
    {
    	GenerateFloorVertex( vertexIndex, sceneVertex );
	  	ApplyFog( vCameraPos, sceneVertex );
    }
  	vertexIndex -= g_floorVertexCount;
  
  	if ( vertexIndex >= 0.0 )
    {
        float fCubeIndex = floor( vertexIndex / g_cubeVertexCount );
        float fCubeStage = mod( fCubeIndex, 2.0 );
        float fCubeId = floor(fCubeIndex / 2.0);
        float fCubeVertex = mod( vertexIndex, g_cubeVertexCount );

        {
          	mat3 mCube;
          	vec3 vCubeOrigin;
          	vec3 vCubeCol;
          	
	        GetCubePosition( fCubeId, mCube, vCubeOrigin, vCubeCol );

            GenerateCubeVertex( fCubeVertex, mCube, vCubeOrigin, vCubeCol, fCubeStage, vCameraPos, sceneVertex );
		  	ApplyFog( vCameraPos, sceneVertex );

            fCubeId += 1.0;
        }
    }


    // Fianl output position
	vec3 vViewPos = sceneVertex.vWorldPos;
    vViewPos -= vCameraPos;
  	vViewPos =  vViewPos * mCamera;
  	
  	vec2 vFov = vec2( 1.0, resolution.x / resolution.y ) * fov;
  	vec2 vScreenPos = vViewPos.xy * vFov;

	gl_Position = vec4( vScreenPos.xy, -1.0, vViewPos.z );
    
  	// Final output color
  	float fExposure = min( gExposure, time * 0.1 );
  	vec3 vFinalColor = sqrt( vec3(1.0) - exp2( sceneVertex.vColor * -fExposure ) );
  
  	v_color = vec4(vFinalColor * sceneVertex.fAlpha, sceneVertex.fAlpha);    
}