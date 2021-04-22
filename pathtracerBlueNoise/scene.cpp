#include "precomp.h"

Scene::Scene()
{
	plane1 = Sphere( 0.0f, -4999.0f, 0.0f, 4998.5f * 4998.5f );		// bottom plane
	plane2 = Sphere( 0.0f, 0.0f, -5000.0f, 4993.0f * 4993.0f );		// back plane
	plane3 = Sphere( 0.0f, 5002.0f, 0.0f, 4998.5f * 4998.5f );		// ceiling plane
	plane4 = Sphere( 0.0f, 0.0f, 5000.0f, 4993.0f * 4993.0f );		// front plane
	plane5 = Sphere( -5000.0f, 0.0f, 0.0f, 4993.0f * 4993.0f );		// left plane
	plane6 = Sphere( 5000.0f, 0.0f, 0.0f, 4993.0f * 4993.0f );		// right plane
	for (int i = 0; i < 3; i++)
	{
		sphere[i] = Sphere( -0.8f + i * 0.8f, 0, -2, 0.3f * 0.3f );
		sphere[i + 3] = Sphere( -0.8f + i * 0.8f, -0.8f, -2, 0.5f * 0.5f );
	}
	// load skybox
	skybox = new float[2500 * 1250 * 3];
	FILE* f = fopen( "sky_15.raw", "rb" );
	fread( skybox, 2500 * 1250 * 3, 1, f );
	fclose( f );
}

float3 Scene::RandomPointOnLight()
{
	return LIGHTPOS + make_float3( (RandomFloat() - 0.5f) * LIGHTSIZE, 0, (RandomFloat() - 0.5f) * LIGHTSIZE );
}

void Scene::IntersectPlane( Ray& ray )
{
	float3 NL = make_float3( 0, -1, 0 );
	float d = -dot( LIGHTPOS, NL );
	float t = -(dot( ray.O, NL ) + d) / (dot( ray.D, NL ));
	float3 I = ray.O + t * ray.D;
	if ((I.x >= (LIGHTPOS.x - 0.5f * LIGHTSIZE)) && (I.x <= (LIGHTPOS.x + 0.5f * LIGHTSIZE)) &&
		(I.z >= (LIGHTPOS.z - 0.5f * LIGHTSIZE)) && (I.z <= (LIGHTPOS.z + 0.5f * LIGHTSIZE)) && (t < ray.t) && (t > 0))
	{
		ray.t = t;
		ray.objIdx = 8;
		ray.N = make_float3( 0, -1, 0 );
	}
}

void Scene::IntersectSphere( const int idx, const Sphere& sphere, Ray& ray )
{
	float3 L = sphere.pos - ray.O;
	float tca = dot( L, ray.D );
	if (tca < 0) return;
	float d2 = dot( L, L ) - tca * tca;
	if (d2 > sphere.r) return;
	float thc = sqrtf( sphere.r - d2 );
	float t0 = tca - thc;
	float t1 = tca + thc;
	if (t0 > 0)
	{
		if (t0 > ray.t) return;
		ray.N = normalize( ray.O + t0 * ray.D - sphere.pos );
		ray.objIdx = idx;
		ray.t = t0;
	}
	else
	{
		if ((t1 > ray.t) || (t1 < 0)) return;
		ray.N = normalize( sphere.pos - (ray.O + t1 * ray.D) );
		ray.objIdx = idx;
		ray.t = t1;
	}
}

float3 Scene::SampleSkydome( const float3& D )
{
	int u = (int)(2500.0f * 0.5f * (1.0f + atan2f( D.x, -D.z ) * INVPI));
	int v = (int)(1250.0f * acosf( D.y ) * INVPI);
	int idx = u + v * 2500;

	if (idx == -3e8)
	{
		return make_float3(0);
	}

	return make_float3(skybox[idx * 3 + 0], skybox[idx * 3 + 1], skybox[idx * 3 + 2]);

}

int Scene::Intersect( Ray& ray )
{
	IntersectSphere( 0, plane1, ray );
	IntersectSphere( 1, plane2, ray );
	for (int i = 0; i < 6; i++) IntersectSphere( i + 2, sphere[i], ray );
	IntersectPlane( ray );
	if (false /* indoor scene */)
	{
		IntersectSphere( 9, plane3, ray );
		IntersectSphere( 10, plane4, ray );
		IntersectSphere( 11, plane5, ray );
		IntersectSphere( 12, plane6, ray );
	}
	return ray.objIdx;
}

Material Scene::GetMaterial( int objIdx, float3& I )
{
	Material mat;
	if (objIdx == 0 || objIdx == 9)																							// checkered floor
	{
		// procedural checkerboard pattern for floor plane
		mat.refl = mat.refr = 0;
		mat.emissive = false;
		int tx = ((int)(I.x * 3.0f + 1000) + (int)(I.z * 3.0f + 1000)) & 1;
		mat.diffuse = make_float3( 1 ) * ((tx == 1) ? 1.0f : 0.6f);
	}
	if (objIdx == 1 || objIdx > 8) { mat.refl = mat.refr = 0; mat.emissive = false; mat.diffuse = make_float3( 1 ); }		// back wall
	if (objIdx == 2) { mat.refl = 0.8f; mat.refr = 0; mat.emissive = false; mat.diffuse = make_float3( 1, 0.2f, 0.2f ); }	// red sphere
	if (objIdx == 3) { mat.refl = 0; mat.refr = 1; mat.emissive = false; mat.diffuse = make_float3( 0.9f, 1.0f, 0.9f ); }	// glass sphere
	if (objIdx == 4) { mat.refl = 0.8f; mat.refr = 0; mat.emissive = false; mat.diffuse = make_float3( 0.2f, 0.2f, 1 ); }	// blue sphere
	if (objIdx > 4 && objIdx < 8) { mat.refl = mat.refr = 0; mat.emissive = false; mat.diffuse = make_float3( 1 ); }		// sphere bases
	if (objIdx == 8) { mat.refl = mat.refr = 0; mat.emissive = true; mat.diffuse = lightColor; }							// light
	if (objIdx == 11) { mat.refl = mat.refr = 0; mat.emissive = false; mat.diffuse = make_float3( 0.3f, 1, 0.3f ); }		// right wall
	if (objIdx == 12) { mat.refl = mat.refr = 0; mat.emissive = false; mat.diffuse = make_float3( 1, 0.3f, 0.3f ); }		// left wall
	return mat;
}

bool Scene::IsOccluded( Ray& ray )
{
	Intersect( ray );
	return ray.objIdx > -1;
}