#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include <vector>
#include <random>
#include <iostream>
#include <ctime>
using namespace sf;

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

// Player class
class Object {
public:
    sf::RectangleShape shape;
    float velocityY;
    const float gravity = 1000.0f;
    const float jumpStrength = -600.0f;
    const float movementSpeed = 200.0f;
    bool isJumping;
    int hitCount = 0; // Track number of times player is hit by monsters

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

// Bullet class
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
        shape.move(speed * dt, 0); // Bullets move to the right
    }

    bool isOutOfScreen() {
        return shape.getPosition().x > 800; // If off-screen to the right
    }
};

// Gun class for shooting bullets
class Gun {
public:
    std::vector<Bullet> bullets;
    const float bulletSpeed = 500.0f; // Speed of bullets

    void shoot(float x, float y) {
        bullets.push_back(Bullet(x, y, bulletSpeed)); // Create a new bullet
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

// Particle class for environment effects
class Particle {
public:
    sf::CircleShape shape;
    float speed;

    Particle(float x, float y, float radius, float speed) {
        shape.setPosition(x, y);
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::Yellow); // Yellow particles for the environment
        this->speed = speed;
    }

    void update(float dt) {
        shape.move(-speed * dt, 0); // Particles move leftward
    }

    bool isOutOfScreen() {
        return shape.getPosition().x < -2 * shape.getRadius(); // If off-screen to the left
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
        shape.setFillColor(canBeKilled ? sf::Color::Red : sf::Color::Yellow); // Red if killable, Yellow if not
        this->speed = speed;
        this->health = health;
        this->canBeKilled = canBeKilled;
    }

    void update(float dt) {
        shape.move(-speed * dt, 0); // Monsters move leftward
    }

    bool isOutOfScreen() {
        return shape.getPosition().x < -shape.getSize().x; // If off-screen to the left
    }
};

// Antidote class
class Antidote {
public:
    sf::RectangleShape shape;
    float speed;

    Antidote(float x, float y, float speed) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(20, 20)); // Antidote shape
        shape.setFillColor(sf::Color::Green);
        this->speed = speed;
    }

    void update(float dt) {
        shape.move(-speed * dt, 0); // Antidote moves leftward
    }

    bool isOutOfScreen() {
        return shape.getPosition().x < -shape.getSize().x; // If off-screen to the left
    }
};

int main() {



    SoundBuffer bulletfile;
    bulletfile.loadFromFile("audio/gunfire.wav");
    Sound bulletSound;
    bulletSound.setBuffer(bulletfile);

    SoundBuffer deadWalking;
    deadWalking.loadFromFile("audio/monsterfoot.wav");
    Sound deadWalkingSound;
    deadWalkingSound.setBuffer(deadWalking);




    sf::RenderWindow window(sf::VideoMode(800, 600), "Player with Gun, Monsters, and Environment Particles");
    Ground ground(0, 500, 800, 100); // Ground position and size
    Object object(100, 450, 50); // Player object
    Gun gun; // Gun for shooting bullets
    int playerScore = 0; // Player's score

    std::vector<Monster> monsters;
    std::vector<Particle> particles; // Environment particles
    std::vector<Antidote> antidotes; // Antidote vector
    sf::Clock clock;
    sf::Clock monsterSpawnClock; // To track monster spawns
    sf::Clock particleSpawnClock; // To track particle spawns

    std::default_random_engine generator(std::time(nullptr)); // Seed for random generator
    std::uniform_real_distribution<float> speedDistribution(100.0f, 300.0f); // Monster speed distribution
    std::uniform_int_distribution<int> healthDistribution(1, 5); // Monster health distribution
    std::uniform_int_distribution<int> spawnTimeDistribution(1, 3); // Monster spawn time
    std::uniform_int_distribution<int> invincibilityDistribution(0, 1); // Determine if monster can be killed
    std::uniform_real_distribution<float> particleSpawnTime(0.1f, 0.3f); // Particle spawn interval
    std::uniform_real_distribution<float> particleSpeed(150.0f, 250.0f); // Particle speed
    std::uniform_real_distribution<float> particleY(100.0f, 400.0f); // Y-coordinate range for particles
    std::uniform_real_distribution<float> antidoteSpawnTime(5.0f, 10.0f); // Antidote spawn interval

    sf::Font font;
    if (!font.loadFromFile("fonts/MightySouly.ttf")) {
        std::cout << "Error loading font file" << std::endl;
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10, 10);

    while (window.isOpen()) {


        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close(); // Close the window
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) { // If left mouse button is pressed
                gun.shoot(object.shape.getPosition().x + object.shape.getSize().x, object.shape.getPosition().y + 10); // Shoot bullets
                bulletSound.play();
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            object.jump(); // Jump
        }

        // Update the object and ground
        object.update(dt, ground);

        // Update the gun and bullets
        gun.update(dt);

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
                it = monsters.erase(it); // Remove monster after hitting the player
            }
            else {
                if (it->canBeKilled) { // If monster can be killed
                    for (auto bullet = gun.bullets.begin(); bullet != gun.bullets.end();) {
                        if (bullet->shape.getGlobalBounds().intersects(it->shape.getGlobalBounds())) {
                            it->health--; // Reduce monster health
                            bullet = gun.bullets.erase(bullet); // Erase bullet after hit
                        }
                        else {
                            ++bullet; // Continue to next bullet
                        }
                    }

                    if (it->health <= 0) {
                        playerScore++; // Increment player's score
                        deadWalkingSound.play();
                        it = monsters.erase(it); // Remove monster when killed
                    }
                    else {
                        ++it; // Continue to next monster
                    }
                }
                else { // If monster cannot be killed
                    if (it->isOutOfScreen()) {
                        it = monsters.erase(it); // If off-screen, remove
                    }
                    else {
                        ++it; // Continue to next monster
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
            it->update(dt); // Update particle
            if (it->isOutOfScreen()) {
                it = particles.erase(it); // If off-screen, remove
            }
            else {
                ++it; // Continue to next particle
            }
        }

        // Antidote spawn logic
        if (playerScore > 10 && antidotes.empty()) { // If player score is above 100 and no antidotes exist
            float spawnY = particleY(generator); // Random Y-coordinate for antidote spawn
            Antidote newAntidote(800, spawnY, 200); // Spawn antidote from the right side
            antidotes.push_back(newAntidote); // Add antidote to vector
        }

        // Update antidotes and remove if off-screen
        for (auto it = antidotes.begin(); it != antidotes.end();) {
            it->update(dt); // Update antidote
            if (object.checkCollision(it->shape)) { // Collision with player
                std::cout << "Player wins! Caught the antidote." << std::endl;
                window.close(); // Close the window
            }
            else if (it->isOutOfScreen()) {
                it = antidotes.erase(it); // If off-screen, remove
            }
            else {
                ++it; // Continue to next antidote
            }
        }

        // Update the score text
        scoreText.setString("Monsters killed: " + std::to_string(playerScore));

        // Draw everything
        window.clear();

        window.draw(ground.shape); // Ground
        window.draw(object.shape); // Player
        for (const auto& monster : monsters) {
            window.draw(monster.shape); // Monsters
        }
        for (const auto& bullet : gun.bullets) {
            window.draw(bullet.shape); // Bullets
        }
        for (const auto& particle : particles) {
            window.draw(particle.shape); // Environment particles
        }
        for (const auto& antidote : antidotes) {
            window.draw(antidote.shape); // Antidotes
        }
        window.draw(scoreText); // Score text
        window.display(); // Show the updated frame
    }

    return 0;
}
