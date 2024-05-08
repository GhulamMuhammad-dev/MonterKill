#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <ctime>

// Ground class
class Ground {
public:
    sf::RectangleShape shape;

    Ground(float x, float y, float width, float height) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color::Green);
    }
};

// Object class for the player
class Object {
public:
    sf::RectangleShape shape;
    float velocityY;
    const float gravity = 1000.0f;
    const float jumpStrength = -400.0f;
    const float movementSpeed = 200.0f;
    bool isJumping;
    int hitCount = 0;

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

// Bullet class with custom sprite
class Bullet {
public:
    sf::Sprite sprite;
    float speed;

    Bullet(const sf::Texture& texture, float x, float y, float speed) {
        sprite.setTexture(texture); // Apply the texture to the sprite
        sprite.setPosition(x, y);
        this->speed = speed;
    }

    void update(float dt) {
        sprite.move(speed * dt, 0); // Bullets move to the right
    }

    bool isOutOfScreen() {
        return sprite.getPosition().x > 800; // If off-screen to the right
    }
};

// Gun class for shooting bullets
class Gun {
public:
    std::vector<Bullet> bullets;
    sf::Texture bulletTexture; // Texture for bullets
    const float bulletSpeed = 500.0f;

    Gun(const std::string& texturePath) {
        if (!bulletTexture.loadFromFile(texturePath)) {
            std::cerr << "Error loading bullet texture." << std::endl;
        }
    }

    void shoot(float x, float y) {
        bullets.push_back(Bullet(bulletTexture, x, y, bulletSpeed)); // Create a bullet with a custom sprite
    }

    void update(float dt) {
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->update(dt); // Update bullets
            if (it->isOutOfScreen()) {
                it = bullets.erase(it); // Remove off-screen bullets
            }
            else {
                ++it; // Continue to the next bullet
            }
        }
    }
};

// Monster class
class Monster {
public:
    sf::RectangleShape shape;
    int health;
    float speed;
    bool canBeKilled;

    Monster(float x, float y, float size, float speed, int health, bool canBeKilled) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(canBeKilled ? sf::Color::Red : sf::Color::Yellow); // Color based on whether it can be killed
        this->speed = speed;
        this->health = health;
        this->canBeKilled = canBeKilled;
    }

    void update(float dt) {
        shape.move(-speed * dt, 0); // Monsters move leftward
    }

    bool isOutOfScreen() {
        return shape.getPosition().x < -shape.getSize().x; // If off-screen
    }
};

// Main game loop
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Player with Custom Bullet Sprite");
    Ground ground(0, 500, 800, 100); // Ground position and size
    Object object(100, 450, 50); // Player object
    Gun gun("bullet_texture.png"); // Load the custom texture for bullets
    int playerScore = 0; // Player's score

    std::vector<Monster> monsters; // Correct vector initialization
    std::vector<Particle> particles; // Environment particles

    sf::Clock clock; // Delta time calculation
    sf::Clock monsterSpawnClock; // To track monster spawns
    sf::Clock particleSpawnClock; // To track particle spawns

    std::default_random_engine generator(std::time(nullptr)); // Seed for random generator
    std::uniform_real_distribution<float> speedDistribution(100.0f, 300.0f); // Monster speed distribution
    std::uniform_int_distribution<int> healthDistribution(1, 5); // Monster health distribution
    std::uniform_int_distribution<int> spawnTimeDistribution(1, 3); // Monster spawn interval
    std::uniform_real_distribution<float> particleSpawnTime(0.1f, 0.3f); // Particle spawn interval
    std::uniform_real_distribution<float> particleSpeed(150.0f, 250.0f); // Particle speed
    std::uniform_real_distribution<float> particleY(100.0f, 400.0f); // Y-coordinate range for particles

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close(); // Close the window
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) { // If left mouse button is pressed
                gun.shoot(object.shape.getPosition().x + object.shape.getSize().x, object.shape.getPosition().y + 10); // Shoot bullets with custom sprite
            }
        }

        float dt = clock.restart().asSeconds(); // Delta time for consistent updates

        // Player input handling
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            object.shape.move(-object.movementSpeed * dt, 0); // Move left
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            object.shape.move(object.movementSpeed * dt, 0); // Move right
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            object.jump(); // Jump
        }

        // Update the object and ground
        object.update(dt, ground);

        // Update the gun and bullets
        gun.update(dt); // Updates with correct iterator handling

        // Monster generation logic
        if (monsterSpawnClock.getElapsedTime().asSeconds() > spawnTimeDistribution(generator)) {
            bool canBeKilled = invincibilityDistribution(generator) == 1;
            Monster newMonster(800, 450, 50, speedDistribution(generator), healthDistribution(generator), canBeKilled);
            monsters.push_back(newMonster); // Add the new monster
            monsterSpawnClock.restart(); // Reset the spawn clock
        }

        // Monster update and collision handling
        for (auto it = monsters.begin(); it != monsters.end();) {
            it->update(dt); // Update monster position

            if (object.checkCollision(it->shape)) { // Collision with player
                object.hitCount++; // Increment hit count
                if (object.hitCount >= 3) { // Game over if hit three times
                    std::cout << "Game over! Player hit three times by monsters." << std::endl;
                    window.close();
                }
                it = monsters.erase(it); // Correctly remove the monster
            }
            else {
                if (it->canBeKilled) { // If monster can be killed
                    for (auto bullet = gun.bullets.begin(); bullet != gun.bullets.end();) {
                        if (bullet.sprite.getGlobalBounds().intersects(it->shape.getGlobalBounds())) {
                            it->health--; // Decrease monster health
                            bullet = gun.bullets.erase(bullet); // Correctly erase the bullet
                        }
                        else {
                            ++bullet; // Move to the next bullet
                        }
                    }

                    if (it->health <= 0) {
                        playerScore++; // Increment player's score
                        it = monsters.erase(it); // Correctly remove the monster
                    }
                    else {
                        ++it; // Move to the next monster
                    }
                }
                else { // If monster cannot be killed
                    if (it->isOutOfScreen()) {
                        it = monsters.erase(it); // If off-screen, remove
                    }
                    else {
                        ++it; // Move to the next monster
                    }
                }
            }
        }

        // Particle generation for environment effect
        if (particleSpawnClock.getElapsedTime().asSeconds() > particleSpawnTime(generator)) {
            // Create a new particle with a random Y-coordinate
            particles.push_back(Particle(800, particleY(generator), 5, particleSpeed(generator)));
            particleSpawnClock.restart(); // Reset the spawn clock
        }

        // Update particles and remove if off-screen
        for (auto it = particles.begin(); it != particles.end();) {
            it->update(dt); // Update the particle
            if (it.isOutOfScreen()) {
                it = particles.erase(it); // If off-screen, remove
            }
            else {
                ++it; // Move to the next particle
            }
        }

        // Draw everything
        window.clear();
        window.draw(ground.shape); // Ground
        window.draw(object.shape); // Player
        for (const auto& monster : monsters) {
            window.draw(monster.shape); // Monsters
        }
        for (const auto& bullet : gun.bullets) {
            window.draw(bullet.sprite); // Draw bullet with custom sprite
        }
        for (const auto& particle : particles) {
            window.draw(particle.shape); // Draw the particles
        }
        window.display(); // Show the updated frame
    }

    return 0;
}
