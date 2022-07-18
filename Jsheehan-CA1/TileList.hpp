#include <SFML/Graphics.hpp>
#include <iostream>
#include "json.hpp"
#include <fstream>
#include "Tile.hpp"

// Class Which contains list of tiles for mapping to sideBar.
class TileList
{
public:
    unsigned int currentType = 0;
    std::vector<Tile> tiles; //List of Tile for Sidebar
    bool load(const std::string& tileset, sf::Vector2u tileSize, unsigned int tileTypes)
    {
        m_tileTypes = tileTypes;

        if (!m_tileset.loadFromFile(tileset))
            return false;

        tiles.reserve(tileTypes);//Reserve number of spaces in tiles vector according to load method call
        for (int i = 0; i < tileTypes; i++) {
            addTile(tileSize, i);
            currentType++;
        }

        return true;
    }

    void addTile(sf::Vector2u tileSize, unsigned int tileNumber) {
        sf::Sprite sprite;
        sprite.setTexture(m_tileset);//Sets all tiles to the tileset texture
        unsigned int x = tileSize.x * tileNumber;//Sets the X coordinate for the current tile
        sprite.setTextureRect(sf::IntRect(x, 0, tileSize.x, tileSize.y));
        tiles.push_back(Tile(currentType, sprite));//Adds tile to vector.
    }


private:

    sf::Texture m_tileset;
    unsigned int m_tileTypes;
};
