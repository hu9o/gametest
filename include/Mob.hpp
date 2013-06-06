#ifndef MOB_H
#define MOB_H

#include "Entity.hpp"
#include "Controllable.hpp"

enum MobStatus { STAT_NULL=0, STAT_IMMUNE=0x1, STAT_DEAD=0x2, STAT_POISON=0x4, STAT_SLOW=0x8, STAT_APNEA=0x10 };

struct MobInfos
{
    int life, maxLife;
    int oxygen, maxOxygen;
    MobStatus status;
};

class Mob : public Entity, public Controllable
{
    public:
        Mob(Game& game);
        virtual ~Mob();

        virtual void draw(sf::RenderTarget& win, int elapsedTime);
        virtual void update(float frameTime);

        void setSkin(str name);
        void setPosition(vec2i pos);
        void damage(int pv);

        MobInfos getInfos() const;

    protected:
        enum State {ST_STAND, ST_JUMP, ST_CLIMB, ST_HANG, ST_DEAD};

        vec2i m_size;

        int m_life;
        int m_maxLife;
        int m_oxygen;
        int m_maxOxygen;
        int m_drowningDamage;
        bool m_canSwim;
        bool m_canHang;
        bool m_debug;

        float m_speedX;
        float m_waterSpeedCoeff;
        float m_accelX;
        float m_maxSpeedX;
        //float m_waterMaxSpeedX;
        float m_speedY;
        float m_accelY;
        float m_maxSpeedY;
        //float m_waterMaxSpeedY;
        float m_jumpStr;
        float m_waterJumpStr;
        float m_leaveWaterJumpStr;

        int m_time;

        // STANDING / FLYING / CLIMBING / SWIMMING / HANGING
        State m_state;
        MobStatus m_status;
        //bool m_onTheGround;
        //bool m_ladderState;
        bool m_inWater;
        bool m_ignoreLadder;
        //bool m_hanging;
        int m_dir;
        int m_skinVariation;
        //weapon
        //controllable
        //holdable

        virtual void actionPressed(Action act, bool pressed);
        sf::IntRect getBoundingBox();
        void die();

        inline void setStatus(MobStatus s, bool set)
        {
            if (set)
                m_status = (MobStatus)(m_status | s);
            else
                m_status = (MobStatus)(m_status & ~s);
        };
        inline bool hasStatus(MobStatus s) { return (m_status&s) != 0; }

    private:
};

#endif // MOB_H
