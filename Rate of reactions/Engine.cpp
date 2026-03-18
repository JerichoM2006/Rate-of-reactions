#include "Engine.h"
#include "Ball.h"
#include "ThreadPool.h"

#include<iostream>;

//Constructor/Destructor
Engine::Engine()
{
	initVariables();
}
Engine::~Engine()
{
	delete this->window;
	delete[] this->grid;
	delete[] this->mutexGrid;
	delete this->threadpool;
	for (auto ball : this->balls)
		delete ball;
	for (auto mutexBall : this->mutexBalls)
		delete mutexBall;
}

//Initialisations
void Engine::initVariables()
{
	//Window
	//Original was (800, 600)
	//Good ones (960, 720), (500, 500), (1200, 900), (1500, 1000)
	this->window = new sf::RenderWindow(sf::VideoMode(1500, 1000), "Rate of reactions", sf::Style::Close | sf::Style::Titlebar);

	//Delta time
	this->deltaTime = deltaTimeClock.restart().asSeconds();

	//Grid
	this->grid = new std::vector<int>[this->gridSize][this->gridSize];
	this->mutexGrid = new std::mutex[this->gridSize][this->gridSize];

	//Thread pool
	this->threadpool = new ThreadPool(4);

	//Reaction
	this->reactionChance = 0.25f;
	this->velocityRange = sf::Vector2f(200.f, 300.f);

	//Objects
	this->initObjects();
}
void Engine::initObjects()
{
	for (int i = 0; i < 750; i++)
	{
		//Position
		sf::Vector2f postion = sf::Vector2f(this->window->getSize().x * static_cast<float>(rand()) / RAND_MAX, this->window->getSize().y * static_cast<float>(rand()) / RAND_MAX);

		//Velocity
		float unitVelocityAngle = rand() / (RAND_MAX / 360.f);
		sf::Vector2f unitVelocity = sf::Vector2f(static_cast<float>(cos(unitVelocityAngle)), static_cast<float>(sin(unitVelocityAngle)));
		float range = this->velocityRange.x + (rand() / (RAND_MAX / (this->velocityRange.y - this->velocityRange.x)));
		sf::Vector2f velocity = unitVelocity * range;

		//Type
		int type = i % 2;
		//std::cout << type << std::endl;

		//Initialise ball and mutex
		this->balls.push_back(new Ball(postion, velocity, type));
		this->mutexBalls.push_back(new std::mutex());
	}
}

//Accessors
const bool Engine::isOpen() const
{
	return this->window->isOpen();
}

//Event polling
void Engine::eventPolling()
{
	while (this->window->pollEvent(this->sfmlEvent))
	{
		switch (this->sfmlEvent.type)
		{
		case(sf::Event::Closed):
			this->window->close();
			break;
		case(sf::Event::KeyPressed):
			if (this->sfmlEvent.type == sf::Keyboard::Escape)
				this->window->close();
			break;
		default:
			break;
		}
	}
}

//Calculations
void Engine::calculateDeltaTime()
{
	this->deltaTime = deltaTimeClock.restart().asSeconds();
}
void Engine::calculateCollisionsThreading()
{
	//Clear grid
	for (int i = 0; i < this->gridSize; i++)
	{
		this->threadpool->Enqueue(std::bind(&Engine::clearGridThread, this, i));
	}

	this->threadpool->Barrier();

	//Grid addition
	for (int i = 0; i < this->balls.size(); i++)
	{
		this->threadpool->Enqueue(std::bind(&Engine::gridAdditionThread, this, i));
	}

	this->threadpool->Barrier();

	//Grid calculation
	for (int i = 0; i < this->gridSize; i++)
	{
		this->threadpool->Enqueue(std::bind(&Engine::calculationCollisionThread, this, i));
	}

	this->threadpool->Barrier();
}
void Engine::calculateCollisions()
{
	//Clear contents of grid
	for (int y = 0; y < this->gridSize; y++)
	{
		for (int x = 0; x < this->gridSize; x++)
		{
			this->grid[y][x].clear();
		}
	}

	for (int i = 0; i < this->balls.size(); i++)
	{
		//Update collisions for walls
		if (balls[i]->getSprite().getGlobalBounds().left < 0.f) //Left collision
		{
			balls[i]->setVelocity(sf::Vector2f(-balls[i]->getVelocity().x, balls[i]->getVelocity().y));
			balls[i]->setPosition(sf::Vector2f(balls[i]->getGlobalRadius() + 1.f, balls[i]->getPosition().y));
		}
		else if (balls[i]->getSprite().getGlobalBounds().left + balls[i]->getSprite().getGlobalBounds().width > this->window->getSize().x) //Right collision
		{
			balls[i]->setVelocity(sf::Vector2f(-balls[i]->getVelocity().x, balls[i]->getVelocity().y));
			balls[i]->setPosition(sf::Vector2f(this->window->getSize().x - balls[i]->getGlobalRadius() - 1.f, balls[i]->getPosition().y));
		}
		if (balls[i]->getSprite().getGlobalBounds().top < 0.f)
		{
			balls[i]->setVelocity(sf::Vector2f(balls[i]->getVelocity().x, -balls[i]->getVelocity().y));
			balls[i]->setPosition(sf::Vector2f(balls[i]->getPosition().x, balls[i]->getGlobalRadius() + 1.f));
		}
		else if (balls[i]->getSprite().getGlobalBounds().top + balls[i]->getSprite().getGlobalBounds().height > this->window->getSize().y)
		{
			balls[i]->setVelocity(sf::Vector2f(balls[i]->getVelocity().x, -balls[i]->getVelocity().y));
			balls[i]->setPosition(sf::Vector2f(balls[i]->getPosition().x, this->window->getSize().y - balls[i]->getGlobalRadius() - 1.f));
		}

		//Assign to grid	
		//What to round it to
		float intervalX = this->window->getSize().x / static_cast<float>(this->gridSize);
		float intervalY = this->window->getSize().y / static_cast<float>(this->gridSize);

		//Offset of the rounding
		float offsetX = intervalX / 2.f;
		float offsetY = intervalY / 2.f;

		//Find the minimum and maximum
		int minimumX = static_cast<int>(round((balls[i]->getPosition().x - balls[i]->getGlobalRadius() - offsetX) / intervalX)) - 1;
		int maximumX = static_cast<int>(round((balls[i]->getPosition().x + balls[i]->getGlobalRadius() - offsetX) / intervalX)) + 1;
		int minimumY = static_cast<int>(round((balls[i]->getPosition().y - balls[i]->getGlobalRadius() - offsetY) / intervalY)) - 1;
		int maximumY = static_cast<int>(round((balls[i]->getPosition().y + balls[i]->getGlobalRadius() - offsetY) / intervalY)) + 1;

		//Add to grid
		for (int y = minimumY; y <= maximumY; y++)
		{
			for (int x = minimumX; x <= maximumX; x++)
			{
				if (x > 0 && x < this->gridSize && y > 0 && y < this->gridSize)
				{
					this->grid[y][x].push_back(i);
				}
			}
		}
	}

	//Calculate ball to ball collsions
	for (int y = 0; y < this->gridSize; y++)
	{
		for (int x = 0; x < this->gridSize; x++)
		{
			if (grid[y][x].size() > 1)
				calculateBallToBallOld(this->grid[y][x]);
		}
	}
}
void Engine::calculateBallToBallOld(const std::vector<int>& _ballIndexes)
{
	for (int i = 0; i < _ballIndexes.size() - 1; i++)
	{
		//Update collisions for other bodies
		for (int j = i + 1; j < _ballIndexes.size(); j++)
		{
			//Check if colliding
			sf::Vector2f normalVector = balls[_ballIndexes[j]]->getPosition() - balls[_ballIndexes[i]]->getPosition();
			float distance = sqrt(normalVector.x * normalVector.x + normalVector.y * normalVector.y);
			if (distance <= balls[_ballIndexes[i]]->getGlobalRadius() + balls[_ballIndexes[j]]->getGlobalRadius())
			{
				//Find normal and tangent units
				sf::Vector2f unitNormal = normalVector / distance;
				sf::Vector2f unitTangent = sf::Vector2f(-unitNormal.y, unitNormal.x);

				//Project these vectors into the ball velocities using dot product
				float v1n = (balls[_ballIndexes[i]]->getVelocity().x * unitNormal.x) + (balls[_ballIndexes[i]]->getVelocity().y * unitNormal.y);
				float v1t = (balls[_ballIndexes[i]]->getVelocity().x * unitTangent.x) + (balls[_ballIndexes[i]]->getVelocity().y * unitTangent.y);
				float v2n = (balls[_ballIndexes[j]]->getVelocity().x * unitNormal.x) + (balls[_ballIndexes[j]]->getVelocity().y * unitNormal.y);
				float v2t = (balls[_ballIndexes[j]]->getVelocity().x * unitTangent.x) + (balls[_ballIndexes[j]]->getVelocity().y * unitTangent.y);

				//Store masses of two objects for simplification
				float m1 = balls[_ballIndexes[i]]->getMass();
				float m2 = balls[_ballIndexes[j]]->getMass();

				//Find the new normal velocities using equation
				float fv1n = (v1n * (m1 - m2) + (2.f * m2 * v2n)) / (m1 + m2);
				float fv2n = (v2n * (m2 - m1) + (2.f * m1 * v1n)) / (m1 + m2);

				//Convert scalars into vectors
				sf::Vector2f Vectorv1n = fv1n * unitNormal;
				sf::Vector2f Vectorv1t = v1t * unitTangent;
				sf::Vector2f Vectorv2n = fv2n * unitNormal;
				sf::Vector2f Vectorv2t = v2t * unitTangent;

				//Set velocities
				balls[_ballIndexes[i]]->setVelocity(Vectorv1n + Vectorv1t);
				balls[_ballIndexes[j]]->setVelocity(Vectorv2n + Vectorv2t);

				//Set positions outside of ball
				balls[_ballIndexes[i]]->setPosition(balls[_ballIndexes[j]]->getPosition() + (-unitNormal * (balls[_ballIndexes[i]]->getGlobalRadius() + balls[_ballIndexes[j]]->getGlobalRadius())));
				balls[_ballIndexes[j]]->setPosition(balls[_ballIndexes[i]]->getPosition() + (unitNormal * (balls[_ballIndexes[i]]->getGlobalRadius() + balls[_ballIndexes[j]]->getGlobalRadius())));
			}
		}
	}
}
void Engine::calculateBallToBall(const std::vector<int>& _ballIndexes)
{
	for (int i = 0; i < _ballIndexes.size() - 1; i++)
	{
		//Update collisions for other bodies
		for (int j = i + 1; j < _ballIndexes.size(); j++)
		{
			//Initialise values need for collision
			//Ball 1
			sf::Vector2f ball1Position;
			sf::Vector2f ball1Velocity;
			float ball1Radius;
			float m1;
			int ball1Type;
			bool ball1Destroy;
			//Ball 2
			sf::Vector2f ball2Position;
			sf::Vector2f ball2Velocity;
			float ball2Radius;
			float m2;
			int ball2Type;
			bool ball2Destroy;

			//Get data from ball 1
			{
				std::unique_lock<std::mutex> lock(*this->mutexBalls[_ballIndexes[i]]);
				ball1Position = this->balls[_ballIndexes[i]]->getPosition();
				ball1Velocity = this->balls[_ballIndexes[i]]->getVelocity();
				ball1Radius = this->balls[_ballIndexes[i]]->getGlobalRadius();
				m1 = this->balls[_ballIndexes[i]]->getMass();
				ball1Type = this->balls[_ballIndexes[i]]->getType();
				ball1Destroy = this->balls[_ballIndexes[i]]->toDestory();
			}
			//Get data from ball 2
			{
				std::unique_lock<std::mutex> lock(*this->mutexBalls[_ballIndexes[j]]);
				ball2Position = this->balls[_ballIndexes[j]]->getPosition();
				ball2Velocity = this->balls[_ballIndexes[j]]->getVelocity();
				ball2Radius = this->balls[_ballIndexes[j]]->getGlobalRadius();
				m2 = this->balls[_ballIndexes[j]]->getMass();
				ball2Type = this->balls[_ballIndexes[j]]->getType();
				ball2Destroy = this->balls[_ballIndexes[j]]->toDestory();
			}

			//Check if colliding
			sf::Vector2f normalVector = ball2Position - ball1Position;
			float distance = sqrt(normalVector.x * normalVector.x + normalVector.y * normalVector.y);
			if (distance <= ball1Radius + ball2Radius)
			{
				//Find normal and tangent units
				sf::Vector2f unitNormal = normalVector / distance;
				sf::Vector2f unitTangent = sf::Vector2f(-unitNormal.y, unitNormal.x);

				//Project these vectors into the ball velocities using dot product
				float v1n = (ball1Velocity.x * unitNormal.x) + (ball1Velocity.y * unitNormal.y);
				float v1t = (ball1Velocity.x * unitTangent.x) + (ball1Velocity.y * unitTangent.y);
				float v2n = (ball2Velocity.x * unitNormal.x) + (ball2Velocity.y * unitNormal.y);
				float v2t = (ball2Velocity.x * unitTangent.x) + (ball2Velocity.y * unitTangent.y);

				//Find the new normal velocities using equation
				float fv1n = (v1n * (m1 - m2) + (2.f * m2 * v2n)) / (m1 + m2);
				float fv2n = (v2n * (m2 - m1) + (2.f * m1 * v1n)) / (m1 + m2);

				//Convert scalars into vectors
				sf::Vector2f Vectorv1n = fv1n * unitNormal;
				sf::Vector2f Vectorv1t = v1t * unitTangent;
				sf::Vector2f Vectorv2n = fv2n * unitNormal;
				sf::Vector2f Vectorv2t = v2t * unitTangent;

				bool rightCombination = ball1Type == BallType::RED && ball2Type == BallType::BLUE || ball1Type == BallType::BLUE && ball2Type == BallType::RED;
				bool reaction = !ball1Destroy && !ball2Destroy && rightCombination && (reactionChance > static_cast<float>(rand()) / RAND_MAX);

				//Set new state for ball 1
				{
					std::unique_lock<std::mutex> lock(*this->mutexBalls[_ballIndexes[i]]);
					this->balls[_ballIndexes[i]]->setVelocity(Vectorv1n + Vectorv1t);
					this->balls[_ballIndexes[i]]->setPosition(ball2Position + (-unitNormal * (ball1Radius + ball2Radius + 0.1f)));
					if (reaction) this->balls[_ballIndexes[i]]->setDestory();
				}

				//Set new state for ball 2
				{
					std::unique_lock<std::mutex> lock(*this->mutexBalls[_ballIndexes[j]]);
					this->balls[_ballIndexes[j]]->setVelocity(Vectorv2n + Vectorv2t);
					this->balls[_ballIndexes[j]]->setPosition(ball1Position + (unitNormal * (ball1Radius + ball2Radius + 0.1f)));
					if (reaction) this->balls[_ballIndexes[j]]->setDestory();
				}

				if (reaction)
				{
					std::unique_lock<std::mutex> lock(this->creationMtx);
					sf::Vector2f postion = (ball1Position + ball2Position) / 2.f;
					this->creationBalls.push_back(postion);
				}
			}
		}
	}
}

//Collsion threads
void Engine::clearGridThread(const int i)
{
	for (int x = 0; x < this->gridSize; x++)
	{
		this->grid[i][x].clear();
	}
}
void Engine::gridAdditionThread(const int i)
{
	//Update collisions for walls
	if (this->balls[i]->getSprite().getGlobalBounds().left < 0.f) //Left collision
	{
		this->balls[i]->setVelocity(sf::Vector2f(-this->balls[i]->getVelocity().x, this->balls[i]->getVelocity().y));
		this->balls[i]->setPosition(sf::Vector2f(this->balls[i]->getGlobalRadius() + 0.1f, this->balls[i]->getPosition().y));
	}
	else if (this->balls[i]->getSprite().getGlobalBounds().left + this->balls[i]->getSprite().getGlobalBounds().width > this->window->getSize().x) //Right collision
	{
		this->balls[i]->setVelocity(sf::Vector2f(-this->balls[i]->getVelocity().x, this->balls[i]->getVelocity().y));
		this->balls[i]->setPosition(sf::Vector2f(this->window->getSize().x - this->balls[i]->getGlobalRadius() - 0.1f, this->balls[i]->getPosition().y));
	}
	if (this->balls[i]->getSprite().getGlobalBounds().top < 0.f)
	{
		this->balls[i]->setVelocity(sf::Vector2f(this->balls[i]->getVelocity().x, -this->balls[i]->getVelocity().y));
		this->balls[i]->setPosition(sf::Vector2f(this->balls[i]->getPosition().x, this->balls[i]->getGlobalRadius() + 0.1f));
	}
	else if (this->balls[i]->getSprite().getGlobalBounds().top + this->balls[i]->getSprite().getGlobalBounds().height > this->window->getSize().y)
	{
		this->balls[i]->setVelocity(sf::Vector2f(this->balls[i]->getVelocity().x, -this->balls[i]->getVelocity().y));
		this->balls[i]->setPosition(sf::Vector2f(this->balls[i]->getPosition().x, this->window->getSize().y - this->balls[i]->getGlobalRadius() - 0.1f));
	}

	//Assign to grid	
	//What to round it to
	float intervalX = this->window->getSize().x / static_cast<float>(this->gridSize);
	float intervalY = this->window->getSize().y / static_cast<float>(this->gridSize);

	//Offset of the rounding
	float offsetX = intervalX / 2.f;
	float offsetY = intervalY / 2.f;

	//Find the minimum and maximum
	int minimumX = static_cast<int>(round((this->balls[i]->getPosition().x - this->balls[i]->getGlobalRadius() - offsetX) / intervalX)) - 1;
	int maximumX = static_cast<int>(round((this->balls[i]->getPosition().x + this->balls[i]->getGlobalRadius() - offsetX) / intervalX)) + 1;
	int minimumY = static_cast<int>(round((this->balls[i]->getPosition().y - this->balls[i]->getGlobalRadius() - offsetY) / intervalY)) - 1;
	int maximumY = static_cast<int>(round((this->balls[i]->getPosition().y + this->balls[i]->getGlobalRadius() - offsetY) / intervalY)) + 1;

	//Add to grid
	for (int y = minimumY; y <= maximumY; y++)
	{
		for (int x = minimumX; x <= maximumX; x++)
		{
			if (x > 0 && x < this->gridSize && y > 0 && y < this->gridSize)
			{
				std::unique_lock<std::mutex> lock(this->mutexGrid[y][x]);
				this->grid[y][x].push_back(i);
			}
		}
	}
}
void Engine::calculationCollisionThread(const int i)
{
	for (int x = 0; x < this->gridSize; x++)
	{
		if (grid[i][x].size() > 1)
			calculateBallToBall(this->grid[i][x]);
	}
}

//Destroy balls
void Engine::destroyBalls()
{
	for (size_t i = 0; i < this->balls.size(); i++)
	{
		if (balls[i]->toDestory())
		{
			delete this->balls[i];
			delete this->mutexBalls[i];
			this->balls.erase(this->balls.begin() + i);
			this->mutexBalls.erase(this->mutexBalls.begin() + i);
			i--;
		}
	}
}
void Engine::createBalls()
{
	for (size_t i = 0; i < this->creationBalls.size(); i++)
	{
		//Velocity
		float unitVelocityAngle = rand() / (RAND_MAX / 360.f);
		sf::Vector2f unitVelocity = sf::Vector2f(static_cast<float>(cos(unitVelocityAngle)), static_cast<float>(sin(unitVelocityAngle)));
		float range = this->velocityRange.x + (rand() / (RAND_MAX / (this->velocityRange.y - this->velocityRange.x)));
		sf::Vector2f velocity = unitVelocity * range;

		//Type
		int type = BallType::GREEN;

		//Initialise ball and mutex
		this->balls.push_back(new Ball(creationBalls[i], velocity, type));
		this->mutexBalls.push_back(new std::mutex());

		//Remove created ball from vector
		this->creationBalls.erase(this->creationBalls.begin() + i);
	}
}

//Game loops
void Engine::Update()
{
	std::cout << "FPS: " << static_cast<int>(1.f / this->deltaTime) << std::endl;

	this->eventPolling();

	//this->calculateCollisions();
	this->calculateCollisionsThreading();

	for (auto ball : this->balls)
		this->threadpool->Enqueue(std::bind(&Ball::Update, ball, this->deltaTime));

	this->destroyBalls();
	this->createBalls();

	this->calculateDeltaTime();
}
void Engine::Render()
{
	//Clear previouse frame
	this->window->clear();
	
	//Draw objects
	for (auto ball : this->balls)
		ball->Render(*this->window);

	//Display drawed objects
	this->window->display();
}