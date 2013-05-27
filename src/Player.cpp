#include "Player.hpp"

using namespace std;

Player::Player()
{
    //ctor
}

Player::~Player()
{
    //dtor
}

void Player::setControls(Controls& keys)
{
    m_keys = keys;
}

void Player::setHuman(Human* h)
{
    m_perso = h;
}

Human* Player::getHuman()
{
    return m_perso;
}

bool Player::pressKey(sf::Keyboard::Key k, bool pressed)
{
    assert(m_perso);

    Action act = m_keys.getActionFromKey(k);
    m_perso->pressAction(act, pressed);

    return act != ACT_NULL;
}
