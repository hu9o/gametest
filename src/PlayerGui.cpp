#include "PlayerGui.hpp"
#include "ResourceManager.hpp"

std::vector<PlayerGui*> PlayerGui::s_guis;

PlayerGui::PlayerGui(const Player& p) : m_player(p), m_playerNameText(rm::getText(p.m_name, 8))
{
    m_baseSprite.setTexture(rm::getTexture(rm::getKeyValue<std::string>("d-interface")+"PlayerGui.png"));

    m_guiIndex = s_guis.size();
    s_guis.push_back(this);
    updateGuis();
}
PlayerGui::~PlayerGui()
{
    s_guis.erase(s_guis.begin() + m_guiIndex);
}

void PlayerGui::updateGuis()
{
    for (std::vector<PlayerGui*>::iterator it=s_guis.begin(); it != s_guis.end(); ++it)
    {
        (*it)->m_baseSprite.setPosition(200 * (*it)->m_guiIndex + 20, 0);
        (*it)->m_playerNameText.setPosition(200 * (*it)->m_guiIndex + 28, 8);
    }
}
void PlayerGui::draw(sf::RenderTarget& target) const
{
    target.draw(m_baseSprite);
    target.draw(m_playerNameText);
}
