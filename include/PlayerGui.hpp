#ifndef PlayerGui_H
#define PlayerGui_H

#include "common.hpp"

class Player;
class PlayerGui
{
    public:
        PlayerGui(const Player& p);
        virtual ~PlayerGui();

        void draw(sf::RenderTarget& target) const;

    protected:
        const Player& m_player;
        sf::Sprite m_baseSprite;
        sf::Text m_playerNameText;
        int m_guiIndex;

        static void updateGuis();
        static std::vector<PlayerGui*> s_guis;

    private:
};

#endif // PlayerGui_H
