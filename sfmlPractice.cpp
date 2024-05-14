#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GRAVITY = 1000.0f; // Pixels per second squared
const float JUMP_VELOCITY = -500.0f; // Pixels per second
const float MONSTER_MIN_SPEED = 100.0f; // Pixels per second
const float MONSTER_MAX_SPEED = 300.0f; // Pixels per second
const float SPAWN_INTERVAL = 2.0f; // Spawn interval in seconds

class Monster {
public:
    Monster(float x, float y, float speed) : shape(sf::Vector2f(30, 30)), velocity(-speed, 0) {
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Magenta);
    }

    void move(float deltaTime) {
        shape.move(velocity * deltaTime);
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
    }

private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
};

int main()
{
    // Create the window
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Game");

    // Create ground
    sf::RectangleShape ground(sf::Vector2f(WINDOW_WIDTH, 50));
    ground.setFillColor(sf::Color::Green);
    ground.setPosition(0, WINDOW_HEIGHT - ground.getSize().y);

    // Create players
    sf::RectangleShape player1(sf::Vector2f(50, 50));
    player1.setFillColor(sf::Color::Blue);
    player1.setPosition(50, WINDOW_HEIGHT - ground.getSize().y - player1.getSize().y);

    sf::RectangleShape player2(sf::Vector2f(50, 50));
    player2.setFillColor(sf::Color::Red);
    player2.setPosition(WINDOW_WIDTH - 100, WINDOW_HEIGHT - ground.getSize().y - player2.getSize().y);

    // AI player
    std::vector<Monster> monsters;

    // Variables to control player movement
    bool player1Jumping = false;
    float player1Velocity = 0;

    // Random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Clock for delta time calculation
    sf::Clock clock;
    sf::Clock spawnTimer;

    // Main game loop
    while (window.isOpen())
    {
        // Calculate delta time
        sf::Time deltaTime = clock.restart();

        // Event handling
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            // Jumping controls
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::W && !player1Jumping)
                {
                    player1Velocity = JUMP_VELOCITY;
                    player1Jumping = true;
                }
            }
        }

        // Apply gravity to player 1
        if (player1.getPosition().y + player1.getSize().y < WINDOW_HEIGHT - ground.getSize().y)
        {
            player1Velocity += GRAVITY * deltaTime.asSeconds();
            player1.move(0, player1Velocity * deltaTime.asSeconds());
        }
        else
        {
            player1.setPosition(player1.getPosition().x, WINDOW_HEIGHT - ground.getSize().y - player1.getSize().y);
            player1Jumping = false;
            player1Velocity = 0;
        }

        // AI player behavior - Spawn monsters gradually
        if (spawnTimer.getElapsedTime().asSeconds() >= SPAWN_INTERVAL) {
            // Spawn a monster in front of player 2
            float monsterX = player2.getPosition().x - 50; // Spawn 50 pixels in front of player 2
            float monsterY = WINDOW_HEIGHT - ground.getSize().y - 30; // Set Y position to be on the ground
            float monsterSpeed = static_cast<float>(rand()) / RAND_MAX * (MONSTER_MAX_SPEED - MONSTER_MIN_SPEED) + MONSTER_MIN_SPEED;
            monsters.emplace_back(monsterX, monsterY, monsterSpeed);

            // Reset spawn timer
            spawnTimer.restart();
        }

        // Move monsters towards player 1
        for (auto& monster : monsters) {
            monster.move(deltaTime.asSeconds());

            // Collision detection with player 1
            if (monster.getBounds().intersects(player1.getGlobalBounds())) {
                // Handle collision
                // For example, you can reduce player health or end the game
            }
        }

        // Rendering
        window.clear();
        window.draw(ground);
        window.draw(player1);
        window.draw(player2);
        for (const auto& monster : monsters) {
            monster.draw(window);
        }
        window.display();
    }

    return 0;
}
