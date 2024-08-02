#ifndef PUB_SUB_HPP
#define PUB_SUB_HPP

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>

class Subscriber{
    public:
        Subscriber() = default;
        virtual void update(sf::String) = 0;
};

class Publisher{
private:
    std::unordered_map<std::string, std::shared_ptr<Subscriber>> m_subscribers{};
public:
    Publisher() = default;

    void subscribe(const sf::String &key, std::shared_ptr<Subscriber> s){
        // store subscriber in the hashmap
        m_subscribers[key.toAnsiString()]=s;
    }

    void unsubscribe(const std::string &key){
        // remove the subscriber from the hashmap
        m_subscribers.erase(key);
    }

    void notify_subscribers(const sf::String &value){
        // call the update function of every subscriber in the hashmap
        for(std::pair<const std::string, std::shared_ptr<Subscriber>> &s : m_subscribers){
            s.second->update(value);
        }
    }

    auto getSubscribers(){
        return m_subscribers;
    }
};

#endif