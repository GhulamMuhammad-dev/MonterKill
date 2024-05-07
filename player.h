#pragma once
#include<SFML/Graphics.hpp>
#include "obstacle.h";
#include<iostream>


using namespace sf;
using namespace std;

class Player:public Obstacle {

protected:
	float speed;
	Vector2f player_position;
	Texture player_texture;
	Sprite playerSprite;
	bool move_Left = false;
	bool move_Right = false;
	bool move_Up = false;
	bool move_Down = false;
	Vector2f player2Prepos;
	Obstacle o1;
	

public:
	Player(Obstacle O, string fileAddress, float playerSpeed, float startX, float startY);

	

	Vector2f getResentPosition();
	void setNewPosition(float x, float y);
	Sprite getPlayer();
	FloatRect getPosition();
	void moveLeft();
	void moveRight();
	void moveUp();
	void moveDown();

	void stopLeft();
	void stopRight();
	void stopUp();
	void stopDown();



	void update(Time dt);



};