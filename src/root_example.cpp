#include "jsontype/Root.hpp"
#include "jsontype/Key.hpp"
#include <string>
#include <iostream>

using namespace jsontype;

// Create name tags either with a macro or manually
JSONTYPE_MAKE_TAG(name);
JSONTYPE_MAKE_TAG(age);
JSONTYPE_MAKE_TAG(contact);
JSONTYPE_MAKE_TAG(address);
struct phone_tag : Tag<phone_tag> { static constexpr const auto name() { return "phone"; } };

// Define the json structure
using Person = Root<Value_field<name_tag, std::string>,
		Value_field<age_tag, unsigned>,
		Object<contact_tag,
				Value_field<address_tag, std::string>,
				Value_field<phone_tag, std::string>>>;

int main()
{
	Person person;
	std::cout << person.stringify() << "\n";

	// Set values
	person[name_tag{}] = "Mario";
	person[contact_tag{}][phone_tag{}] = "435425245";
	auto contact = person[contact_tag{}]; // it might be useful to use an handle to the contact object
	contact[address_tag{}] = "some street";

	// Read values
	const auto name = person[name_tag{}].get();
	const std::string phone = person[contact_tag{}][phone_tag{}]; // don't use auto here!
	std::cout << "Name: " << name << "\n";
	std::cout << "Phone: " << phone << "\n";

	using contact_key = Key<contact_tag>;
	using address_leaf_key = Key<address_tag>;
	using address_key = Key<contact_tag, address_tag>;
	using composite_key = Key<contact_key, address_leaf_key>;
	std::string address = person[contact_key{} + address_leaf_key{}];
	std::cout << "Address: " << address << "\n";
	address = person[contact_key{} + address_tag{}];
	std::cout << "Address: " << address << "\n";
	address = person[address_key{}];
	std::cout << "Address: " << address << "\n";
	address = person[composite_key{}];
	std::cout << "Address: " << address << "\n";
}
