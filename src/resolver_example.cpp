#include "jsontype/Resolver.hpp"
#include "jsontype/Key.hpp"
#include <string>
#include <iostream>

using namespace jsontype;

// Create the tags
JSONTYPE_MAKE_TAG(vehicle);
JSONTYPE_MAKE_TAG(car);
JSONTYPE_MAKE_TAG(bike);
// Create the keys
using key_car = Key<vehicle_tag, car_tag>;
using key_bike = Key<vehicle_tag, bike_tag>;
// Some example inputs
constexpr auto car_json = "{\"vehicle\":{\"car\":{}}}";
constexpr auto bike_json = "{\"vehicle\":{\"bike\":{}}}";

int main()
{
	/**
	 * First example
	 */
    Resolver<int(*)(int)> resolver; // could also be parametrized with a std::function or with a functor

    resolver.add(key_car{}, [](int price) { return price * 4; });  // cars have 4 tyres
    resolver.add(key_bike{}, [](int price) { return price * 2; }); // and bikes 2

    auto total = resolver.scan(car_json, 10); // total = 40
    std::cout << "Total cost for car: " << total << "\n";

    total = resolver.scan(bike_json, 10); // total = 20
    std::cout << "Total cost for bike: " << total << "\n";



    /**
     * Second example
     */
    JSONTYPE_MAKE_TAG(sidecar);
    using key_bike_sidecar = Key<vehicle_tag, bike_tag, sidecar_tag>;
    constexpr auto bike_sidecar_json = "{\"vehicle\":{\"bike\":{\"sidecar\":{}}}}";

    struct Functor
    {
    	Functor() = default;
    	Functor(bool has_sidecar) : has_sidecar(has_sidecar) {}
    	void operator()()
    	{
    		std::cout << ((has_sidecar) ? "Bike with sidecar" : "Bike") << "\n";
    	}
    	bool has_sidecar = false;
    };

    Resolver<Functor> resolver_functor;
    resolver_functor.add(key_bike{}); // here we use the default constructed functor
    resolver_functor.add(key_bike_sidecar{}, Functor(true)); // here we give our instance

    resolver_functor.scan(bike_json);
    resolver_functor.scan(bike_sidecar_json); // the given json is more specialized and the key_bike_sidecar is selected
}
