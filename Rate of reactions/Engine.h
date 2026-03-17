#pragma once

#include  <SFML/Graphics.hpp>

#include<vector>
#include<mutex>

class Ball;
class ThreadPool;

class Engine
{
private:
	//Variables
	//Window
	sf::RenderWindow* window;
	sf::Event sfmlEvent;

	//Delta time
	sf::Clock deltaTimeClock;
	float deltaTime;

	//Objects
	std::vector<Ball*> balls;
	std::vector<std::mutex*> mutexBalls;
	std::mutex creationMtx;
	std::vector<sf::Vector2f> creationBalls;

	//Grid
	static const unsigned gridSize = 75;
	std::vector<int> (*grid)[gridSize];
	std::mutex (*mutexGrid)[gridSize];

	//Threads
	ThreadPool* threadpool;

	//Reaction
	float reactionChance;
	sf::Vector2f velocityRange;

	//Functions
	//Initialisation
	void initVariables();
	void initObjects();

	//Event
	void eventPolling();

	//Calculations
	void calculateDeltaTime();
	void calculateCollisionsThreading();
	void calculateCollisions();
	void calculateBallToBall(const std::vector<int>& _ballIndexes);
	void calculateBallToBallOld(const std::vector<int>& _ballIndexes);

	//Collsion threads:
	void clearGridThread(const int i);
	void gridAdditionThread(const int i);
	void calculationCollisionThread(const int i);

	//Ball edits
	void destroyBalls();
	void createBalls();

public:
	//Constructor/Destructor
	Engine();
	~Engine();

	//Accessors
	const bool isOpen() const;

	//Game loops
	void Update();
	void Render();
};

