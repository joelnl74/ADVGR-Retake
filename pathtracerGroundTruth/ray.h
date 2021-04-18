class Ray 
{ 
public: 
	Ray( float3 o, float3 d, float l = 1e34f, bool i = false ) : O( o ), D( d ), t( l ), inside( i ) {}
	float3 O, D, N; 
	float t = 1e34f; 
	bool inside = false; 
	int objIdx = -1; 
};