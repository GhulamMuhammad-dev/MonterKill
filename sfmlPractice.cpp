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
        shape.setFillColor(sf::Color::Green); // Ground color
    }
};

// Player class (Object)
class Object {
public:
    sf::RectangleShape shape;
    float velocityY;
    const float gravity = 1000.0f;
    const float jumpStrength = -600.0f;
    const float movementSpeed = 200.0f;
    bool isJumping;
    int hitCount = 0;
    float health = 100.0f; // Health in percentage

    Object(float x, float y, float size) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(sf::Color::Blue);
        velocityY = 0;
        isJumping = false;
    }

    void applyGravity(float dt) {
        velocityY += gravity * dt; // Apply gravity
    }

    void jump() {
        if (!isJumping) {
            velocityY = jumpStrength;
            isJumping = true;
        }
    }

    void update(float dt, const Ground& ground) {
        applyGravity(dt); // Apply gravity effect
        shape.move(0, velocityY * dt); // Apply movement

        if (shape.getGlobalBounds().intersects(ground.shape.getGlobalBounds())) {
            shape.setPosition(shape.getPosition().x, ground.shape.getPosition().y - shape.getSize().y);
            velocityY = 0; // Reset velocity when on the ground
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
    std::vector<Bullet> bullets; // Store bullets
    const float bulletSpeed = 500.0f;

    Gun() {}

    void shoot(float x, float y) {
        bullets.push_back(Bullet(x, y, bulletSpeed)); // Create a new bullet
    }

    void update(float dt) {
        // Use an optimized loop for bullet updates
        bullets.erase(
            std::remove_if(
                bullets.begin(),
                bullets.end(),
                [dt](Bullet& bullet) {
                    bullet.update(dt);
                    return bullet.isOutOfScreen(); // Remove off-screen bullets
                }
            ),
            bullets.end()
        );
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
        shape.setFillColor(sf::Color::Yellow);
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
        shape.setFillColor(canBeKilled ? sf::Color::Red : sf::Color::Yellow);
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

// Reset the game
void resetGame(Object& object, Gun& gun, std::vector<Monster>& monsters, std::vector<Particle>& particles, int& playerScore) {
    object.shape.setPosition(100, 450);
    object.velocityY = 0;
    object.isJumping = false;
    object.hitCount = 0;
    object.health = 100.0f; // Reset health to 100%

    gun.bullets.clear(); // Clear all bullets
    monsters.clear(); // Clear all monsters
    particles.clear(); // Clear all particles
    playerScore = 0; // Reset the player's score
}

// Explicit casting to avoid data loss
std::default_random_engine generator(static_cast<unsigned int>(std::time(nullptr)));
sf::Clock monsterSpawnClock;
sf::Clock particleSpawnClock;
// Main game loop with HUD and Game Over logic
int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Game with HUD and Game Over Logic");
    Ground ground(0, 500, 800, 100); // Ground position and size
    Object object(100, 450, 50); // Player object
    Gun gun; // Gun for shooting bullets
    int playerScore = 0; // Player's score

    sf::Clock gameClock; // Correctly initialize the game clock

    std::uniform_real_distribution<float> speedDistribution(100.0f, 300.0f); // Monster speed distribution
    std::uniform_int_distribution<int> healthDistribution(1, 5); // Monster health distribution
    std::uniform_int_distribution<int> invincibilityDistribution(0, 1); // Determine if monster can be killed
    std::uniform_int_distribution<int> spawnTimeDistribution(1, 3); // Monster spawn interval

    std::uniform_real_distribution<float> particleSpawnTime(0.1f, 0.3f); // Particle spawn interval
    std::uniform_real_distribution<float> particleSpeed(150.0f, 250.0f); // Particle speed
    std::uniform_real_distribution<float> particleY(100.0f, 400.0f); // Y-coordinate range for particles

    std::vector<Monster> monsters; // Vector of monsters
    std::vector<Particle> particles; // Environment particles

    bool isGameOver = false; // To track game over state

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close(); // Close the window
            }

            if (!isGameOver && sf::Mouse::isButtonPressed(sf::Mouse::Left)) { // If left mouse button is pressed and not game over
                gun.shoot(object.shape.getPosition().x + object.shape.getSize().x, object.shape.getPosition().y + 10); // Shoot bullets
            }
        }

        float dt = gameClock.restart().asSeconds(); // Correct usage of SFML clock

        if (isGameOver) {
            // Game over screen logic
            window.clear();
            sf::Text gameOverText;
            sf::Font font;
            if (!font.loadFromFile("fonts/EvilEmpire.ttf")) {
                std::cerr << "Error loading font." << std::endl;
            }

            gameOverText.setFont(font);
            gameOverText.setString("Game Over");
            gameOverText.setCharacterSize(50);
            gameOverText.setFillColor(sf::Color::White);
            gameOverText.setPosition(300, 250); // Centered position

            window.draw(gameOverText); // Display "Game Over" message

            // Add "Play Again" and "Quit" buttons
            sf::RectangleShape playAgainButton(sf::Vector2f(100, 40));
            playAgainButton.setFillColor(sf::Color::Green);
            playAgainButton.setPosition(200, 350);
            sf::Text playAgainText("Play Again", font, 20);
            playAgainText.setFillColor(sf::Color::White);
            playAgainText.setPosition(210, 355);

            sf::RectangleShape quitButton(sf::Vector2f(100, 40));
            quitButton.setFillColor(sf::Color::Red);
            quitButton.setPosition(500, 350);
            sf::Text quitText("Quit", font, 20);
            quitText.setPosition(510, 355);

            window.draw(playAgainButton);
            window.draw(playAgainText);
            window.draw(quitButton);
            window.draw(quitText);

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) { // Handle mouse click
                auto mousePos = sf::Mouse::getPosition(window);
                auto playButtonBounds = playAgainButton.getGlobalBounds();
                auto quitButtonBounds = quitButton.getGlobalBounds();

                float mouseX = static_cast<float>(mousePos.x);
                float mouseY = static_cast<float>(mousePos.y);

                if (playButtonBounds.contains(mouseX, mouseY)) {
                    resetGame(object, gun, monsters, particles, playerScore); // Reset the game
                    isGameOver = false; // Reset the game over state
                }
                else if (quitButtonBounds.contains(mouseX, mouseY)) {
                    window.close(); // Quit the game
                }
            }

            window.display(); // Display the updated frame
            continue; // Skip the rest of the loop if game is over
        }

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
        object.update(dt, ground); // Apply gravity and movement

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
                object.health -= 33.33f; // Decrease health accordingly

                if (object.hitCount >= 3 || object.health <= 0) {
                    isGameOver = true; // Trigger game over
                    it = monsters.erase(it); // Correctly remove the monster after collision
                }
            }
            else {
                if (it->canBeKilled) { // If the monster can be killed
                    for (auto bullet = gun.bullets.begin(); bullet != gun.bullets.end();) {
                        if (bullet->shape.getGlobalBounds().intersects(it->shape.getGlobalBounds())) {
                            it->health--; // Reduce monster health
                            bullet = gun.bullets.erase(bullet); // Correctly erase the bullet

                            if (it->health <= 0) {
                                playerScore += 2; // Score increases by 2 for killing a monster
                                it = monsters.erase(it); // Correctly remove the monster
                            }
                            else {
                                ++bullet; // Continue to the next bullet
                            }
                        }
                    }
                }
                else { // If the monster cannot be killed
                    if (object.checkCollision(it->shape)) {
                        playerScore += 5; // Score increases by 5 for jumping over the unkillable monster
                    }

                    if (it->isOutOfScreen()) {
                        it = monsters.erase(it); // Correctly remove off-screen monsters
                    }
                    else {
                        ++it; // Move to the next monster
                    }
                }
            }
        }

        // Particle generation for environment effect
        if (particleSpawnClock.getElapsedTime().asSeconds() > particleSpawnTime(generator)) {
            particles.push_back(Particle(800, particleY(generator), 5, particleSpeed(generator)));
            particleSpawnClock.restart(); // Reset the spawn clock
        }

        // Update particles and remove if off-screen
        for (auto it = particles.begin(); it != particles.end();) {
            it->update(dt); // Update particle
            if (it->isOutOfScreen()) {
                it = particles.erase(it); // Correctly remove off-screen particles
            }
            else {
                ++it; // Continue to the next particle
            }
        }

        // Draw everything
        window.clear(); // Clear the window for new frame
        window.draw(ground.shape); // Ground
        window.draw(object.shape); // Player
        for (const auto& monster : monsters) {
            window.draw(monster.shape); // Monsters
        }
        for (const auto& bullet : gun.bullets) {
            window.draw(bullet.shape); // Draw bullets
        }
        for (const auto& particle : particles) {
            window.draw(particle.shape); // Environment particles
        }

        // Draw the score HUD
        sf::Font font;
        if (!font.loadFromFile("fonts/EvilEmpire.ttf")) {
            std::cerr << "Error loading font." << std::endl;
        }

        sf::Text scoreText("Score: " + std::to_string(playerScore), font, 20); // Score display
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, 10); // Top-left corner
        window.draw(scoreText);

        // Draw the health HUD
        float healthPercentage = object.health; // Health percentage
        sf::RectangleShape healthBar(sf::Vector2f(healthPercentage * 2, 10)); // Health bar width based on percentage
        healthBar.setFillColor(sf::Color::Red);
        healthBar.setPosition(680, 10); // Top-right corner
        window.draw(healthBar);

        window.display(); // Show the updated frame
    }

    return 0;
}

