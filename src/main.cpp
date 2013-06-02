#include <time.h>
#include <SFML/Graphics.hpp>

#include "ResourceManager.hpp"
#include "GameMenu.hpp"

int main()
{
    srand(time(NULL));
    sf::RenderWindow app(sf::VideoMode(800, 600), "Game");

    rm::init();

    GameMenu menu(app);
    menu.run();

    app.close();

    return EXIT_SUCCESS;
}
