#ifndef HUMAN_H
#define HUMAN_H

#include "Mob.hpp"


class Human : public Mob
{
    public:
        Human(Game& game);
        virtual ~Human();

    protected:

    private:
};


#endif // HUMAN_H
