#include "GameMenu.h"
#include "Game.h"

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
