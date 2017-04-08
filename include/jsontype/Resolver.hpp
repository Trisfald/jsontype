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

#ifndef JSONTYPE_RESOLVER_HPP_
#define JSONTYPE_RESOLVER_HPP_

#define RAPIDJSON_HAS_STDSTRING 1

#include <type_traits>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <utility>
#include <string>
#include <rapidjson/document.h>
#include "Key.hpp"

namespace jsontype
{
	namespace detail
	{
		template <typename Document, typename F>
		class Key_node
		{
			typedef F Func;
			typedef std::basic_string<typename Document::Ch> Key_type;
		public:
			Key_node() = default;
			Key_node(const F& func) : activable_(true), func_(func) {}

			void activate(const F& func);

			template <typename T, typename... Ts, typename std::enable_if_t<sizeof...(Ts) >= 1>* = nullptr>
			void add(const F&);

			template <typename T, typename... Ts, typename std::enable_if_t<sizeof...(Ts) == 0>* = nullptr>
			void add(const F&);

			template <typename T,
					typename... Ts,
					typename... Fargs,
					typename std::enable_if_t<sizeof...(Ts) >= 1>* = nullptr>
			auto invoke(Fargs&&...) const;

			template <typename T,
					typename... Ts,
					typename... Fargs,
					typename std::enable_if_t<sizeof...(Ts) == 0>* = nullptr>
			auto invoke(Fargs&&...) const;

			template <typename... Fargs, typename... Ts, typename std::enable_if_t<sizeof...(Ts) == 0>* = nullptr>
			auto invoke(Fargs&&...) const;

			template <typename... Fargs,
					typename U = std::result_of_t<F(Fargs...)>,
					typename std::enable_if_t<std::is_same<U, void>::value>* = nullptr>
			auto scan(const Document&, Fargs&&... fargs) const;

			template <typename... Fargs,
					typename U = std::result_of_t<F(Fargs...)>,
					typename std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
			auto scan(const Document&, Fargs&&... fargs) const;
		private:
			template <typename Json_ref,
					typename... Fargs,
					typename U = std::result_of_t<F(Fargs...)>,
					typename std::enable_if_t<std::is_same<U, void>::value>* = nullptr>
			std::result_of_t<F(Fargs...)> do_scan(const Json_ref&, bool& found, Fargs&&... fargs) const;

			template <typename Json_ref,
					typename... Fargs,
					typename U = std::result_of_t<F(Fargs...)>,
					typename std::enable_if_t<!std::is_same<U, void>::value>* = nullptr>
			std::result_of_t<F(Fargs...)> do_scan(const Json_ref&, bool& found, Fargs&&... fargs) const;

			bool activable_ = false;
			F func_;
			std::unordered_map<Key_type, std::unique_ptr<Key_node>> children_;
		};
	}

	template <typename Document, typename F>
	class Generic_resolver
	{
		typedef F Func;
		typedef std::basic_string<typename Document::Ch> String_type;
	public:
		/**
		 * Add a mapping <key, function>
		 */
		template <typename Key>
		void add(Key&&, const Func& f = Func()) { add(typename Key::Args(), f); }

		/**
		 * Invokes the function linked to the given key
		 *
		 * @throws Out_of_range if the key is not found
		 */
		template <typename Key, typename... Fargs>
		auto invoke(Key&&, Fargs&&... ar) const { return invoke(typename Key::Args(), std::forward<Fargs>(ar)...); }

		/**
		 * Scans a json and tries to invoke the best fitting function
		 *
		 * @throws Out_of_range if no matching key is not found
		 */
		template <typename... Fargs>
		auto scan(const Document& doc, Fargs&&...) const;
		/**
		 * Scans a raw string and tries to invoke the best fitting function
		 *
		 * @throws Runtime_error if the string is not a valid json, out_of_range if no matching key is not found
		 */
		template <typename... Fargs>
		auto scan(const String_type&, Fargs&&... fargs) const;
	private:
		template <typename... Args>
		void add(detail::Pack<Args...>&&, const Func&);

		template <typename... Args, typename... Fargs>
		auto invoke(detail::Pack<Args...>&&, Fargs&&...) const;

		detail::Key_node<Document, F> root_;
	};

	template <typename F>
	using Resolver = Generic_resolver<rapidjson::Document, F>;

	//
	// Definitions
	//

	template <typename Document, typename F>
	template <typename... Args>
	void Generic_resolver<Document, F>::add(detail::Pack<Args...>&&, const Func& f)
	{
		root_.template add<Args...>(f);
	}

	template <typename Document, typename F>
	template <typename... Args, typename... Fargs>
	auto Generic_resolver<Document, F>::invoke(detail::Pack<Args...>&&, Fargs&&... fargs) const
	{
		return root_.template invoke<Fargs..., Args...>(std::forward<Fargs>(fargs)...);
	}

	template <typename Document, typename F>
	template <typename... Fargs>
	auto Generic_resolver<Document, F>::scan(const Document& doc, Fargs&&... fargs) const
	{
		return root_.template scan(doc, std::forward<Fargs>(fargs)...);
	}

	template <typename Document, typename F>
	template <typename... Fargs>
	auto Generic_resolver<Document, F>::scan(const String_type& str, Fargs&&... fargs) const
	{
		Document doc;
		doc.Parse(str);
		if (!doc.IsObject())
		{
			throw std::runtime_error("Cannot parse string as json: " + str);
		}
		return scan(doc, std::forward<Fargs>(fargs)...);
	}

	namespace detail
	{
		template <typename Document, typename F>
		void Key_node<Document, F>::activate(const F& func)
		{
			func_ = func;
			activable_ = true;
		}

		template <typename Document, typename F>
		template <typename T, typename... Ts, typename std::enable_if_t<sizeof...(Ts) >= 1>*>
		void Key_node<Document, F>::add(const F& f)
		{
			auto pair = children_.emplace(std::piecewise_construct,
					 std::forward_as_tuple(T::name()),
					 std::forward_as_tuple(std::make_unique<Key_node<Document, F>>()));
			pair.first->second->template add<Ts...>(f);
		}

		template <typename Document, typename F>
		template <typename T, typename... Ts, typename std::enable_if_t<sizeof...(Ts) == 0>*>
		void Key_node<Document, F>::add(const F& f)
		{
			auto pair = children_.emplace(std::piecewise_construct,
					 std::forward_as_tuple(T::name()),
					 std::forward_as_tuple(std::make_unique<Key_node<Document, F>>()));
			pair.first->second->activate(f);
		}

		template <typename Document, typename F>
		template <typename T, typename... Ts, typename... Fargs, typename std::enable_if_t<sizeof...(Ts) >= 1>*>
		auto Key_node<Document, F>::invoke(Fargs&&... fargs) const
		{
			const auto it = children_.find(T::name());
			if (it == children_.cend())
			{
				throw std::out_of_range("Key not found!");
			}
			return it->second->template invoke<Ts...>(std::forward<Fargs>(fargs)...);
		}

		template <typename Document, typename F>
		template <typename T, typename... Ts, typename... Fargs, typename std::enable_if_t<sizeof...(Ts) == 0>*>
		auto Key_node<Document, F>::invoke(Fargs&&... fargs) const
		{
			const auto it = children_.find(T::name());
			if (it == children_.cend() || !it->second->activable_)
			{
				throw std::out_of_range("Key not found!");
			}
			return it->second->func_(std::forward<Fargs>(fargs)...);
		}

		template <typename Document, typename F>
		template <typename... Fargs, typename U, typename std::enable_if_t<std::is_same<U, void>::value>*>
		auto Key_node<Document, F>::scan(const Document& doc, Fargs&&... fargs) const
		{
			bool found = false;
			do_scan(doc, found, std::forward<Fargs>(fargs)...);
			if (!found)
			{
				throw std::out_of_range("No matching key found!");
			}
		}

		template <typename Document, typename F>
		template <typename... Fargs, typename U, typename std::enable_if_t<!std::is_same<U, void>::value>*>
		auto Key_node<Document, F>::scan(const Document& doc, Fargs&&... fargs) const
		{
			bool found = false;
			const auto ret = do_scan(doc, found, std::forward<Fargs>(fargs)...);
			if (!found)
			{
				throw std::out_of_range("No matching key found!");
			}
			return ret;
		}

		template <typename Document, typename F>
		template <typename Json_ref,
				typename... Fargs,
				typename U,
				typename std::enable_if_t<std::is_same<U, void>::value>*>
		auto Key_node<Document, F>::do_scan(const Json_ref& ref, bool& found, Fargs&&... fargs) const
				-> std::result_of_t<F(Fargs...)>
		{
			for (auto& member : ref.GetObject())
			{
				const auto it = children_.find(member.name.GetString());
				if (it == children_.cend())
				{
					continue;
				}
				bool found_child = false;
				if (member.value.IsObject())
				{
					it->second->do_scan(member.value, found_child, std::forward<Fargs>(fargs)...);
					if (found_child)
					{
						found = true;
						return;
					}
				}
				if (it->second->activable_)
				{
					found = true;
					return it->second->func_(std::forward<Fargs>(fargs)...);
				}
			}
		}

		template <typename Document, typename F>
		template <typename Json_ref,
				typename... Fargs,
				typename U,
				typename std::enable_if_t<!std::is_same<U, void>::value>*>
		auto Key_node<Document, F>::do_scan(const Json_ref& ref, bool& found, Fargs&&... fargs) const
				-> std::result_of_t<F(Fargs...)>
		{
			for (auto& member : ref.GetObject())
			{
				const auto it = children_.find(member.name.GetString());
				if (it == children_.cend())
				{
					continue;
				}
				bool found_child = false;
				if (member.value.IsObject())
				{
					const auto ret = it->second->do_scan(member.value, found_child, std::forward<Fargs>(fargs)...);
					if (found_child)
					{
						found = true;
						return ret;
					}
				}
				if (it->second->activable_)
				{
					found = true;
					return it->second->func_(std::forward<Fargs>(fargs)...);
				}
			}
			return std::result_of_t<F(Fargs...)>{};
		}
	}
}

#endif
