#ifndef CONTROLLABLE_H
#define CONTROLLABLE_H

#include "Controls.h"

class Controllable
{
    public:
        Controllable();
        virtual ~Controllable();

        void pressAction(Action act, bool pressed);
        bool isActionPressed(Action act);

    protected:
        std::map<Action, bool> m_actionsPressed;

        virtual void actionPressed(Action act, bool pressed);

    private:
};

#endif // CONTROLLABLE_H
