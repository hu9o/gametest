#ifndef GAMEMENU_H
#define GAMEMENU_H

#include "common.hpp"

class GameMenu
{
    public:
        GameMenu(sf::RenderWindow& win);
        virtual ~GameMenu();

        void run();

    protected:
        sf::RenderWindow& m_win;

    private:
};

#endif // GAMEMENU_H
