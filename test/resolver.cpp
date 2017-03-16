#include "gtest/gtest.h"
#include "jsontype/Root.hpp"
#include "jsontype/Resolver.hpp"
#include <string>
#include <cstdint>
#include <iostream>

using namespace jsontype;

namespace
{
	JSONTYPE_MAKE_TAG(name_0);
	JSONTYPE_MAKE_TAG(name_1);
	JSONTYPE_MAKE_TAG(name_2);
	JSONTYPE_MAKE_TAG(name_3);
	JSONTYPE_MAKE_TAG(name_4);
	JSONTYPE_MAKE_TAG(name_5);

	using key_01 = Key<name_0_tag, name_1_tag>;
	using key_012 = Key<name_0_tag, name_1_tag, name_2_tag>;
	using key_3 = Key<name_3_tag>;
	using key_412 = Key<name_4_tag, name_1_tag, name_2_tag>;
	using key_4412 = Key<name_4_tag, name_4_tag, name_1_tag, name_2_tag>;
	using key_52 = Key<name_5_tag, name_2_tag>;

	int counter = 0;

	Resolver<std::function<void()>> resolver;
}

TEST(RESOLVER, INVOKE)
{
	counter = 0;
	resolver.add(key_01{}, []{ counter += 1; });

	resolver.invoke(key_01{});

	EXPECT_EQ(1, counter);

	EXPECT_ANY_THROW(
	{
		resolver.invoke(key_012{});
	});
}

TEST(RESOLVER, SCAN)
{
	counter = 0;
	resolver.add(key_012{}, []{ counter += 2; });
	resolver.add(key_3{}, []{ counter += 3; });
	resolver.add(key_412{}, []{ counter += 4; });
	resolver.add(key_4412{}, []{ counter += 5; });
	resolver.add(key_52{}, []{ counter += 6; });

	resolver.scan("{\"name_0\":{\"name_1\":0}}");
	EXPECT_EQ(1, counter);

	resolver.scan("{\"name_0\":{\"name_1\":{\"name_2\":0}}}");
	EXPECT_EQ(3, counter);

	resolver.scan("{\"name_3\":0}");
	EXPECT_EQ(6, counter);

	resolver.scan("{\"name_4\":{\"name_1\":{\"name_2\":0}}}");
	EXPECT_EQ(10, counter);

	resolver.scan("{\"name_4\":{\"name_4\":{\"name_1\":{\"name_2\":0}}}}");
	EXPECT_EQ(15, counter);

	resolver.scan("{\"name_5\":{\"name_2\":0}}");
	EXPECT_EQ(21, counter);
}

TEST(RESOLVER, BEST_MATCH)
{
	Resolver<std::function<void()>> resolver;
	resolver.add(key_012{}, []{ counter += 2; });
	resolver.add(key_01{}, []{ counter += 1; });
	counter = 0;

	resolver.scan("{\"name_0\":{\"name_1\":0}}");
	EXPECT_EQ(1, counter);
}

TEST(RESOLVER, FUNC_ARGS)
{
	struct Work
	{
		int operator()(int i) { return i * 2; }
	};

	Resolver<Work> resolver;
	resolver.add(key_01{});

	const auto res = resolver.scan("{\"name_0\":{\"name_1\":0}}", 4);
	EXPECT_EQ(8, res);
}

