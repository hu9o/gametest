#include "Player.hpp"

using namespace std;

Player::Player(const std::string& name) : m_name(name), m_gui(*this)
{
    //ctor
}

Player::~Player()
{
    //dtor
}

void Player::setControls(Controls& ctrls)
{
    m_ctrls = ctrls;
}

void Player::setHuman(Human* h)
{
    m_perso = h;
}

const Human* Player::getHuman() const
{
    return m_perso;
}

void Player::drawGui(sf::RenderTarget& target) const
{
    return m_gui.draw(target);
}

bool Player::pressKey(sf::Keyboard::Key k, bool pressed)
{
    assert(m_perso);

    Action act = m_ctrls.getActionFromKey(k);
    m_perso->pressAction(act, pressed);

    return act != ACT_NULL;
}

