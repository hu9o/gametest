#include "ResourceManager.hpp"
#include "TileMap.hpp"

using namespace std;

namespace rm
{
    void setTheme(const std::string& name)
    {
        m_theme = name;
    }

    sf::Texture& getThemedTexture(const std::string& path)
    {
        // on va chercher le fichier dans le thème sélectionné
        sf::Texture* tex;
        string gfxDir = getKeyValue<std::string>("d-graphics");

        string filepath(gfxDir);
        filepath += m_theme + "/" + path;

        tex = _getTexture(filepath);

        if (tex == NULL)
        {
            // fichier introuvable, on va chercher dans 'default'
            string filepath = gfxDir;
            filepath += "default/" + path;

            // le fichier doit être présent dans 'default'
            tex = _getTexture(filepath);
            assert(tex != NULL);
        }

        return *tex;
    }

    sf::Texture& getTexture(const std::string& path)
    {
        // le chargement doit réussir
        sf::Texture* tex = _getTexture(path);
        assert(tex != NULL);

        return *tex;
    }

    sf::Texture* _getTexture(const std::string& path)
    {
        cout << "Demande l'image \"" << path << "\"" << endl;

        map<string, sf::Texture*>::iterator it = m_textures.find(path);

        if (it == m_textures.end())
        {
            cout << "Premier chargement" << endl;

            sf::Texture* tex = new sf::Texture();

            if (!tex->loadFromFile(path))
            {
                cout << "Introuvable, retourne NULL" << endl;
                delete tex;
                tex = NULL;
            }
            else
            {
                cout << "Image chargée" << endl;
                m_textures[path] = tex;
                tex->setSmooth(false);
            }

            return tex;
        }
        else
        {
            cout << "L'image est déjà chargée" << endl;
            return it->second;
        }
    }

    Tileset& getTileset()
    {
        return m_tileset;
    }

    void setTileset(const std::string& name)
    {
        cout << "Chargement du tileset \"" << name << "\"" << endl;
        m_tileset.loadFromFile(rm::getKeyValue<std::string>("d-tilesets") + name + ".json");
    }

    sf::Text getText(const std::string& content, int size)
    {
        sf::Text txt(content, m_font);
        txt.setCharacterSize(size);
        txt.setStyle(sf::Text::Regular);
        txt.setColor(sf::Color::Black);

        return txt;
    }

    void init()
    {
        string path = "conf.json";

        // On charge le fichier json

        // TODO: fclose?
        FILE* handle = fopen(path.c_str(), "r");
        assert(handle != NULL);

        js::FileStream fstream(handle);

        assert(!m_settings.ParseStream<0>(fstream).HasParseError());
        assert(m_settings.IsObject());

        //assert(m_settings.HasMember("directories") && m_settings["directories"].IsObject());

        // Clés
        assert(m_settings.HasMember("keys") && m_settings["keys"].IsObject());

        js::Value keys;
        keys = m_settings["keys"];

        js::Value::MemberIterator itr = keys.MemberBegin();
        while(itr != keys.MemberEnd())
        {
            setKeyJsValue(itr->name.GetString(), itr->value);
            ++itr;
        }

        // Touches
        assert(m_settings.HasMember("keylayouts") && m_settings["keylayouts"].IsObject());
    //	assert(m_settings.HasMember("keylayouts") && m_settings["keylayouts"].IsArray());

    //	js::Value keylayouts;
    //	keylayouts = m_settings["keylayouts"];
    //
    //    for (js::SizeType i = 0; i < keylayouts.Size(); i++)
    //    {
    //        KeySet keyset = {keylayouts[i]["up"].GetInt(),
    //                         keylayouts[i]["down"].GetInt(),
    //                         keylayouts[i]["left"].GetInt(),
    //                         keylayouts[i]["right"].GetInt(),
    //                         keylayouts[i]["jump"].GetInt(),
    //                         keylayouts[i]["act1"].GetInt(),
    //                         keylayouts[i]["act2"].GetInt()};
    //
    //        m_keylayouts[] = keyset;
    //
    //        ++itr;
    //
    //    }
        m_font = sf::Font();
        m_font.loadFromFile(getKeyValue<std::string>("d-resources")+getKeyValue<std::string>("g-font"));
    }

    //string getDirectory(string name)
    //{
    //    const js::Value& dirs = m_settings["directories"];
    //    const char* cname = name.c_str();
    //
    //    assert(dirs.HasMember(cname) && dirs[cname].IsString());
    //
    //    return dirs[cname].GetString();
    //}

    bool hasKeyValue(const std::string& key)
    {
        return m_keys.find(key) != m_keys.end();
    }

    js::Value& getKeyJsValue(const std::string& key)
    {
        map<string, js::Value*>::iterator it = m_keys.find(key);
        assert(it != m_keys.end());

        return *it->second;
    }

    // TODO: ATTENTION! Est-ce raisonnable de passer un pointeur sur la valeur passée? Si ça devient NULL?
    void setKeyJsValue(const std::string& key, js::Value& val)
    {
        m_keys[key] = &val;
    }

    Controls& getControlsByName(const std::string& name)
    {
        // pas déjà chargé
        if (m_controls.find(name) == m_controls.end())
        {
            const js::Value& layouts = m_settings["keylayouts"];

            assert(layouts.HasMember(name.c_str()));
            const js::Value& layout = layouts[name.c_str()];

            assert(layout.HasMember("up") && layout["up"].IsInt());
            assert(layout.HasMember("down") && layout["down"].IsInt());
            assert(layout.HasMember("left") && layout["left"].IsInt());
            assert(layout.HasMember("right") && layout["right"].IsInt());
            assert(layout.HasMember("jump") && layout["jump"].IsInt());
            assert(layout.HasMember("act1") && layout["act1"].IsInt());
            assert(layout.HasMember("act2") && layout["act2"].IsInt());

            KeySet keyset = {static_cast<key>(layout["up"].GetInt()),
                             static_cast<key>(layout["down"].GetInt()),
                             static_cast<key>(layout["left"].GetInt()),
                             static_cast<key>(layout["right"].GetInt()),
                             static_cast<key>(layout["jump"].GetInt()),
                             static_cast<key>(layout["act1"].GetInt()),
                             static_cast<key>(layout["act2"].GetInt())};

            m_controls[name] = new Controls(keyset);
        }

        return *(m_controls[name]);
    }

    void createPlayers(Game& game, TileMap& tilemap, std::list<Player*>& players)
    {
        const js::Value& pls = m_settings["players"];

        cout << "Il y a " << pls.Size() << " joueurs :" << endl;

        for (js::SizeType i=0; i<pls.Size(); ++i)
        {
            const js::Value& pl = pls[i];
            std::string type = pl["control"].GetString();
            Player* p;

            if (type == "keyboard")
            {
                p = new Player(pl["name"].GetString());
            }
            else if (type == "bot")
            {
                // à implémenter...
                continue;
            }
            else
            {
                continue;
            }

            Human* h = new Human(game);

            h->setSkin(pl["skin"].GetString());
            h->setPosition(tilemap.getStartPosition());

            p->setHuman(h);
            p->setControls(getControlsByName(pl["keys"].GetString()));

            players.push_back(p);
        }
    }

}
