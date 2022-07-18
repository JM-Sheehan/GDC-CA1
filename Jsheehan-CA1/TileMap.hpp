#include <SFML/Graphics.hpp>
#include <iostream>
#include "json.hpp"
#include <fstream>
class TileMap : public sf::Drawable, public sf::Transformable
{
public:
    sf::Vector2u m_tileSize;
    bool load(const std::string& tileset, sf::Vector2u tileSize, const int* tiles, sf::Vector2u mapSize)
    {
        // load the tileset texture
        if (!m_tileset.loadFromFile(tileset))
            return false;

        unsigned int width = mapSize.x;
        unsigned int height = mapSize.y;
        m_tileSize = tileSize;
        // resize the vertex array to fit the level size
        m_vertices.setPrimitiveType(sf::Quads);
        m_vertices.resize(width * height * 4);

        // populate the vertex array, with one quad per tile
        for (unsigned int i = 0; i < width; ++i)
            for (unsigned int j = 0; j < height; ++j)
            {
                // get the current tile number
                int tileNumber = tiles[i + j * width];

                // find its position in the tileset texture
                int tu = tileNumber % (m_tileset.getSize().x / tileSize.x);
                int tv = tileNumber / (m_tileset.getSize().x / tileSize.x);

                // get a pointer to the current tile's quad
                sf::Vertex* quad = &m_vertices[(i + j * width) * 4];

                // define its 4 corners
                quad[0].position = sf::Vector2f(i * tileSize.x, j * tileSize.y);
                quad[1].position = sf::Vector2f((i + 1) * tileSize.x, j * tileSize.y);
                quad[2].position = sf::Vector2f((i + 1) * tileSize.x, (j + 1) * tileSize.y);
                quad[3].position = sf::Vector2f(i * tileSize.x, (j + 1) * tileSize.y);

                // define its 4 texture coordinates
                quad[0].texCoords = sf::Vector2f(tu * tileSize.x, tv * tileSize.y);
                quad[1].texCoords = sf::Vector2f((tu + 1) * tileSize.x, tv * tileSize.y);
                quad[2].texCoords = sf::Vector2f((tu + 1) * tileSize.x, (tv + 1) * tileSize.y);
                quad[3].texCoords = sf::Vector2f(tu * tileSize.x, (tv + 1) * tileSize.y);
            }

        return true;
    }

    void changeTile(unsigned int tileNumber, unsigned int newTile) {
        // find tiles position in the tileset texture
        int tu = newTile % (m_tileset.getSize().x / m_tileSize.x);
        int tv = newTile / (m_tileset.getSize().x / m_tileSize.x);

        // get a pointer to the current tile's quad
        sf::Vertex* quad = &m_vertices[tileNumber * 4];

        // define its 4 texture coordinates
        quad[0].texCoords = sf::Vector2f(tu * m_tileSize.x, tv * m_tileSize.y);
        quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, tv * m_tileSize.y);
        quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, (tv + 1) * m_tileSize.y);
        quad[3].texCoords = sf::Vector2f(tu * m_tileSize.x, (tv + 1) * m_tileSize.y);
    }

    // Method to convert tileMap to JSON format for saving.
    std::vector<int> convertToMap(const int* tiles, sf::Vector2u tileSize, sf::Vector2u mapSize) {
        std::vector<int> map;
        unsigned int width = mapSize.x;
        unsigned int height = mapSize.y;
        for (unsigned int i = 0; i < width; ++i)
        {
            for (unsigned int j = 0; j < height; ++j)
            {
                // get the current tile number
                int tileNumber = tiles[i + j * width];

                // find its position in the tileset texture
                int tu = tileNumber % (m_tileset.getSize().x / tileSize.x);
                int tv = tileNumber / (m_tileset.getSize().x / tileSize.x);

                // get a pointer to the current tile's quad
                sf::Vertex* quad = &m_vertices[(i + j * width) * 4];

                auto pos = quad[0].texCoords.x;
                int vecInt = 0;
                if (pos != 0) {
                    vecInt = pos / tileSize.x;
                }
                map.push_back(vecInt);
            }
        }
        return map;
    }

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        sf::Transform transform = getTransform();
        states.transform = transform;

        // apply the tileset texture
        states.texture = &m_tileset;

        // draw the vertex array
        target.draw(m_vertices, states);
    }

    sf::VertexArray m_vertices;
    sf::Texture m_tileset;

};
