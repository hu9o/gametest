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
        static GenericKeyValue* newGenericKeyValue(str);

        virtual bool getBool();
        int getInt();
        float getFloat();
        str getString();

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

    void setTheme(const str& name);
    sf::Texture& getThemedTexture(const str& path);
    sf::Texture& getTexture(const str& path);

    Tileset& getTileset();
    void setTileset(const str& name);

    sf::Text getText(const str& content, int size=16);

    //str getDirectory(str name);
    bool hasKeyValue(const str& key);
    js::Value& getKeyJsValue(const str& key);
    void setKeyJsValue(const str& key, js::Value& val);

    template<typename T> T getKeyValue(const str& key);
    template<> inline bool getKeyValue<bool>(const str& key) { return getKeyJsValue(key).GetBool(); }
    template<> inline int getKeyValue<int>(const str& key) { return getKeyJsValue(key).GetInt(); }
    template<> inline float getKeyValue<float>(const str& key) { return getKeyJsValue(key).GetDouble(); }
    template<> inline double getKeyValue<double>(const str& key) { return getKeyJsValue(key).GetDouble(); }
    template<> inline str getKeyValue<str>(const str& key) { return getKeyJsValue(key).GetString(); }

    Controls& getControlsByName(const str& name);
    void createPlayers(Game& game, TileMap& tilemap, std::list<Player*>& players);

}

namespace
{
    sf::Texture* _getTexture(const str& path);

    std::map<str, sf::Texture*> m_textures;
    std::map<str, js::Value*> m_keys;
    std::map<str, Controls*> m_controls;
    str m_theme = "gfx/default";
    Tileset m_tileset;
    js::Document m_settings;
    sf::Font m_font;
}

#endif // RESOURCEMANAGER_H
