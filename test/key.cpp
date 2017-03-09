#include "gtest/gtest.h"
#include "jsontype/Root.hpp"
#include "jsontype/Key.hpp"
#include <type_traits>

using namespace jsontype;

JSONTYPE_MAKE_TAG(size);
JSONTYPE_MAKE_TAG(color);
JSONTYPE_MAKE_TAG(version);

using Key_1 = Key<size_tag, color_tag>;
using Key_2 = Key<version_tag>;
using Key_3 = Key<size_tag, color_tag, version_tag>;
using Key_4 = Key<version_tag, size_tag, color_tag>;

TEST(KEY, TYPE_COMPOSITION)
{
	// Compose key + tag
	{
		using Key_result = Key<Key_1, version_tag>;
		const bool same = std::is_same<Key_3::Args, Key_result::Args>::value;
		EXPECT_EQ(true, same);
	}
	// Compose tag + key
	{
		using Key_result = Key<version_tag, Key_1>;
		const bool same = std::is_same<Key_4::Args, Key_result::Args>::value;
		EXPECT_EQ(true, same);
	}
	// Compose key + key
	{
		using Key_result = Key<Key_1, Key_2>;
		const bool same = std::is_same<Key_3::Args, Key_result::Args>::value;
		EXPECT_EQ(true, same);
	}
}

TEST(KEY, OBJECT_COMPOSITION)
{
	// Compose key + tag
	{
		const bool same = std::is_same<decltype(Key_3{})::Args, decltype(Key_1{} + version_tag{})::Args>::value;
		EXPECT_EQ(true, same);
	}
	// Compose tag + key
	{
		const bool same = std::is_same<decltype(Key_4{})::Args, decltype(version_tag{} + Key_1{})::Args>::value;
		EXPECT_EQ(true, same);
	}
	// Compose key + key
	{
		const bool same = std::is_same<decltype(Key_3{})::Args, decltype(Key_1{} + Key_2{})::Args>::value;
		EXPECT_EQ(true, same);
	}
	// Compose assorted
	{
		const bool same = std::is_same
				<
					decltype(Key_4{})::Args,
					decltype(Key_2{} + size_tag{} + color_tag{})::Args
				>::value;
		EXPECT_EQ(true, same);
	}
}
