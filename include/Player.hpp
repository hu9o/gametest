#ifndef PLAYER_H
#define PLAYER_H

#include "common.hpp"
#include "Human.hpp"
#include "Controls.hpp"

class Player
{
    public:
        Player();
        virtual ~Player();

        void setControls(Controls& ctrls);
        void setHuman(Human* h);
        Human* getHuman();

        bool pressKey(sf::Keyboard::Key k, bool pressed);

    protected:

        str m_name;
        int m_score;
        int team; //enum?
        int money;
        Human* m_perso;
        Controls m_ctrls;

    private:
};

#endif // PLAYER_H