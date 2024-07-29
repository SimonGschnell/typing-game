#ifndef GAME_TEXT_OLD_HPP
#define GAME_TEXT_OLD_HPP

#include <SFML/Graphics.hpp>
#include "include/Observer.hpp"

class GameText : public sf::Drawable, public Subscriber{

private:
    sf::Text m_coloredText{};
    sf::Text m_endingText{};
    sf::Vector2f m_pos{};

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(m_coloredText);
        target.draw(m_endingText);
    }

    void updateEndingPosition(){
        sf::FloatRect coloredTextBounds{m_coloredText.getLocalBounds()};
        m_endingText.setPosition(m_coloredText.getPosition().x + coloredTextBounds.width + m_coloredText.getLetterSpacing(), m_coloredText.getPosition().y);
    }

public:
    GameText(sf::Font &f, std::string value, sf::Vector2f position): m_pos{position}{

        m_endingText.setFont(f);
        m_endingText.setString(value);

        m_coloredText.setFont(f);
        m_coloredText.setString("");
        m_coloredText.setFillColor(sf::Color::Red);

        m_coloredText.setPosition(position);
        updateEndingPosition();
    }

    void update(sf::String character){

        sf::String endingString{m_endingText.getString()};

        if(endingString.getSize() == 0) return;


        sf::String first {endingString.substring(0,1)};
        if(character == first){
            m_coloredText.setString(m_coloredText.getString() + first);
            m_endingText.setString(endingString.substring(1,endingString.getSize()));
        }else{
            m_endingText.setString(m_coloredText.getString() + endingString);
            m_coloredText.setString("");
        }

        if(m_endingText.getString().getSize() == 0) m_coloredText.setFillColor(sf::Color::Transparent);

        updateEndingPosition();
    }

    sf::String getString() const {
        return m_coloredText.getString() + m_endingText.getString();
    }

    void move(sf::Vector2f direction){
        m_coloredText.move(direction);
        m_endingText.move(direction);
    }

};

#endif