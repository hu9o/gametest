#include "GameMenu.hpp"
#include "Game.hpp"

GameMenu::GameMenu(sf::RenderWindow& win) : m_win(win)
{
    //ctor
}

GameMenu::~GameMenu()
{
    //dtor
}

void GameMenu::run()
{
    Game game(m_win);
    game.run();
}
