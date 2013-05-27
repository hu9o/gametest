#include <time.h>
#include <SFML/Graphics.hpp>

#include "GameMenu.hpp"

int main()
{
    srand(time(NULL));
    sf::RenderWindow app(sf::VideoMode(800, 600), "Game");

    GameMenu menu(app);
    menu.run();

    app.close();

    return EXIT_SUCCESS;
}
