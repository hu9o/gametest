#ifndef PLAYER_H
#define PLAYER_H

#include "common.hpp"
#include "Human.hpp"
#include "Controls.hpp"
#include "PlayerGui.hpp"

class PlayerGui;
class Player
{
    public:
        Player(const str& name);
        virtual ~Player();

        void setControls(Controls& ctrls);
        void setHuman(Human* h);
        const Human* getHuman() const;
        void drawAndUpdateGui(sf::RenderTarget& target);

        bool pressKey(sf::Keyboard::Key k, bool pressed);

        friend class PlayerGui;

    protected:

        str m_name;
        int m_score;
        int team; //enum?
        int money;
        Human* m_perso;
        Controls m_ctrls;
        PlayerGui m_gui;

    private:
};

#endif // PLAYER_H
