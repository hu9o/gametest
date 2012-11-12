#include "Controllable.h"

using namespace std;

Controllable::Controllable()
{
    //ctor
}

Controllable::~Controllable()
{
    //dtor
}

void Controllable::pressAction(Action act, bool pressed)
{
    m_actionsPressed[act] = pressed;

    actionPressed(act, pressed);
}

bool Controllable::isActionPressed(Action act)
{
    map<Action, bool>::iterator it =  m_actionsPressed.find(act);

    if (it != m_actionsPressed.end())
        return it->second;
    else
        return false;
}

void Controllable::actionPressed(Action act, bool pressed)
{
}
