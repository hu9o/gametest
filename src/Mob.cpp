#include "Mob.hpp"
#include "ResourceManager.hpp"
#include "Game.hpp"

using namespace std;

std::list<Mob*> Mob::s_mobs;

Mob::Mob(Game& game) : Entity(game)
{
    m_canSwim = rm::getKeyValue<bool>("p-can-swim");
    m_canHang = rm::getKeyValue<bool>("p-can-hang");
    m_size.x = rm::getKeyValue<int>("p-size-w");
    m_size.y = rm::getKeyValue<int>("p-size-h");
    m_maxLife = rm::getKeyValue<int>("p-life");
    m_maxOxygen = rm::getKeyValue<int>("p-oxygen");
    m_drowningDamage = rm::getKeyValue<int>("p-drown-damage");
    m_life = m_maxLife;
    m_oxygen = m_maxOxygen;
    m_speedX = 0;
    m_accelX = rm::getKeyValue<float>("p-accel-x");
    m_maxSpeedX = rm::getKeyValue<float>("p-speed-x");
    m_maxSpeedY = rm::getKeyValue<float>("p-speed-y");
    m_waterSpeedCoeff = rm::getKeyValue<float>("p-speed-water-coeff");
    //m_waterMaxSpeedX = rm::getKeyValue<float>("p-speed-water-x");
    //m_waterMaxSpeedY = rm::getKeyValue<float>("p-speed-water-y");
    m_speedY = 0;
    m_accelY = rm::getKeyValue<float>("p-accel-y");
    m_jumpStr = -rm::getKeyValue<float>("p-jump-str");
    m_waterJumpStr = -rm::getKeyValue<float>("p-jump-str-water");
    m_leaveWaterJumpStr = -rm::getKeyValue<float>("p-jump-str-leave-water");
    m_debug = rm::getKeyValue<bool>("g-debug");
    //m_onTheGround = false;
    //m_ladderState = false;
    //m_inWater = false;
    //m_ignoreLadder = false;
    //m_hanging = false;
    m_time = 0;
    m_state = ST_JUMP;
    m_status = STAT_NULL;
    m_dir = 1;
    m_skinVariation = 0;
    m_poisonTime = 0;

    s_mobs.push_back(this);
}

Mob::~Mob()
{
    s_mobs.remove(this);
}

void Mob::draw(sf::RenderTarget& win, int elapsedTime)
{
    int image = 0;
    int t = (elapsedTime % 400) / 200;

    if (m_state == ST_DEAD)
    {
        image = 8;
        m_sprite.setRotation(90);
    }
    if (m_state == ST_CLIMB)
        image = 4 + t;
    else if (m_state == ST_HANG)
        image = 6;
    else
    {
        if (m_state == ST_STAND)
        {
            image = 0 + (m_speedX)? t : 0;
        }
        else if (hasStatus(STAT_APNEA))
        {
            image = 7;
        }
        else if (m_speedY <= 0)
            image = 2;
        else if (m_speedY > 0)
            image = 3;
    }

    m_sprite.setTextureRect(sf::IntRect(image * m_size.x,
                                        m_skinVariation * m_size.y,
                                        m_size.x,
                                        m_size.y));

    m_sprite.scale((m_sprite.getScale().x == m_dir)? 1 : -1, 1.f);
    m_sprite.setOrigin(m_size.x/2, m_size.y);
    m_sprite.setPosition((int)m_pos.x, (int)m_pos.y);

    drawSprite(win);

    Entity::draw(win, elapsedTime);
}

void Mob::update(float frameTime)
{
    const TileMap& map = m_game.getTileMap();

    if (m_state == ST_DEAD)
    {/*
        m_speedY = ((m_inWater)?  m_waterJumpStr : m_jumpStr) / 2;
        m_pos.y += m_speedY * frameTime;

        m_speedY += m_accelY * (m_inWater? 0.02 : 1) * frameTime;*/
    }
    else if (m_state == ST_STAND || m_state == ST_JUMP)
    {
        // dans l'eau?
        bool wasInWater = m_inWater;
        m_inWater = map.isCollision(m_pos.x, m_pos.y - 8, TILE_WATER);

        setStatus(STAT_APNEA, m_inWater && map.isCollision(m_pos.x, m_pos.y - 14, TILE_WATER));

        // la vitesse baisse quand on arrive dans l'eau
        if (m_inWater && !wasInWater)
            m_speedY *= 0.2;

        if (!hasStatus(STAT_APNEA))
            m_oxygen = m_maxOxygen;

        // à chaque seconde
        m_time += frameTime*40;
        if (m_time >= 1000)
        {
            if (hasStatus(STAT_APNEA))
            {
                m_oxygen -= 1;
                if (m_oxygen < 0)
                {
                    m_oxygen = 0;
                    damage(m_drowningDamage);
                    if (m_state == ST_DEAD)
                        return;
                }
            }
            if (hasStatus(STAT_POISON))
            {
                damage(4);
                if (m_state == ST_DEAD)
                    return;

                if (m_poisonTime <= 0)
                    setStatus(STAT_POISON, false);
            }

            m_time -= 1000;
            m_poisonTime -= 1000;
        }

        // pics?
        if (map.isCollision(getBoundingBox(), TILE_SPIKE))
        {
            damage(m_maxLife);
            if (m_state == ST_DEAD)
                return;
        }

        // grimpe?
        if ((isActionPressed(ACT_UP)
             && map.isCollision(m_pos.x - m_size.x/2, m_pos.y - m_size.y, TILE_LADDER)
             && map.isCollision(m_pos.x + m_size.x/2, m_pos.y - m_size.y, TILE_LADDER))
          || (isActionPressed(ACT_DOWN)
              && map.isCollision(m_pos.x - m_size.x/2, m_pos.y + 1, TILE_LADDER)
              && map.isCollision(m_pos.x + m_size.x/2 , m_pos.y + 1, TILE_LADDER)))
        {
            m_state = ST_CLIMB;
            return;
        }

        // Mouvement X
        if (isActionPressed(ACT_LEFT))
        {
            m_speedX -= m_accelX * frameTime;
            m_dir = -1;
        }
        if (isActionPressed(ACT_RIGHT))
        {
            m_speedX += m_accelX * frameTime;
            m_dir = 1;
        }

        if (m_speedX > m_maxSpeedX)
            m_speedX = m_maxSpeedX;
        if (m_speedX < -m_maxSpeedX)
            m_speedX = -m_maxSpeedX;

        m_pos.x += m_speedX * (m_inWater? m_waterSpeedCoeff : 1) * frameTime;

        // collisions X
        if (map.isCollision(getBoundingBox()))
        {
            while (map.isCollision(getBoundingBox()))
            {
                m_pos.x -= (m_speedX > 0)? 0.1 : -0.1;
            }

            m_speedX = 0;
        }

        // décélération
        m_speedX -= m_speedX * 0.3 * frameTime;
        if (m_speedX < 0.2 * frameTime && m_speedX > -0.2 * frameTime)
            m_speedX = 0;

        // Mouvement Y
        m_speedY += m_accelY * (m_inWater? 0.02 : 1) * frameTime;
        if (m_speedY > m_maxSpeedY)
            m_speedY = m_maxSpeedY;

        {/*
        m_pos.y += m_speedY;

        m_onTheGround = false;

        if (map.isCollision(getBoundingBox()))
        {
            m_onTheGround = true;
            m_ignoreLadder = false;

            m_pos.y -= m_speedY;
            bool positiveSpeed = m_speedY > 0;
            short signe = positiveSpeed? 1 : -1;

            float i;
            bool addRest = true;
            for (i = m_speedY; positiveSpeed? (i >= 1) : (i <= -1); i -= signe)
            {
                if (map.isCollision(getBoundingBox()))
                {
                    addRest = false;
                    break;
                }
                m_pos.y += signe;
            }
            if (addRest)
                m_pos.y += i * signe;

            m_pos.y -= signe;
            m_speedY = 0;
        }
        */}

        m_state = ST_JUMP;
        if (m_speedY > 0)
        {
            for (float i = m_speedY * frameTime; i > 0; i -= 0.1)
            {
                m_pos.y += 0.1;

                if (map.isCollision(getBoundingBox())
                    || (map.isCollision(m_pos.x, m_pos.y-1, TILE_LADDER)
                        && !map.isCollision(m_pos.x, m_pos.y-2, TILE_LADDER)))
                {
                    m_pos.y -= 0.1;
                    m_state = ST_STAND;
                    m_ignoreLadder = false;
                    m_speedY = 0;
                    break;
                }
                else if (m_canHang)
                {
                    if ((map.isCollision(m_pos.x - m_size.x/2 - 1, m_pos.y - m_size.y + 2)
                        && !map.isCollision(m_pos.x - m_size.x/2 - 1, m_pos.y - m_size.y + 1)
                        && !map.isCollision(m_pos.x, m_pos.y + 3)
                        && m_dir == -1
                        && isActionPressed(ACT_LEFT))
                        ||
                        (map.isCollision(m_pos.x + m_size.x/2, m_pos.y - m_size.y + 2)
                        && !map.isCollision(m_pos.x + m_size.x/2, m_pos.y - m_size.y + 1)
                        && !map.isCollision(m_pos.x, m_pos.y + 3)
                        && m_dir == 1
                        && isActionPressed(ACT_RIGHT)))
                    {
                        m_state = ST_HANG;
                        m_speedY = 0;
                        break;
                    }
                }
                else if (map.isCollision(m_pos.x + m_size.x/2 + 1, m_pos.y-1)
                        )
                {
                    m_speedY = 0;
                    break;
                }
            }
        }
        else
        {
            for (float i = m_speedY * frameTime; i < 0; i += 0.1)
            {
                m_pos.y -= 0.1;

                if (map.isCollision(getBoundingBox()))
                {
                    m_pos.y += 0.1;
                    m_state = ST_STAND;
                    m_ignoreLadder = false;
                    m_speedY = 0;
                    break;
                }
            }
        }

        // collision échelles
        //if (m_state != ST_HANG && !m_ignoreLadder && map.isCollision(m_pos.x, m_pos.y, TILE_LADDER) && m_speedY > 0)
        //{
        //    m_state = ST_STAND;
        //    m_pos.y -= m_speedY;
        //    m_speedY = 0;
        //}
    }
    else if (m_state == ST_CLIMB)
    {
        m_speedY = 0;
        m_speedX = 0;
        vec2f lastpos = m_pos;

        if (isActionPressed(ACT_UP))
        {
            m_pos.y -= 1.5 * frameTime;
        }
        if (isActionPressed(ACT_DOWN))
        {
            m_pos.y += 1.5 * frameTime;
        }

        if (map.isCollision(getBoundingBox()))
            m_pos = lastpos;

        if (!map.isCollision(getBoundingBox(), TILE_LADDER)
         || (isActionPressed(ACT_DOWN) && map.isCollision(m_pos.x, m_pos.y+1, TILE_SOLID)))
            m_state = ST_JUMP;
    }

    Entity::update(frameTime);
}

void Mob::actionPressed(Action act, bool pressed)
{
    if (pressed && m_state != ST_DEAD)
    {
        if (act == ACT_JUMP && (m_state == ST_STAND || m_state == ST_HANG || (m_inWater && m_canSwim)))
        {
            m_speedY = (m_inWater)? ((!hasStatus(STAT_APNEA))? m_leaveWaterJumpStr : m_waterJumpStr) : m_jumpStr;
            m_state = ST_JUMP;
        }
        if (act == ACT_JUMP && m_state == ST_CLIMB)
        {
            m_state = ST_JUMP;
            m_ignoreLadder = true;
        }
        if (act == ACT_DOWN && m_state == ST_HANG)
            m_state = ST_JUMP;

        if (act == ACT_JUMP && m_state == ST_HANG)
            m_state = ST_JUMP;

        if (m_debug && act == ACT_1)
        {
            int pos[2] = {0, 0};

            if (isActionPressed(ACT_LEFT) && !isActionPressed(ACT_RIGHT))
            {
                pos[0] = -1;
            }
            else if (isActionPressed(ACT_RIGHT) && !isActionPressed(ACT_LEFT))
            {
                pos[0] = 1;
            }
            else if (isActionPressed(ACT_UP) && !isActionPressed(ACT_DOWN))
            {
                pos[1] = -1;
            }
            else if (isActionPressed(ACT_DOWN) && !isActionPressed(ACT_UP))
            {
                pos[1] = 1;
            }

            m_game.destroyTileAt(m_pos.x + pos[0]*16, m_pos.y-8 + pos[1]*16);
        }
    }
}

void Mob::setSkin(str name)
{
    // charge une nouvelle texture pour le sprite
    str path = rm::getKeyValue<str>("d-skins") + name + ".png";
    sf::Texture& tex = rm::getTexture(path);

    m_sprite.setTexture(tex);
    m_skinVariation = rand() % (tex.getSize().y / m_size.y);
}

void Mob::setPosition(vec2i pos)
{
    pos.x += m_size.x/2;
    pos.y += m_size.y;

    Entity::setPosition(pos);
};

void Mob::damage(int pv)
{
    m_life -= pv;

    if (m_life <= 0)
    {
        m_life = 0;
        die();
    }
    cout << "ouch! " << m_life << endl;
}

void Mob::poison(int time)
{
    m_poisonTime = time;
    setStatus(STAT_POISON, true);
}

void Mob::die()
{
    m_state = ST_DEAD;
    m_status = STAT_DEAD;
    cout << "aargh" << endl;
}

sf::IntRect Mob::getBoundingBox()
{
    return sf::IntRect(m_pos.x - m_size.x/2, m_pos.y - m_size.y, m_size.x - 1, m_size.y - 1);
}

bool Mob::isCollision(Mob& mob)
{
    sf::IntRect a = getBoundingBox();
    sf::IntRect b = mob.getBoundingBox();

    return a.left <= (b.left+b.width) && b.left <= (a.left+a.width)
        && a.top <= (b.top+b.height) && b.top <= (a.top+a.height);
}

MobInfos Mob::getInfos() const
{
    return {m_life, m_maxLife, m_oxygen, m_maxOxygen, m_status};
}
