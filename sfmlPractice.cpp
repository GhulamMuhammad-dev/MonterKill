#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <ctime>

class Ground {
public:
    sf::RectangleShape shape;

    Ground(float x, float y, float width, float height) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color::Green);
    }
};
class Particle {
public:
    sf::CircleShape shape;
    float speed;

    Particle(float x, float y, float radius, float speed) {
        shape.setPosition(x, y);
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::Yellow);
        this->speed = speed;
    }

    void update(float dt) {
        shape.move(-speed * dt, 0); // Moves left
    }

    bool isOutOfScreen() {
        return shape.getPosition().x < -2 * shape.getRadius(); // If off-screen
    }
};


class Object {
public:
    sf::RectangleShape shape;
    float velocityY;
    const float gravity = 1000.0f;
    const float jumpStrength = -400.0f;
    const float movementSpeed = 200.0f;
    bool isJumping;

    Object(float x, float y, float size) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(sf::Color::Blue);
        velocityY = 0;
        isJumping = false;
    }

    void applyGravity(float dt) {
        velocityY += gravity * dt;
    }

    void jump() {
        if (!isJumping) {
            velocityY = jumpStrength;
            isJumping = true;
        }
    }

    void update(float dt, const Ground& ground) {
        applyGravity(dt);
        shape.move(0, velocityY * dt);

        if (shape.getGlobalBounds().intersects(ground.shape.getGlobalBounds())) {
            shape.setPosition(shape.getPosition().x, ground.shape.getPosition().y - shape.getSize().y);
            velocityY = 0;
            isJumping = false;
        }
    }

    bool checkCollision(const sf::RectangleShape& other) {
        return shape.getGlobalBounds().intersects(other.getGlobalBounds());
    }
};

class Bullet {
public:
    sf::RectangleShape shape;
    float speed;

    Bullet(float x, float y, float speed) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(10, 5)); // Bullet shape
        shape.setFillColor(sf::Color::White);
        this->speed = speed;
    }

    void update(float dt) {
        shape.move(speed * dt, 0); // Move right
    }

    bool isOutOfScreen() {
        return shape.getPosition().x > 800; // If off-screen to the right
    }
};

class Gun {
public:
    std::vector<Bullet> bullets;
    const float bulletSpeed = 500.0f; // Speed of bullets

    void shoot(float x, float y) {
        bullets.push_back(Bullet(x, y, bulletSpeed)); // Shoot from given position
    }

    void update(float dt) {
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->update(dt); // Update bullets

            if (it->isOutOfScreen()) {
                it = bullets.erase(it); // Remove off-screen bullets
            }
            else {
                ++it;
            }
        }
    }
};

class Monster {
public:
    sf::RectangleShape shape;
    int health; // Monster's health
    float speed;
    bool canBeKilled; // Determines if a monster is invincible

    Monster(float x, float y, float size, float speed, int health, bool canBeKilled) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(canBeKilled ? sf::Color::Red : sf::Color::Black); // Different color for invincible monsters
        this->speed = speed;
        this->health = health;
        this->canBeKilled = canBeKilled;
    }

    void update(float dt) {
        shape.move(-speed * dt, 0); // Move left
    }

    bool isOutOfScreen() {
        return shape.getPosition().x < -shape.getSize().x; // If off-screen to the left
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Player with Gun, Monsters, and Environment Particles");
    Ground ground(0, 500, 800, 100);
    Object object(100, 450, 50);
    Gun gun; // Create a gun for the player
    int playerScore = 0; // Initialize player's score

    std::vector<Monster> monsters;
    std::vector<Particle> particles;
    sf::Clock clock;
    sf::Clock monsterSpawnClock;
    sf::Clock particleSpawnClock;
    std::default_random_engine generator(std::time(nullptr)); // Seed random generator with time
    std::uniform_real_distribution<float> speedDistribution(100.0f, 300.0f); // Monster speed distribution
    std::uniform_int_distribution<int> healthDistribution(1, 5); // Monster health distribution
    std::uniform_int_distribution<int> spawnTimeDistribution(1, 3); // Monster spawn time
    std::uniform_int_distribution<int> invincibilityDistribution(0, 1); // 0 or 1 for canBeKilled
    std::uniform_real_distribution<float> particleSpawnTime(0.1f, 0.3f);
    std::uniform_real_distribution<float> particleSpeed(150.0f, 250.0f);
    std::uniform_real_distribution<float> particleY(100.0f, 400.0f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) { // Check for mouse click
                gun.shoot(object.shape.getPosition().x + object.shape.getSize().x, object.shape.getPosition().y + 10); // Shoot from player position
            }
        }

        float dt = clock.restart().asSeconds();

        // Input handling
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            object.shape.move(-object.movementSpeed * dt, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            object.shape.move(object.movementSpeed * dt, 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            object.jump();
        }

        // Update object
        object.update(dt, ground);

        // Update the gun and bullets
        gun.update(dt);

        // Monster generation
        if (monsterSpawnClock.getElapsedTime().asSeconds() > spawnTimeDistribution(generator)) {
            bool canBeKilled = invincibilityDistribution(generator) == 1;
            monsters.push_back(Monster(800, 450, 50, speedDistribution(generator), healthDistribution(generator), canBeKilled)); // Create new monster with random speed, health, and invincibility
            monsterSpawnClock.restart();
        }

        // Update monsters and check for collisions with bullets
        for (auto it = monsters.begin(); it != monsters.end();) {
            it->update(dt);

            if (it->canBeKilled) { // Only apply bullet logic if the monster can be killed
                bool monsterHit = false;
                for (auto bullet = gun.bullets.begin(); bullet != gun.bullets.end();) {
                    if (bullet->shape.getGlobalBounds().intersects(it->shape.getGlobalBounds())) {
                        it->health--; // Reduce monster health
                        bullet = gun.bullets.erase(bullet); // Remove bullet after hit
                        monsterHit = true;
                    }
                    else {
                        ++bullet;
                    }
                }

                if (it->health <= 0) { // If monster's health is zero or less
                    playerScore++; // Increase player's score
                    it = monsters.erase(it); // Remove monster
                }
                else if (it->isOutOfScreen()) {
                    it = monsters.erase(it); // If off-screen
                }
                else {
                    ++it;
                }
            }
            else {
                // If the monster cannot be killed by bullets, just check if it's out of screen
                if (it->isOutOfScreen()) {
                    it = monsters.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        // Particle generation
        if (particleSpawnClock.getElapsedTime().asSeconds() > particleSpawnTime(generator)) {
            particles.push_back(Particle(800, particleY(generator), 5, particleSpeed(generator))); // Create environment particles
            particleSpawnClock.restart();
        }

        for (auto it = particles.begin(); it != particles.end();) {
            it->update(dt); // Correctly call `update()` on the particle object

            if (it->isOutOfScreen()) {
                it = particles.erase(it); // Erase if off-screen
            }
            else {
                ++it; // Move to the next iterator
            }
        }

        // Draw everything
        window.clear();
        window.draw(ground.shape);
        window.draw(object.shape);
        for (const auto& monster : monsters) {
            window.draw(monster.shape);
        }
        for (const auto& particle : particles) {
            window.draw(particle.shape);
        }
        for (const auto& bullet : gun.bullets) {
            window.draw(bullet.shape); // Draw bullets
        }
        window.display();
    }

    return 0;
}
