// Copyright (C) 2017 Andrea Spurio. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#ifndef JSONTYPE_KEY_HPP_
#define JSONTYPE_KEY_HPP_

#include "detail/Utility.hpp"

#define JSONTYPE_STRING(S) #S

/**
 * Declares and defines a new type 'X_tag' with a name() member that returns X
 */
#define JSONTYPE_MAKE_TAG(X)               									  \
	struct X ## _tag : Tag<X ## _tag> { static constexpr auto name() { return JSONTYPE_STRING(X); } }  \

namespace jsontype
{
	template <typename T>
	struct Tag
	{
		using type = T;
		static constexpr auto name();
	};

	namespace detail
	{
		template <typename... Args>
		struct Pack {};

		template <typename T>
		constexpr bool is_tag() { return std::is_base_of<Tag<T>, T>::value; }

		template <typename... T>
		constexpr bool is_all_tags();
	}

	template <typename... K>
	struct Key
	{
		static_assert(detail::is_all_tags<K...>(), "Keys must contain only tag classes");
		using Args = detail::Pack<K...>;
	};

	template <typename T, typename... K>
	struct Key<T, Key<K...>>
	{
		using Args = typename Key<typename T::type, typename Key<K...>::Args>::Args;
	};

	template <typename T, typename... K>
	struct Key<T, detail::Pack<K...>>
	{
		static_assert(detail::is_all_tags<T, K...>(), "Keys must contain only tag classes");
		using Args = detail::Pack<typename T::type, K...>;
	};

	template <typename T, typename... K>
	struct Key<Key<K...>, T>
	{
		using Args = typename Key<typename Key<K...>::Args, typename T::type>::Args;
	};

	template <typename T, typename... K>
	struct Key<detail::Pack<K...>, T>
	{
		static_assert(detail::is_all_tags<K..., T>(), "Keys must contain only tag classes");
		using Args = detail::Pack<K..., typename T::type>;
	};

	template <typename... K_1, typename... K_2>
	struct Key<Key<K_1...>, Key<K_2...>>
	{
		using Args = detail::Pack<K_1..., K_2...>;
	};

	template <typename... K_1, typename... K_2>
	struct Key<detail::Pack<K_1...>, detail::Pack<K_2...>>
	{
		using Args = detail::Pack<K_1..., K_2...>;
	};

	template <typename T, typename... K>
	auto operator+(T, Key<K...>) { return Key<typename T::type, Key<K...>>{}; }

	template <typename T, typename... K>
	auto operator+(Key<K...>, T) { return Key<Key<K...>, typename T::type>{}; }

	template <typename... K_1, typename... K_2>
	auto operator+(Key<K_1...>, Key<K_2...>) { return Key<Key<K_1...>, Key<K_2...>>{}; }

	//
	// Definitions
	//

	template <typename T>
	constexpr auto Tag<T>::name()
	{
		static_assert(detail::Signal_error<T>::value,
			 "Tag classes must have a name() static constexpr member returning a const char*");
	}

	namespace detail
	{
		template <typename... T>
		constexpr bool is_all_tags()
		{
		    bool pairs[] = { std::is_base_of<Tag<T>, T>::value... };
		    for (bool p: pairs)
		    {
		    	if (!p)
				{
		    		return false;
				}
		    }
		    return true;
		}
	}
}

#endif
