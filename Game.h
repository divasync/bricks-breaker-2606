#pragma once
#pragma push_macro("new")
#undef new
#include <vector>
#pragma pop_macro("new")
#include "Box.h"
#include "Ball.h"

class Game
{
	Ball ball;
	Box paddle;

	std::vector<Box> bricks;
	bool won = false;
	bool lost = false;

public:
	Game();
	bool Update();
	void Render() const;
	void Reset();
	void ResetBall();
	void CheckCollision();
};