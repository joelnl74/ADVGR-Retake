#include "precomp.h"
#include "Utils.h"

// some helper functions

static float3 Reflect( const float3& A, const float3& B ) { return A - 2.0f * B * dot( A, B ); }

static void Refraction( bool inside, const float3& D, const float3& N, float3& R )
{
	const float n1 = inside ? 1.2f : 1, n2 = inside ? 1 : 1.2f;
	const float eta = n1 / n2, cosi = -dot( D, N );
	const float cost2 = 1.0f - eta * eta * (1 - cosi * cosi);
	R = Reflect( D, N );
	if (cost2 <= 0) return;
	const float a = n1 - n2, b = n1 + n2, R0 = (a * a) / (b * b), c = 1 - cosi;
	const float Fr = R0 + (1 - R0) * (c * c * c * c * c);
	if (RandomFloat() > Fr) R = eta * D + ((eta * cosi - sqrtf( abs( cost2 ) )) * N);
}

static float CalculateLightPDF( const float3& D, float t, const float3& NL )
{
	const float NLdotL = max( 0.0f, -dot( D, NL ) );
	const float solidAngle = (NLdotL * Scene::LIGHTAREA) / (t * t);
	return 1.0f / solidAngle;
}

static float3 DiffuseReflectionCosWeighted()
{
	const float r0 = RandomFloat(), r1 = RandomFloat();
	const float term1 = 2 * PI * r0, term2 = sqrtf( 1 - r1 );
	return make_float3( cosf( term1 ) * term2, sinf( term1 ) * term2, sqrtf( r1 ) );
}

static float3 Local2World( const float3& V, const float3& N )
{
	const float3 tmp = (abs( N.x ) > 0.99f) ? make_float3( 0, 1, 0 ) : make_float3( 1, 0, 0 );
	const float3 B = normalize( cross( N, tmp ) ), T = cross( B, N );
	return V.x * T + V.y * B + V.z * N;
}

static float3 SampleLambert( const float3& diffuse, const float3& N, float3& R, float& pdf )
{
	R = Local2World( DiffuseReflectionCosWeighted(), N ), pdf = 1 / (2 * PI);
	return diffuse * INVPI;
}

// -----------------------------------------------------------
// Path Tracer
// -----------------------------------------------------------
float3 Game::SampleNEEShaded( Ray& ray )
{
	// random walk
	float3 T = make_float3( 1 ), E = make_float3( 0 );
	bool lastSpecular = true;

	while(true)
	{
		// find nearest ray/scene intersection
		if (scene.Intersect( ray ) == -1) 
		{ 
			E += T * scene.SampleSkydome( ray.D ); 
			
			break; 
		}

		float3 I = ray.O + ray.t * ray.D;
		Material material = scene.GetMaterial( ray.objIdx, I );
		if (material.emissive) 
		{ 
			if (lastSpecular) 
				E += T * material.diffuse; break; 
		}
		
		// handle specular, dielectric, diffuse
		float r0 = RandomFloat();
		float3 R = make_float3( 0 );

		if (r0 < material.refr)
		{
			// dielectric: refract or reflect
			bool hitInside = dot( ray.N, R ) < 0;
			Refraction( ray.inside, ray.D, ray.N, R );
			ray = Ray( I + R * EPSILON, R, 1e34, hitInside ? false : true );
			T *= material.diffuse;
			lastSpecular = true;
		}
		else if (r0 < (material.refl + material.refr))
		{
			// pure specular reflection
			R = Reflect( ray.D, ray.N );
			ray = Ray( I + R * EPSILON, R );
			T *= material.diffuse;
			lastSpecular = true;
		}
		else
		{
			// next event estimation
			float3 L = scene.RandomPointOnLight() - I;
			float dist = length(L);
			L = normalize(L);

			if (m_NEE)
			{
				float NdotL = dot(ray.N, L);
				if (NdotL > 0)
				{
					Ray r(I + L * EPSILON, L, dist - 2 * EPSILON);
					if (!scene.IsOccluded(r))
					{
						float lightPDF = CalculateLightPDF(L, dist, make_float3(0, -1, 0));
						float3 sampledBRDF = material.diffuse * INVPI;
						E += T * (NdotL / lightPDF) * sampledBRDF * scene.lightColor;
					}
				}
			}
			else
			{
				float cos_o = dot(-L, make_float3(0, -1, 0));
				float cos_i = dot(L, ray.N);

				if ((cos_o <= 0) || (cos_i <= 0))
				{
					T += make_float3(0);
				}

				Ray r(I + L * EPSILON, L, dist - 2 * EPSILON);

				if (!scene.IsOccluded(r))
				{
					float3 sampledBRDF = material.diffuse * INVPI;
					float solidAngle = (cos_o * Scene::LIGHTAREA) / (dist * dist);

					E += T * sampledBRDF * 1 * Scene::lightColor * solidAngle * cos_i;
				}
			}

			float p = PSurvice(material.diffuse);
			float rng = Utils::RandomNumber();

			if (p < rng) {
				break;
			}

			T *= 1 / p;

			// sample random direction on hemisphere
			float hemiPDF = 0;
			float3 BRDF = SampleLambert(material.diffuse, ray.N, R, hemiPDF);
			
			if (BRDF.x == 0 && BRDF.y == 0 && BRDF.z == 0)
			{
				break;
			}

			T *= (dot(R, ray.N) / hemiPDF) * BRDF;
			ray = Ray(I + R * EPSILON, R);
			lastSpecular = false;
		}
	}

	return E;
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick( float deltaTime )
{
	// std::vector<int> valuess = Utils::LoadFromFile("GroundTruth.txt");

	// iterate over the pixels on the screen
	const float focalPlane = -1.7f;
	for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++)
	{
		// setup the primary ray
		const float u = (float)x / SCRWIDTH, v = (float)y / SCRHEIGHT;
		const float3 pointOnScreenPlane = make_float3( -1 + 2 * u, 1 - 2 * v, focalPlane );
		Ray r( make_float3( 0, 0, 0 ), normalize( pointOnScreenPlane ) );
		// intersect the ray with the scene
		accumulator[x + y * SCRWIDTH] += SampleNEEShaded(r);
	}

	// visualize the accumulator
	samplesTaken++;
	float scale = 1.0f / samplesTaken;

	std::vector<int> values;

	for (int y = 0; y < SCRHEIGHT; y++) for (int x = 0; x < SCRWIDTH; x++)
	{
		float3 p = accumulator[x + y * SCRWIDTH] * scale;
		int r = (int)(sqrtf( min( 1.0f, p.x ) ) * 255.0f);
		int g = (int)(sqrtf( min( 1.0f, p.y ) ) * 255.0f);
		int b = (int)(sqrtf( min( 1.0f, p.z ) ) * 255.0f);

		values.push_back(r + g + b);
		screen->Plot( x, y, (r << 16) + (g << 8) + b );
	}

	cout << samplesTaken << endl;

	// Save ground truth image.
	if (samplesTaken == 1024)
	{
		Utils::SaveToFile("LightWithOutNEEGroundTruthEpsilon.txt", values);
	}
}

float Tmpl8::Game::PSurvice(float3 albedoColor)
{
	float p = std::max(albedoColor.x, std::max(albedoColor.y, albedoColor.z));

	if (p < 0.1f)
	{
		p = 0.1f;
	}

	return p;
}
