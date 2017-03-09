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

#ifndef JSONTYPE_DETAIL_ENCODING_TRAITS_HPP_
#define JSONTYPE_DETAIL_ENCODING_TRAITS_HPP_

#include <string>
#include "Utility.hpp"

namespace jsontype
{
	namespace detail
	{
		template <typename T>
		struct Character_traits
		{
			static_assert(Signal_error<T>::value, "No trait found for this type!");
		};

		template <>
		struct Character_traits<char>
		{
			using String_type = std::string;
		};

		template <>
		struct Character_traits<wchar_t>
		{
			using String_type = std::wstring;
		};
	}
}

#endif
