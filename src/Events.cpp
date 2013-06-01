#include "Events.hpp"
#include "Zombie.hpp"

namespace evt
{
    void wakeTheDead(Game& game)
    {
        const TileMap& map = game.getTileMap();
        const std::vector<vec2i>& positions = map.test_tombstonesPositions;

        for (std::vector<vec2i>::const_iterator it=positions.begin(); it != positions.end(); ++it)
        {
            std::cout << .5 + it->x << ':' << 1. + it->y << std::endl;
            Zombie& z = * new Zombie(game);
            z.setSkin("zombie");
            z.setPosition(map.fromTileCoords(*it));
            z.setTarget(*game.getPlayers().front()->getHuman());
        }
    }

    void quakeTheEarth(Game& game)
    {
        const TileMap& map = game.getTileMap();


    }
}
