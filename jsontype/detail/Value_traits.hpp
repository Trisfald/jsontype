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

#ifndef JSONTYPE_DETAIL_VALUE_TRAITS_HPP_
#define JSONTYPE_DETAIL_VALUE_TRAITS_HPP_

#include "Utility.hpp"

namespace jsontype
{
	namespace detail
	{
		template <typename T>
		struct Value_traits
		{
			static_assert(Signal_error<T>::value, "No trait found for this type!");

			using Param_type = No_type;

			static No_type default_value() { return No_type{}; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref&, Alloc&, const T&);

			template <typename Json_ref>
			static T get(Json_ref&);

			template <typename Json_ref>
			static bool check(Json_ref&);
		};

		template <>
		struct Value_traits<bool>
		{
			using Param_type = bool;

			static bool default_value() { return false; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc&, const bool& value) { ref.SetBool(value); }

			template <typename Json_ref>
			static bool get(Json_ref& ref) { return ref.GetBool(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsBool(); }
		};

		template <>
		struct Value_traits<int>
		{
			using Param_type = int;

			static int default_value() { return 0; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc&, const int& value) { ref.SetInt(value); }

			template <typename Json_ref>
			static int get(Json_ref& ref) { return ref.GetInt(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsInt(); }
		};

		template <>
		struct Value_traits<unsigned>
		{
			using Param_type = unsigned;

			static unsigned default_value() { return 0; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc&, const unsigned& value) { ref.SetUint(value); }

			template <typename Json_ref>
			static unsigned get(Json_ref& ref) { return ref.GetUint(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsUint(); }
		};

		template <>
		struct Value_traits<int64_t>
		{
			using Param_type = int64_t;

			static int64_t default_value() { return 0; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc&, const int64_t& value) { ref.SetInt64(value); }

			template <typename Json_ref>
			static int64_t get(Json_ref& ref) { return ref.GetInt64(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsInt64(); }
		};

		template <>
		struct Value_traits<uint64_t>
		{
			using Param_type = uint64_t;

			static uint64_t default_value() { return 0; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc&, const uint64_t& value) { ref.SetUint64(value); }

			template <typename Json_ref>
			static uint64_t get(Json_ref& ref) { return ref.GetUint64(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsUint64(); }
		};

		template <>
		struct Value_traits<float>
		{
			using Param_type = float;

			static float default_value() { return 0.0f; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc&, const float& value) { ref.SetFloat(value); }

			template <typename Json_ref>
			static float get(Json_ref& ref) { return ref.GetFloat(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsFloat(); }
		};

		template <>
		struct Value_traits<double>
		{
			using Param_type = double;

			static double default_value() { return 0.0; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc&, const double& value) { ref.SetDouble(value); }

			template <typename Json_ref>
			static double get(Json_ref& ref) { return ref.GetDouble(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsDouble(); }
		};

		template <>
		struct Value_traits<std::string>
		{
			using Param_type = const std::string&;

			static std::string default_value() { return std::string{}; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc& alloc, const std::string& value) { ref.SetString(value, alloc); }

			template <typename Json_ref>
			static std::string get(Json_ref& ref) { return ref.GetString(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsString(); }
		};

		template <>
		struct Value_traits<const char*>
		{
			using Param_type = const char*;

			static const char* default_value() { return ""; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc& alloc, const char* value) { ref.SetString(value, alloc); }

			template <typename Json_ref>
			static const char* get(Json_ref& ref) { return ref.GetString(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsString(); }
		};

		template <>
		struct Value_traits<std::wstring>
		{
			using Param_type = const std::wstring&;

			static std::wstring default_value() { return std::wstring{}; }

			template <typename Json_ref, typename Alloc>
			static void set(Json_ref& ref, Alloc& alloc, const std::wstring& value) { ref.SetString(value, alloc); }

			template <typename Json_ref>
			static std::wstring get(Json_ref& ref) { return ref.GetString(); }

			template <typename Json_ref>
			static bool check(Json_ref& ref) { return ref.IsString(); }
		};
	}
}

#endif
