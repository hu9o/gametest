#include "Controls.h"

using namespace std;

Controls::Controls()
{
}

Controls::Controls(KeySet keyset)
{
    m_actionsByKey[keyset.up] = ACT_UP;
    m_actionsByKey[keyset.down] = ACT_DOWN;
    m_actionsByKey[keyset.left] = ACT_LEFT;
    m_actionsByKey[keyset.right] = ACT_RIGHT;
    m_actionsByKey[keyset.jump] = ACT_JUMP;
    m_actionsByKey[keyset.act1] = ACT_1;
    m_actionsByKey[keyset.act2] = ACT_2;
}

Controls::~Controls()
{
    //dtor
}

Action Controls::getActionFromKey(key k)
{
    map<key, Action>::iterator it =  m_actionsByKey.find(k);

    if (it != m_actionsByKey.end())
        return it->second;
    else
        return ACT_NULL;
}

void Controls::setKeyForAction(Action act, key k)
{
    m_actionsByKey[k] = act;
}
