
#ifndef GAME_TEXT_HPP

#define GAME_TEXT_HPP


#include <SFML/Graphics.hpp>
#include "../include/Observer.hpp"

class GameText : public sf::Drawable, public sf::Transformable, public Subscriber{
private:
    int                         m_row{1};
    const sf::Font*             m_font{};
    sf::String                  m_string{};
    unsigned int                m_characterSize{};
    mutable sf::VertexArray     m_vertices{sf::PrimitiveType::Triangles};
    mutable bool                m_geometryNeedUpdate{true};
    float                       m_lineSpacingFactor{1.f};
    float                       m_letterSpacingFactor{1.f};
    sf::Color                   m_fillColor;
    mutable sf::FloatRect       m_bounds;

    // used to determine how many letters should be highlighted
    size_t                      coloredIndex{0};

    
    virtual void draw(sf::RenderTarget&, sf::RenderStates) const;

    void ensureGeometryUpdate() const;

public:
    GameText(sf::String, const sf::Font&, unsigned int characterSize);

    GameText(sf::String, const sf::Font&, unsigned int characterSize, int row);

    GameText() = default;

    virtual void update(const sf::String);

    bool isCompleted();

    sf::String getString();

    int getRow();

};


#endif