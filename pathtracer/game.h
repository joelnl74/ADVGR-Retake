#pragma once
namespace Tmpl8 {

class Game
{
public:
	float3 SampleNEEShaded( Ray& ray );
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init() { memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 3);  }
	void Shutdown() {}
	void Tick( float deltaTime );
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { /* implement if you want to detect mouse movement */ }
	void KeyUp( int key ) { /* implement if you want to handle keys */ }
	void KeyDown( int key ) { /* implement if you want to handle keys */ }
	Surface* screen;
	Scene scene;
	float3* accumulator = new float3[SCRWIDTH * SCRHEIGHT];
	int samplesTaken = 0;

private:
	const int m_maxDepth = 20;
	std::vector<float> rmsValues;
};

}; // namespace Tmpl8