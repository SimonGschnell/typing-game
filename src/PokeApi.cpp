#include "../include/PokeApi.hpp"

namespace PokeApi{
    std::pair<std::string,int> getPokemon(int id){
        try
        {
            // That's all that is needed to do cleanup of used resources (RAII style).
            curlpp::Cleanup myCleanup;

            // Our request to be sent.
            curlpp::Easy myRequest;

            // Set the URL.
            myRequest.setOpt<Url>("https://pokeapi.co/api/v2/pokemon/"+std::to_string(id));

            // Send request and get a result.
            // By default the result goes to standard output.

            std::ostringstream os;
            curlpp::options::WriteStream ws(&os);
            myRequest.setOpt(ws);
            myRequest.perform();

            auto data = json::parse(os.str());
            std::cout << "fetched pokemon: Name => " << data["name"] << " | id => " << data["id"] << std::endl;
            return {data["name"],data["id"]};
        }

        catch(curlpp::RuntimeError & e)
        {
            std::cout << e.what() << std::endl;
        }

        catch(curlpp::LogicError & e)
        {
            std::cout << e.what() << std::endl;
        }

        return {"fail", -1};
    }
}