#include "Ball.h"

#include<iostream>

//Constructor/Destructor
Ball::Ball(sf::Vector2f _position, sf::Vector2f _velocity, int _type)
{
	//Setup sprite
	switch (_type)
	{
	case(BallType::RED):
		this->sprite.setFillColor(sf::Color::Red);
		this->sprite.setRadius(10);
		this->mass = 1.25;
		break;
	case(BallType::BLUE):
		this->sprite.setFillColor(sf::Color::Blue);
		this->sprite.setRadius(9);
		this->mass = 1;
		break;
	case(BallType::GREEN):
		this->sprite.setFillColor(sf::Color::Green);
		this->sprite.setRadius(12);
		this->mass = 1.5;
		break;
	case(BallType::YELLOW):
		this->sprite.setFillColor(sf::Color::Yellow);
		this->sprite.setRadius(10);
		this->mass = 1;
		break;
	default:
		std::cout << "ERROR::BALL.CPP::BALL::Passed invalid type";
		this->sprite.setFillColor(sf::Color::White);
		this->sprite.setRadius(10);
		this->mass = 1;
		break;
	}

	this->sprite.setPosition(_position);
	this->sprite.setOrigin(this->sprite.getLocalBounds().width / 2.f, this->sprite.getLocalBounds().width / 2.f);

	this->velocity = _velocity / this->mass;
	this->type = _type;
	this->destory = false;
}
Ball::~Ball()
{

}

//Accessors
const sf::CircleShape Ball::getSprite() const
{
	return this->sprite;
}
const sf::Vector2f Ball::getVelocity() const
{
	return this->velocity;
}
const sf::Vector2f Ball::getPosition() const
{
	return this->sprite.getPosition();
}
const float Ball::getMass() const
{
	return this->mass;
}
const float Ball::getGlobalRadius() const
{
	return this->sprite.getGlobalBounds().width / 2.f;
}
const int Ball::getType() const
{
	return this->type;
}
const bool Ball::toDestory() const
{
	return this->destory;
}

//Modifiers
void Ball::setVelocity(sf::Vector2f _velocity)
{
	this->velocity = _velocity;
}
void Ball::setPosition(sf::Vector2f _position)
{
	this->sprite.setPosition(_position);
}
void Ball::setDestory()
{
	this->destory = true;
}

//Game loops
void Ball::Update(const float& _deltaTime)
{
	//Movement
	this->sprite.move(this->velocity * _deltaTime);
}
void Ball::Render(sf::RenderWindow& _window)
{
	_window.draw(this->sprite);
}