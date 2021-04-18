class Plane
{
public:
	Plane( float A, float B, float C, float D ) : N( make_float3( A, B, C ) ), d( D ) {}
	Plane( float3 ABC, float D ) : N( ABC ), d( D ) {}
	float3 N;
	float d;
};

class Sphere
{
public:
	Sphere() {}
	Sphere( float x, float y, float z, float radius ) : pos( make_float3( x, y, z ) ), r( radius ) {}
	float3 pos;
	float r;
};

class Material
{
public:
	float refl, refr;
	bool emissive;
	float3 diffuse;
};

class Scene
{
public:
	Scene();
	float3 RandomPointOnLight();
	void IntersectPlane( Ray& ray );
	void IntersectSphere( const int idx, const Sphere& sphere, Ray& ray );
	float3 SampleSkydome( const float3& D );
	int Intersect( Ray& ray );
	bool IsOccluded( Ray& ray );
	Material GetMaterial( int objIdx, float3& I );
// private:
	Sphere plane1, plane2, plane3, plane4, plane5, plane6;
	Sphere sphere[6];
	float* skybox = 0;
	inline static float LIGHTSIZE = 1.0f;
	inline static float3 LIGHTPOS = make_float3( 2.7f, 2.4f, -1.0f );
	inline static float LIGHTAREA = LIGHTSIZE * LIGHTSIZE;
	inline static float LIGHTSCALE = 0.1f * (25.0f / LIGHTAREA);
	inline static float3 lightColor = make_float3( 8.5f * LIGHTSCALE, 8.5f * LIGHTSCALE, 7.0f * LIGHTSCALE );
};