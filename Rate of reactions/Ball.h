#pragma once

#include<SFML/Graphics.hpp>

enum BallType{RED = 0, BLUE, GREEN, YELLOW, NUMOFTYPES};

class Ball
{
private:
	//Variables
	sf::CircleShape sprite;
	sf::Vector2f velocity;
	float mass;
	int type;
	bool destory;

public:
	//Constructor/Destructor
	Ball(sf::Vector2f _position, sf::Vector2f _velocity, int _type);
	~Ball();

	//Accessors
	const sf::CircleShape getSprite() const;
	const sf::Vector2f getVelocity() const;
	const sf::Vector2f getPosition() const;
	const float getMass() const;
	const float getGlobalRadius() const;
	const int getType() const;
	const bool toDestory() const;

	//Modifiers
	void setVelocity(sf::Vector2f _velocity);
	void setPosition(sf::Vector2f _position);
	void setDestory();

	//Game loops
	void Update(const float& _deltaTime);
	void Render(sf::RenderWindow& _window);
};