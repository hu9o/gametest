#include "Game.hpp"
#include "ResourceManager.hpp"
#include "Events.hpp"
#include "Zombie.hpp"

using namespace std;

Game::Game(sf::RenderWindow& win) : m_win(win)
{
}

Game::~Game()
{
    m_debugMode = rm::getKeyValue<bool>("g-debug");
}

void Game::run()
{
    mainLoop();
}

void Game::loadFromFile(const str& path)
{
    cout << "Chargement du niveau" << endl;

    FILE* handle;

    str fullpath(rm::getKeyValue<str>("d-maps"));
    fullpath += path;

    handle = fopen(fullpath.c_str(), "r");
    assert(handle != NULL);

    js::FileStream fstream(handle);

	assert(!json.ParseStream<0>(fstream).HasParseError());

	// TODO: fclose?

	assert(json.HasMember("name") && json["background"].IsString());
    cout << "La carte s'appelle \"" << json["name"].GetString() << "\"" << endl;

    // Clés avant le reste!
	if(json.HasMember("keys"))
    {
        js::Value keys;
        keys = json["keys"];
        str overloadable = rm::getKeyValue<str>("g-overloadable-keys-map");

        js::Value::MemberIterator itr = keys.MemberBegin();
        while(itr != keys.MemberEnd())
        {
            str name = itr->name.GetString();
            bool authorized = false;

            for (uint i=0; i<overloadable.length(); i++)
            {
                if (overloadable[i] == name[0])
                {
                    authorized = true;
                    break;
                }
            }

            if (authorized && name[1] == '-')
            {
                if (rm::hasKeyValue(name))
                {
                    rm::setKeyJsValue(name, itr->value);
                    cout << " Clé: " << name << endl;
                }
                else
                    cout << " Clé ignorée: " << name << " (inexistante dans conf.json)" << endl;
            }
            else
                cout << " Clé ignorée: " << name << " (surcharge illégale)" << endl;

            ++itr;
        }
    }
    // fin clés

    // carte
	assert(json.HasMember("tileset") && json["background"].IsString());
    rm::setTileset(json["tileset"].GetString());

	assert(json.HasMember("background") && json["background"].IsString());
	sf::Texture& bgtex = rm::getTexture(rm::getKeyValue<str>("d-backgrounds") + json["background"].GetString());
	bgtex.setRepeated(true);
	m_bg.setTexture(bgtex);

	assert(json.HasMember("tilemap"));
	m_tilemap.loadFromJson(json["tilemap"]);

	assert(json.HasMember("entities"));
}

void Game::mainLoop()
{
    cout << "Début de la boucle" << endl;

    // Réglages
    m_win.setFramerateLimit(rm::getKeyValue<int>("g-framerate-limit"));
    cout << "Début de la boucle" << endl;
    m_win.setKeyRepeatEnabled(false);
    bool framerateAdjust = rm::getKeyValue<bool>("g-framerate-adjust");
    vec2u winSize = m_win.getSize();
    // doubletaille
    sf::View view = m_win.getView();
    m_usePostFX = sf::Shader::isAvailable();
    //sf::Shader lightEffect;
    //sf::Sprite screen;

    /*if (m_usePostFX)
    {
        lightEffect.setParameter("framebuffer", sf::Shader::CurrentTexture);
        lightEffect.setParameter("color", 1.f, 1.f, 1.f);
        bool okay = lightEffect.loadFromFile(rm::getDirectory("post-fx") + "colorize.sfx", sf::Shader::Fragment);
        assert(okay);

        lightEffect.bind();
    }*/

    // Temps
    sf::Clock clock;

    // Map
    loadFromFile(rm::getKeyValue<str>("g-default-map"));

    // Joueurs et persos
    rm::createPlayers(*this, m_tilemap, m_players);

    // Zombies
    for (int i=rm::getKeyValue<int>("m-nb-zombies"); i>0; i--)
    {
        Zombie* z = new Zombie(*this);
        z->setSkin("zombie");
        z->setPosition(m_tilemap.fromTileCoords(m_tilemap.test_tombstones.front()->getPosition()));
        z->setTarget(*m_players.front()->getHuman());
    }

    // Shader
    sf::Shader blur, quake;
    if (m_usePostFX)
    {
        blur.loadFromFile(rm::getKeyValue<str>("d-resources")+"blur.frag", sf::Shader::Fragment);
        blur.setParameter("texture", sf::Shader::CurrentTexture);
        blur.setParameter("blur_radius", 0.005);

        quake.loadFromFile(rm::getKeyValue<str>("d-resources")+"wave.frag", sf::Shader::Fragment);
        quake.setParameter("texture", sf::Shader::CurrentTexture);
    }

    // Vue
    vec2i mapSize = m_tilemap.fromTileCoords(m_tilemap.getSize());
    view.move(-((int)winSize.x-mapSize.x)/2, -((int)winSize.y-mapSize.y)/2);
    view.zoom(1/rm::getKeyValue<float>("g-zoom-factor"));

    sf::RenderTexture shadowTex, screenTex;
    screenTex.create(winSize.x, winSize.y, 32);
    shadowTex.create(winSize.x, winSize.y, 32);
	m_bg.setTextureRect(sf::IntRect(0, 0, winSize.x, winSize.y));

    // Boucle
    int timeElapsed = clock.getElapsedTime().asMilliseconds();
    float frameTime;
    sf::Clock zombieClock;

    while (m_win.isOpen())
    {
        sf::Event evt;
        sf::Clock tick;


        m_win.setView(view);

        frameTime = (float)(clock.getElapsedTime().asMilliseconds() - timeElapsed) / 40;
        timeElapsed = clock.getElapsedTime().asMilliseconds();

        quake.setParameter("time", timeElapsed);

        while (m_win.pollEvent(evt))
        {
            if (evt.type == sf::Event::Closed || (evt.type == sf::Event::KeyPressed && evt.key.code == sf::Keyboard::Escape))
                return;

            if (evt.type == sf::Event::KeyPressed || evt.type == sf::Event::KeyReleased)
            {
                for (list<Player*>::iterator it = m_players.begin(); it != m_players.end(); ++it)
                {
                    (*it)->pressKey(evt.key.code, evt.type == sf::Event::KeyPressed);
                }

                if (evt.type == sf::Event::KeyReleased && rm::getKeyValue<bool>("g-debug") && evt.key.code == sf::Keyboard::Space)
                    evt::wakeTheDead(*this);
            }

            if (evt.type == sf::Event::MouseButtonPressed && rm::getKeyValue<bool>("g-debug"))
            {
                //john.goTo(vec2i(evt.mouseButton.x/32, evt.mouseButton.y/32));
                quake.setParameter("origin", (float)(evt.mouseButton.x)/1600-.25, (float)(evt.mouseButton.y)/1200-.25);

                vec2f realCoords = m_win.mapPixelToCoords(vec2i(evt.mouseButton.x, evt.mouseButton.y));
                destroyTileAt(realCoords.x, realCoords.y);
            }
        }

        m_win.clear();
        screenTex.clear();

        // Ordre d'affichage: background, backtiles, player, fronttiles, water, overlay

        screenTex.draw(m_bg);

        // Affiche la carte
        // couche 1
        m_tilemap.drawLayer(screenTex, LAYER_BACK, timeElapsed);

        // affiche les entités
        for (list<Entity*>::iterator it = m_entities.begin(); it != m_entities.end(); it++)
        {
            (*it)->update(framerateAdjust? frameTime : 1);
            (*it)->draw(screenTex, timeElapsed);
        }

        // couches 2, 3 et 4
        m_tilemap.drawLayer(screenTex, LAYER_FRONT, timeElapsed);
        m_tilemap.drawLayer(screenTex, LAYER_OVERLAY, timeElapsed);
        m_tilemap.drawLayer(screenTex, LAYER_WATER, timeElapsed);

        //sf::RenderStates states;
        //states.shader = &shader;
        shadowTex.clear(sf::Color::Transparent);
        m_tilemap.drawLayer(shadowTex, LAYER_SHADOW, timeElapsed);
        shadowTex.display();
        screenTex.draw(sf::Sprite(shadowTex.getTexture()), (m_usePostFX && rm::getKeyValue<bool>("g-smooth-shadows"))? &blur : NULL);

        m_tilemap.drawLayer(screenTex, LAYER_TEST, timeElapsed);

        screenTex.display();
        m_win.draw(sf::Sprite(screenTex.getTexture()), false? &quake : NULL);


        // Affiche l'interface

        m_win.setView(m_win.getDefaultView());
        for (std::list<Player*>::iterator it=m_players.begin(); it != m_players.end(); ++it)
        {
            (*it)->drawAndUpdateGui(m_win);
        }

        m_win.display();
    }

}

void Game::registerEntity(Entity& e)
{
    m_entities.push_back(&e);
}

const TileMap& Game::getTileMap() const
{
    return m_tilemap;
}
const std::list<Player*>& Game::getPlayers() const
{
    return m_players;
}

void Game::destroyTileAt(int x, int y)
{
    m_tilemap.destroyTileAt(x, y);
}
