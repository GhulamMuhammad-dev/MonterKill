#pragma once
#include<SFML/Graphics.hpp>
using namespace sf;
class Obstacle {
public:
	RectangleShape shape;

	Obstacle() {
		shape.setSize(sf::Vector2f(100.0f, 100.0f));
		shape.setFillColor(sf::Color::Red);
	}

	void getShape() {
		 shape.setFillColor(Color::Yellow);
	}

	void setPosition(float x, float y) {
		shape.setPosition(x, y);
	}

	sf::FloatRect getBounds() const {
		return shape.getGlobalBounds();
	}

	
};