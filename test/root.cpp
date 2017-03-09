#include "gtest/gtest.h"
#include "jsontype/Root.hpp"
#include <string>
#include <cstdint>
#include <iostream>
#include <utility>

using namespace jsontype;

namespace
{
	struct city_tag : Tag<city_tag> { static constexpr const auto name() { return "city"; } };
	JSONTYPE_MAKE_TAG(name);
	JSONTYPE_MAKE_TAG(state);
	JSONTYPE_MAKE_TAG(capital);
	JSONTYPE_MAKE_TAG(time);

	using City = Object<city_tag,
			Value_field<name_tag, std::string>,
			Value_field<state_tag, std::string>,
			Value_field<capital_tag, bool>>;
	using Travel = Root<City, Value_field<time_tag, int>>;
	using Travel_no_time = Root<City>;
	using Travel_time_float = Root<City, Value_field<time_tag, float>>;

	Travel travel;
}

TEST(ROOT, CONSTRUCTION)
{
	EXPECT_NO_THROW(
	{
		Travel t(travel.stringify());
	});

	EXPECT_NO_THROW(
	{
		rapidjson::Document doc;
		doc.Parse(travel.stringify());
		Travel t(std::move(doc));
	});

	EXPECT_ANY_THROW(
	{
		Travel_no_time t_no_time;
		Travel t(t_no_time.stringify());
	});

	EXPECT_ANY_THROW(
	{
		Travel_time_float t_time_float;
		Travel t(t_time_float.stringify());
	});

	EXPECT_NO_THROW(
	{
		std::string json("{\"city\":{\"name\":\"Rome\",\"state\":\"Italy\",\"capital\":true},\"time\":2}");
		Travel t(json);
	});

	EXPECT_ANY_THROW(
	{
		std::string json("{\"city\":{\"name\":\"Rome\",\"time\":4,\"capital\":true}");
		Travel t(json);
	});

	EXPECT_NO_THROW(
	{
		Travel t;
		Travel t2(std::move(t));
		Travel t3;
		t3 = std::move(t2);
	});
}

TEST(ROOT, OBJECT_SIZE)
{
	const auto travel_size = sizeof(travel);
	auto travel_ptr = &travel;
	const auto doc_size = sizeof(decltype(const_cast<const Travel*>(travel_ptr)->document()));
	EXPECT_EQ(doc_size, travel_size);
}

TEST(ROOT, STRINGIFY)
{
	std::string json("{\"city\":{\"name\":\"\",\"state\":\"\",\"capital\":false},\"time\":0}");
	EXPECT_STREQ(json.c_str(), travel.stringify().c_str());

	std::string city("{\"name\":\"\",\"state\":\"\",\"capital\":false}");
	EXPECT_STREQ(city.c_str(), travel.find(city_tag{}).stringify().c_str());
}

TEST(ROOT, MANIPULATION)
{
	using namespace std::string_literals;

	travel[time_tag{}].set(4);
	EXPECT_EQ(4, travel[time_tag{}].get());

	auto city = travel[city_tag{}];

	city[name_tag{}] = "Paris";
	city[state_tag{}] = "France";
	city[capital_tag{}] = true;
	EXPECT_EQ("Paris"s, city[name_tag{}].get());
	EXPECT_EQ("France"s, city[state_tag{}].get());
	EXPECT_EQ(true, city[capital_tag{}].get());
}

TEST(ROOT, CONST)
{
	const Travel travel;
	EXPECT_EQ(0, travel[time_tag{}]);

	EXPECT_EQ(false, travel[city_tag{}][capital_tag{}]);

	EXPECT_EQ(0, travel[Key<time_tag>{}]);

	using key = Key<city_tag, capital_tag>;
	EXPECT_EQ(false, travel[key{}]);
}

TEST(ROOT, SAME_TAG)
{
	using city = Object<city_tag, Value_field<time_tag, int>>;
	using Test = Root<Object<city_tag, city, Value_field<time_tag, int>>>;
	Test t;
	t[city_tag{}][time_tag{}] = 5;
	t[city_tag{}][city_tag{}][time_tag{}] = 10;
	EXPECT_EQ(5, t[city_tag{}][time_tag{}]);
	EXPECT_EQ(10, t[city_tag{}][city_tag{}][time_tag{}]);
}

TEST(ROOT, KEY)
{
	using namespace std::string_literals;
	Travel travel;

	using key = Key<city_tag, name_tag>;
	travel[key{}] = "Paris";
	EXPECT_EQ("Paris"s, travel[key{}].get());

	using key_city = Key<city_tag>;
	using key_capital = Key<capital_tag>;
	travel[key_city{}][key_capital{}] = true;
	EXPECT_EQ(true, travel[key_city{} + key_capital{}].get());
}

TEST(ROOT, ARRAY)
{
	JSONTYPE_MAKE_TAG(node);
	JSONTYPE_MAKE_TAG(array);
	JSONTYPE_MAKE_TAG(value);
	using Json = Root<Value_field<value_tag, unsigned>,
			Object<node_tag, Array<array_tag>>>;
 	Json json;
 	const std::string json_str = json.stringify();
 	const std::string json_expected("{\"value\":0,\"node\":{\"array\":[]}}");
 	EXPECT_EQ(json_expected, json_str);
}

TEST(ROOT, VALUE_TYPES)
{
	struct Val : Tag<Val> { static constexpr auto name() { return "val"; } };

	Root<Value_field<Val, bool>> bool_v;
	bool_v[Val{}].set(true);
	EXPECT_EQ(true, bool_v[Val{}]);

	Root<Value_field<Val, int>> int_v;
	int_v.find(Val{}).set(-56);
	EXPECT_EQ(-56, int_v.find(Val{}).get());

	Root<Value_field<Val, unsigned>> uint_v;
	uint_v[Val{}].set(22u);
	EXPECT_EQ(22u, uint_v[Val{}]);

	Root<Value_field<Val, int64_t>> int64_v;
	int64_v[Val{}].set(-9999999999);
	EXPECT_EQ(-9999999999, int64_v[Val{}].get());

	Root<Value_field<Val, uint64_t>> uint64_v;
	uint64_v[Val{}].set(9999999999u);
	EXPECT_EQ(9999999999u, uint64_v[Val{}].get());

	Root<Value_field<Val, float>> float_v;
	float_v[Val{}].set(5.45f);
	EXPECT_EQ(5.45f, float_v[Val{}].get());

	Root<Value_field<Val, double>> double_v;
	double_v[Val{}].set(6.777);
	EXPECT_EQ(6.777, double_v[Val{}].get());

	Root<Value_field<Val, std::string>> string_v;
	string_v[Val{}] = std::string("bla");
	EXPECT_EQ(std::string("bla"), string_v[Val{}].get());

	Root<Value_field<Val, const char*>> cstring_v;
	cstring_v[Val{}] = "blobloblo";
	EXPECT_STREQ("blobloblo", cstring_v[Val{}].get());
}
