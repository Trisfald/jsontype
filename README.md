# jsontype

Jsontype is a tiny library for managing json in a structured way. It's built on top of rapidjson, which is used to handle the json creation and parsing.


### Prerequisites
- Include [rapidjson](https://github.com/miloyip/rapidjson)
- Use a C++14 compiler
    
    
### Installation
Add the jsontype directory in your includes. The library is header only!  
Tests use the [google test framework](https://github.com/google/googletest). Examples are in the src directory and don't use any extra dependency.  

    
### How to use
Suppose we want to map a json with the following structure into a type:

```
{
	"name": "Paul",
	"age": 20,
	"contact": {
		"address": "74 Green St",
		"phone": "564565132"
	}
}
```

We can do it in this way:

```C++
// Create tags either with a macro or manually
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
```

Now we can create instances of this json by simply creating an object. All values are initially default constructed, in fact printing the object's string representation gives us:

```
{"name":"","age":0,"contact":{"address":"","phone":""}}
```

### Parsing and move initialization
It's possible to initialize a root object with an already existent rapidjson document or with a string. The library will check if the given json has a structure compatible with the one embedded in the object's type: it can have additional elements but all the specified ones must be present and of the right kind. When a string is passed in to the constructor, parsing will be done automatically.


### Finding and manipulating nodes
A jsontype object can be navigated by using the get() member function or the operator[], passing a tag instance. A value field's current value can be read or changed via member functions and operators.

```C++
// Set values
person[name_tag{}] = "Mario";
person[contact_tag{}][phone_tag{}] = "435425245";
auto contact = person[contact_tag{}]; // it might be useful to use an handle to the contact object
contact[address_tag{}] = "some street";

// Read values
const auto name = person[name_tag{}].get();
std::string phone = person[contact_tag{}][phone_tag{}]; // don't use auto here!
```


### Keys
Keys are used to identify and access nodes; they can be added together or bundled in a new type. Here are presented some valid ways to retrieve the address from our json:

```C++
using contact_key = Key<contact_tag>;
using address_leaf_key = Key<address_tag>;
using address_key = Key<contact_tag, address_tag>;
using composite_key = Key<contact_key, address_leaf_key>;

std::string address = person[contact_key{} + address_leaf_key{}];
address = person[contact_key{} + address_tag{}];
address = person[address_key{}];
address = person[composite_key{}];
```





















