#ifndef CONTROLS_H
#define CONTROLS_H

#include "common.hpp"

typedef sf::Keyboard::Key key;

enum Action
{
    ACT_NULL,
    ACT_UP,
    ACT_DOWN,
    ACT_LEFT,
    ACT_RIGHT,
    ACT_JUMP,
    ACT_1,
    ACT_2
};

struct KeySet
{
    key up;
    key down;
    key left;
    key right;
    key jump;
    key act1;
    key act2;
};

class Controls
{
    public:
        Controls();
        Controls(KeySet keyset);
        virtual ~Controls();

        std::map<key, Action> m_actionsByKey;

        void setKeyForAction(Action act, key k);
        Action getActionFromKey(key k);

    protected:
    private:
};

#endif // CONTROLS_H
