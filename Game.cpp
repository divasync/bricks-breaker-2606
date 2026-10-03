#include "stdafx.h"
#include "Game.h"

Game::Game()
{
	Reset();
}

void Game::Reset()
{
	Console::SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	Console::CursorVisible(false);

	won = false;
	lost = false;

	paddle.width = 12;
	paddle.height = 2;
	paddle.x_position = 32;
	paddle.y_position = 30;

	ball.visage = 'O';
	ball.color = ConsoleColor::Cyan;
	ResetBall();

	// Create five bricks with equal spacing.
	bricks.clear();

	const int brickCount = 5;
	const int brickWidth = 10;
	const int gap =
		(WINDOW_WIDTH - brickCount * brickWidth) / (brickCount + 1);

	for (int i = 0; i < brickCount; ++i)
	{
		Box brick;
		brick.width = brickWidth;
		brick.height = 2;
		brick.x_position = gap + i * (brickWidth + gap);
		brick.y_position = 5;
		brick.doubleThick = true;

		// Three hits: DarkCyan -> DarkGreen -> DarkBlue -> Black.
		brick.color = ConsoleColor::DarkCyan;
		bricks.push_back(brick);
	}
}

void Game::ResetBall()
{
	ball.x_position = paddle.x_position + paddle.width / 2;
	ball.y_position = paddle.y_position - 1;
	ball.x_velocity = rand() % 2 ? 1 : -1;
	ball.y_velocity = -1;
	ball.moving = false;
}

bool Game::Update()
{
	if (GetAsyncKeyState(VK_ESCAPE) & 0x1)
		return false;

	if (GetAsyncKeyState('R') & 0x1)
	{
		Reset();
		return true;
	}

	// Keep the game stopped after winning or losing.
	if (won || lost)
		return true;

	if (GetAsyncKeyState(VK_RIGHT) &&
		paddle.x_position < WINDOW_WIDTH - paddle.width)
	{
		paddle.x_position += 2;
	}

	if (GetAsyncKeyState(VK_LEFT) && paddle.x_position > 0)
	{
		paddle.x_position -= 2;
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x1)
		ball.moving = !ball.moving;

	// Check the next position before moving.
	CheckCollision();
	ball.Update();

	return true;
}

void Game::Render() const
{
	Console::Lock(true);
	Console::Clear();

	paddle.Draw();
	ball.Draw();

	for (const Box& brick : bricks)
	{
		brick.Draw();
	}

	if (won || lost)
	{
		const char* message = won
			? "You win! Press R to play again."
			: "You lose. Press R to play again.";

		int length = 0;
		while (message[length] != '\0')
		{
			++length;
		}

		Console::ForegroundColor(ConsoleColor::White);
		Console::SetCursorPosition(
			(WINDOW_WIDTH - length) / 2,
			WINDOW_HEIGHT / 2);

		std::cout << message << std::flush;
	}

	Console::Lock(false);
}

void Game::CheckCollision()
{
	if (won || lost)
		return;

	if (bricks.empty())
	{
		won = true;
		ball.moving = false;
		return;
	}

	// Pausing must also stop collision damage.
	if (!ball.moving)
		return;

	const int nextX = ball.x_position + ball.x_velocity;
	const int nextY = ball.y_position + ball.y_velocity;

	// Touching the bottom ends the game.
	if (ball.y_position >= WINDOW_HEIGHT - 1 ||
		nextY >= WINDOW_HEIGHT - 1)
	{
		ball.y_position = WINDOW_HEIGHT - 1;
		ball.moving = false;
		lost = true;
		return;
	}

	for (auto brick = bricks.begin(); brick != bricks.end(); ++brick)
	{
		if (!brick->Contains(nextX, nextY))
			continue;

		const bool hitSide =
			brick->Contains(nextX, ball.y_position);

		const bool hitTopOrBottom =
			brick->Contains(ball.x_position, nextY);

		if (hitSide)
			ball.x_velocity *= -1;

		if (hitTopOrBottom)
			ball.y_velocity *= -1;

		// A diagonal corner hit reverses both directions.
		if (!hitSide && !hitTopOrBottom)
		{
			ball.x_velocity *= -1;
			ball.y_velocity *= -1;
		}

		brick->color = static_cast<ConsoleColor>(brick->color - 1);

		if (brick->color == ConsoleColor::Black)
		{
			bricks.erase(brick);
		}

		// Stop immediately so an erased iterator is never reused.
		break;
	}

	if (bricks.empty())
	{
		won = true;
		ball.moving = false;
		return;
	}

	// Bounce off the paddle while moving downward.
	if (ball.y_velocity > 0 &&
		paddle.Contains(
			ball.x_position + ball.x_velocity,
			ball.y_position + ball.y_velocity))
	{
		ball.y_velocity *= -1;
	}
}