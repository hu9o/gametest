#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"
#include "Human.h"
#include "Controls.h"

class Player
{
    public:
        Player();
        virtual ~Player();

        void setControls(Controls& keys);
        void setHuman(Human* h);
        Human* getHuman();

        bool pressKey(sf::Keyboard::Key k, bool pressed);

    protected:

        str m_name;
        int m_score;
        int team; //enum?
        int money;
        Human* m_perso;
        Controls m_keys;

    private:
};

#endif // PLAYER_H
