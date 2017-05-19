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

#ifndef JSONTYPE_DETAIL_UTILITY_HPP_
#define JSONTYPE_DETAIL_UTILITY_HPP_

#include <type_traits>

namespace jsontype
{
	namespace detail
	{
		/// Type used to trigger a static assert on demand
		template <typename T> struct Signal_error : std::false_type {};

		struct No_type {};

		struct No_result {};
	}
}

#endif
