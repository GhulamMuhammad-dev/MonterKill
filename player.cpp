#include "player.h"




Player::Player(Obstacle O, string fileAddress, float playerSpeed, float startX, float startY) {
	player_position.x = startX;
	player_position.y = startY;
	speed = playerSpeed;
	o1 = O;

	player_texture.loadFromFile(fileAddress);
	playerSprite.setTexture(player_texture);
	playerSprite.setPosition(player_position);
	

	
}

void Player::setNewPosition(float x, float y) {
	playerSprite.setPosition(x, y);
}

Vector2f Player::getResentPosition() {
	return  playerSprite.getPosition();
}



Sprite Player::getPlayer() {
	return playerSprite;
}

FloatRect Player::getPosition() {
	return playerSprite.getGlobalBounds();
}

void Player::moveLeft() {
	move_Left = true;
}

void Player::moveRight() {
	move_Right = true;
}

void Player::moveUp() {
	move_Up = true;
}
void Player::moveDown() {
	move_Down = true;
}

void Player::stopLeft() {
	move_Left = false;
}


void Player::stopDown() {
	move_Down = false;
}


void Player::stopRight() {
	move_Right = false;
}


void Player::stopUp() {
	move_Up = false;
}

void Player::update(Time dt) {
	
	if (move_Left) {
		player_position.x -= speed * dt.asSeconds();
	}
	if (move_Right) {
		player_position.x += speed * dt.asSeconds();
	}
	if (move_Up) {
		player_position.y -= speed * dt.asSeconds();
	}
	if (move_Down) {
		player_position.y += speed * dt.asSeconds();
	}

	if (playerSprite.getGlobalBounds().intersects(o1.getBounds())) {
		player_position.x = player2Prepos.x;
		player_position.y = player2Prepos.y;
		o1.getShape();
		
	}

	player2Prepos = playerSprite.getPosition();
	playerSprite.setPosition(player_position);

}

