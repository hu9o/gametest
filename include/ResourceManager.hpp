#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "common.hpp"
#include "Tileset.hpp"
#include "Controls.hpp"
#include "Player.hpp"
#include <list>

namespace rm
{
    /*
    struct GenericKeyValue
    {
        static GenericKeyValue* newGenericKeyValue(js::Value&);
        static GenericKeyValue* newGenericKeyValue(bool);
        static GenericKeyValue* newGenericKeyValue(int);
        static GenericKeyValue* newGenericKeyValue(float);
        static GenericKeyValue* newGenericKeyValue(std::string);

        virtual bool getBool();
        int getInt();
        float getFloat();
        std::string getString();

        protected:
            GenericKeyValue();
    };

    template<typename T>
    struct KeyValue : GenericKeyValue
    {
        T value;
        KeyValue(T val) { value = val; }
        void setValue(T val) { value = val; }
        T getValue() { return value; }
    };
    */

    void init();

    void setTheme(std::string name);
    sf::Texture& getThemedTexture(std::string path);
    sf::Texture& getTexture(std::string path);

    Tileset& getTileset();
    void setTileset(std::string name);

    //std::string getDirectory(std::string name);
    bool hasKeyValue(std::string key);
    js::Value& getKeyValue(std::string key);
    void setKeyValue(std::string key, js::Value& val);

    bool getKeyValueBool(std::string key);
    int getKeyValueInt(std::string key);
    float getKeyValueFloat(std::string key);
    std::string getKeyValueString(std::string key);

    Controls& getControlsByName(std::string name);
    void createPlayers(Game& game, TileMap& tilemap, std::list<Player*>& players);


    static sf::Texture* _getTexture(std::string path);

    static std::map<std::string, sf::Texture*> m_textures;
    static std::map<std::string, js::Value*> m_keys;
    static std::map<std::string, Controls*> m_controls;
    static std::string m_theme = "gfx/default";
    static Tileset m_tileset;
    static js::Document m_settings;
}

#endif // RESOURCEMANAGER_H
