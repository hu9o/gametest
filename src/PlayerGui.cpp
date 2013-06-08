#include "PlayerGui.hpp"
#include "ResourceManager.hpp"

std::vector<PlayerGui*> PlayerGui::s_guis;

PlayerGui::PlayerGui(const Player& p) : m_player(p), m_playerNameText(rm::getText(p.m_name, 8))
{
    const sf::Texture& tex = rm::getTexture(rm::getKeyValue<str>("d-interface")+"playergui.png");

    m_baseSprite.setTexture(tex);
    m_baseSprite.setTextureRect(sf::IntRect(0, 0, 160, 35));
    m_healthSprite.setTexture(tex);
    m_healthSprite.setTextureRect(sf::IntRect(4, 36, 152, 8));

    m_guiIndex = s_guis.size();
    s_guis.push_back(this);

    update();
}
PlayerGui::~PlayerGui()
{
    s_guis.erase(s_guis.begin() + m_guiIndex);
}

void PlayerGui::update()
{
    if (m_player.getHuman())
    {
        MobInfos infos = m_player.getHuman()->getInfos();
        vec2i pos(200 * m_guiIndex + 20, 0);

        m_baseSprite.setPosition(pos.x, pos.y);
        m_playerNameText.setPosition(pos.x+8, pos.y+8);
        m_healthSprite.setPosition(pos.x+4, pos.y+23);

        m_healthSprite.setTextureRect(sf::IntRect(4, 36, (float)infos.life*152/infos.maxLife, 8));
    }
}

void PlayerGui::updateGuis()
{
    for (std::vector<PlayerGui*>::iterator it=s_guis.begin(); it != s_guis.end(); ++it)
    {
        (*it)->update();
    }
}
void PlayerGui::draw(sf::RenderTarget& target) const
{
    target.draw(m_baseSprite);
    target.draw(m_playerNameText);
    target.draw(m_healthSprite);

    // Affiche les "états" du jouer

    sf::Sprite m_statusSprite(*m_baseSprite.getTexture());
    sf::IntRect statusSpritePos(3, 45, 16, 16);
    MobStatus status[3] = {STAT_POISON, STAT_DEAD, STAT_APNEA};

    for (int i=0, k=0; i<3; ++i)
    {
        if (m_player.getHuman() && m_player.getHuman()->getInfos().status & status[i])
        {
            statusSpritePos.left = 3 + 19*i;
            m_statusSprite.setTextureRect(statusSpritePos);
            m_statusSprite.setPosition(200 * m_guiIndex + 20 + 140 - 19*k, 4);
            ++k;

            target.draw(m_statusSprite);
        }
    }
}
