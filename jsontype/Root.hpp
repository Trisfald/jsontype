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

#ifndef JSONTYPE_ROOT_HPP_
#define JSONTYPE_ROOT_HPP_

#define RAPIDJSON_HAS_STDSTRING 1

#include <string>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <tuple>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "Key.hpp"
#include "detail/Value_traits.hpp"
#include "detail/Encoding_traits.hpp"
#include "detail/Utility.hpp"

namespace jsontype
{
	namespace detail
	{
		struct No_name_tag : Tag<No_name_tag> { static constexpr auto name() { return "No_name"; } };
		struct Const_alloc {};

		struct Build_worker;
		struct Structure_check_worker;

		struct Finder;

		template <typename Json_ref>
		typename Character_traits<typename Json_ref::Ch>::String_type do_stringify(const Json_ref&);
	}

	template <typename Payload, typename Json_ref, typename Alloc>
	class Object_proxy;

	template <typename Payload, typename Json_ref, typename Alloc>
	class Array_proxy;

	template <typename Payload, typename Json_ref, typename Alloc>
	class Value_field_proxy;

	template <typename Document>
	class Generic_basic_root
	{
	public:
		/**
		 * @returns A json string representation of this object
		 */
		auto stringify() const { return detail::do_stringify(document_); }
	protected:
		Generic_basic_root() { document_.SetObject(); }
		Generic_basic_root(rapidjson::Document&& doc) : document_(std::move(doc)) {}
		Generic_basic_root(Generic_basic_root&&) = default;
		~Generic_basic_root() = default;
		Generic_basic_root& operator=(Generic_basic_root&&) = default;
		auto& document() { return document_; }
		const auto& document() const { return document_; }
	private:
		Document document_;
	};

	/**
	 * Root of a json entity.
	 * It maps compile times defined types over a json document; it allows concise definition of a fixed structure,
	 * easy parsing and easy access/modify of value leaves.
	 */
	template <typename Document, typename... Payloads>
	class Generic_root : private Generic_basic_root<Document>
	{
		friend struct detail::Build_worker;
		friend struct detail::Structure_check_worker;
		friend struct detail::Finder;
		using Base = Generic_basic_root<Document>;
	public:
		/**
		 * Creates a json document with all fields defaulted
		 */
		Generic_root();
		/**
		 * Creates a json document and populates all fields with the values parsed from the given json string
		 *
		 * @throws Bad_structure if the json string's structure is not compatible with this type
		 */
		explicit Generic_root(const std::basic_string<typename Document::Ch>&);
		/**
		 * Initializes a new object using the given document as its basis.
		 *
		 * @throws Bad_structure if the document's structure is not compatible with this type
		 */
		explicit Generic_root(rapidjson::Document&&);

		template <typename Name_tag>
		auto find(Name_tag);

		template <typename Name_tag>
		auto find(Name_tag) const;

		template <typename... K>
		auto find(Key<K...>);

		template <typename... K>
		auto find(Key<K...>) const;

		template <typename T>
		inline auto operator[](T);

		template <typename T>
		inline auto operator[](T) const;

		/**
		 * @returns A reference to the underlying document
		 */
		const auto& document() const { return Base::document(); }
		/**
		 * @returns A reference to the underlying rapidjson object
		 */
		const auto& ref() const { return Base::document(); }
		using Base::stringify;
	private:
		auto& document() { return Base::document(); }
		void structure_check();

		template <typename Json_ref, typename Alloc, typename F, typename T, typename... Ts>
		static void expand(Json_ref&, Alloc&, const F& = F());

		template <typename Json_ref, typename Alloc, typename F, typename... Ts>
		static auto expand(Json_ref&, Alloc&, const F& = F()) -> typename std::enable_if<sizeof...(Ts) == 0>::type {}
	};

	// Shortcut for radidjson::Document
	template <typename... Payloads>
	using Root = Generic_root<rapidjson::Document, Payloads...>;

	/**
	 * Represents a composable json object that can be either a leaf or a node in the document's hierarchy
	 */
	template <typename Name_tag = detail::No_name_tag, typename... Payloads>
	class Object
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag template argument must be a tag class");

		friend struct detail::Build_worker;
		friend struct detail::Structure_check_worker;

		template <typename Payload, typename Json_ref, typename Alloc>
		friend class Object_proxy;
	public:
		typedef Name_tag name_tag;

		template <typename... Ts>
		using Proxy_category = Object_proxy<Ts...>;

		template <typename Name_tag_search, typename Json_ref, typename Alloc>
		static auto find(Name_tag_search, Json_ref&, Alloc&);

		template <typename Json_ref, typename Alloc, typename... K>
		static auto find(Key<K...>, Json_ref&, Alloc&);
	private:
		template <typename Json_ref>
		static auto stringify(const Json_ref& ref) { return detail::do_stringify(ref); }

		template <typename Json_ref, typename Alloc>
		static void build(Json_ref&, Alloc&);

		template <typename Json_ref, typename Alloc>
		static void structure_check(Json_ref&, Alloc&);

		template <typename Json_ref, typename Alloc, typename F, typename T, typename... Ts>
		static auto expand(Json_ref&,
				Alloc&,
				const F& = F()) -> typename std::enable_if<!std::is_same<Name_tag, T>::value>::type;

		template <typename Json_ref, typename Alloc, typename F, typename T, typename... Ts>
		static auto expand(Json_ref&,
				Alloc&,
				const F& = F()) -> typename std::enable_if<std::is_same<Name_tag, T>::value>::type {}

		template <typename Json_ref, typename Alloc, typename F, typename... Ts>
		static auto expand(Json_ref&,
				Alloc&,
				const F& = F()) -> typename std::enable_if<sizeof...(Ts) == 0>::type {}
	};

	/**
	 * Represents a json array
	 */
	template <typename Name_tag = detail::No_name_tag>
	class Array
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag template argument must be a tag class");

		friend struct detail::Build_worker;
		friend struct detail::Structure_check_worker;

		template <typename Payload, typename Json_ref, typename Alloc>
		friend class Array_proxy;
	public:
		typedef Name_tag name_tag;

		template <typename... Ts>
		using Proxy_category = Array_proxy<Ts...>;
	private:
		template <typename Json_ref>
		static auto stringify(const Json_ref& ref) { return detail::do_stringify(ref); }

		template <typename Json_ref, typename Alloc>
		static void build(Json_ref&, Alloc&);

		template <typename Json_ref, typename Alloc>
		static void structure_check(Json_ref&, Alloc&);
	};

	template <typename Name_tag, typename T>
	class Basic_value_field
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag template argument must be a tag class");

		friend struct detail::Structure_check_worker;

		template <typename Payload, typename Json_ref, typename Alloc>
		friend class Value_field_proxy;
	public:
		typedef Name_tag Tag_type;
		typedef T Value_type;

		template <typename... Ts>
		using Proxy_category = Value_field_proxy<Ts...>;
	private:
		template <typename Json_ref>
		static T get(Json_ref& ref);

		template <typename Json_ref, typename Alloc>
		static void structure_check(Json_ref&, Alloc&);
	};

	/**
	 * Leaf object to which it is possible to associate a value
	 */
	template <typename Name_tag, typename T>
	class Value_field : public Basic_value_field<Name_tag, T>
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag template argument must be a tag class");

		friend struct detail::Build_worker;

		typedef typename detail::Value_traits<T>::Param_type Param_type;

		template <typename Payload, typename Json_ref, typename Alloc>
		friend class Value_field_proxy;

		template <typename Json_ref, typename Alloc>
		static void set(Json_ref&, Alloc&, Param_type);

		template <typename Json_ref, typename Alloc>
		static void build(Json_ref&, Alloc&, Param_type = detail::Value_traits<T>::default_value());
	};

	template <typename Name_tag>
	class Value_field<Name_tag, const char*> : public Basic_value_field<Name_tag, const char*>
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag template argument must be a tag class");

		friend struct detail::Build_worker;

		template <typename Payload, typename Json_ref, typename Alloc>
		friend class Value_field_proxy;

		template <typename Json_ref, typename Alloc>
		static void set(Json_ref&, Alloc&, const char*);

		template <typename Json_ref, typename Alloc>
		static void build(Json_ref&, Alloc&, const char* = "");
	};

	template <typename Payload, typename Json_ref, typename Alloc = detail::Const_alloc>
	class Base_member_proxy
	{
		using Json_ref_type = std::conditional_t<std::is_same<Alloc, detail::Const_alloc>::value,
				std::add_lvalue_reference_t<std::add_const_t<Json_ref>>,
				std::add_lvalue_reference_t<Json_ref>>;
		using Alloc_type = std::conditional_t<std::is_same<Alloc, detail::Const_alloc>::value,
				detail::Const_alloc,
				std::add_lvalue_reference_t<Alloc>>;
	public:
		Base_member_proxy(Json_ref_type& json, Alloc_type& alloc) : json_(json), alloc_(alloc) {}
		Base_member_proxy(Json_ref_type& json) : json_(json), alloc_(detail::Const_alloc{}) {}
		const auto& ref() const { return json_; }
	protected:
		~Base_member_proxy() = default;
		auto& ref() { return json_; }
		auto& alloc() { return alloc_; }
		const auto& alloc() const { return alloc_; }
	private:
		Json_ref_type& json_;
		Alloc_type alloc_;
	};

	template <typename Payload, typename Json_ref, typename Alloc = detail::Const_alloc>
	class Object_proxy : public Base_member_proxy<Payload, Json_ref, Alloc>
	{
		typedef Base_member_proxy<Payload, Json_ref, Alloc> Base;
	public:
		using Base::Base_member_proxy;

		template <typename Name_tag>
		auto find(Name_tag name_tag) { return Payload::template find(name_tag, this->ref(), this->alloc()); }

		template <typename T>
		auto operator[](T tag) { return find(tag); }

		auto stringify() const { return Payload::stringify(this->ref()); }
	};

	template <typename Payload, typename Json_ref, typename Alloc = detail::Const_alloc>
	class Array_proxy : public Base_member_proxy<Payload, Json_ref, Alloc>
	{
		typedef Base_member_proxy<Payload, Json_ref, Alloc> Base;
	public:
		using Base::Base_member_proxy;

		auto stringify() const { return Payload::stringify(this->ref()); }
	};

	template <typename Payload, typename Json_ref, typename Alloc = detail::Const_alloc>
	class Value_field_proxy : public Base_member_proxy<Payload, Json_ref, Alloc>
	{
		typedef Base_member_proxy<Payload, Json_ref, Alloc> Base;
		typedef typename detail::Value_traits<typename Payload::Value_type>::Param_type Param_type;
	public:
		typedef typename Payload::Value_type Value_type;
		using Base::Base_member_proxy;
		void set(Param_type val) { Payload::set(this->ref(), this->alloc(), val); }
		Value_type get() const { return Payload::get(this->ref()); }
		operator Value_type() const { return get(); }
		Value_field_proxy& operator=(Param_type val);
	};

	class Bad_structure : public std::runtime_error
	{
	public:
		template <typename... Args>
		Bad_structure(Args... args) : std::runtime_error(std::forward<Args>(args)...) {}
	};

	//
	// Definitions
	//

	namespace detail
	{
		template <typename Name_tag, typename... Ts> struct Payload_finder;

		template <typename Name_tag, typename T_name_tag, typename... T_extra, typename... Ts>
		struct Payload_finder<Name_tag, Object<T_name_tag, T_extra...>, Ts...>
		{
			typedef typename std::conditional<std::is_same<Name_tag, T_name_tag>::value,
					Object<T_name_tag, T_extra...>,
					typename Payload_finder<Name_tag, Ts...>::type>::type type;
		};

		template <typename Name_tag, typename T_name_tag, typename... T_extra, typename... Ts>
		struct Payload_finder<Name_tag, Value_field<T_name_tag, T_extra...>, Ts...>
		{
			typedef typename std::conditional<std::is_same<Name_tag, T_name_tag>::value,
					Value_field<T_name_tag, T_extra...>,
					typename Payload_finder<Name_tag, Ts...>::type>::type type;
		};

		template <typename Name_tag>
		struct Payload_finder<Name_tag>
		{
			typedef No_result type;
		};

		template <typename T>
		struct Member_proxy_traits
		{
			template <typename... Ts>
			using Proxy_category = typename T::template Proxy_category<Ts...>;
		};

		struct Build_worker
		{
			template <typename Owner, typename Json_ref, typename Alloc, typename T>
			void operator()(Json_ref& ref, Alloc& alloc) const
			{
				T::build(ref, alloc);
			}
		};

		struct Structure_check_worker
		{
			template <typename Owner, typename Json_ref, typename Alloc, typename T>
			void operator()(Json_ref& ref, Alloc& alloc) const
			{
				T::structure_check(ref, alloc);
			}
		};

		struct Finder
		{
			template <typename Origin, typename Json_ref, typename Alloc, typename Name_tag, typename... Payloads>
			auto operator()(Json_ref& ref, Alloc& alloc) const
			{
				using Member = typename detail::Payload_finder<Name_tag, Payloads...>::type;
				static_assert(!std::is_same<Member, No_result>::value, "Can't find any member with the given name tag");

				auto json_handle = ref.FindMember(Name_tag::name());
				assert(json_handle != ref.MemberEnd());

				using Proxy_object = typename detail::Member_proxy_traits<Member>::template Proxy_category<Member,
						decltype(json_handle->value), decltype(alloc)>;
				return Proxy_object(json_handle->value, alloc);
			}

			template <typename Origin, typename Json_ref, typename Alloc, typename Name_tag, typename... Payloads>
			auto operator()(Json_ref& ref, const Const_alloc) const
			{
				using Member = typename detail::Payload_finder<Name_tag, Payloads...>::type;
				static_assert(!std::is_same<Member, No_result>::value, "Can't find any member with the given name tag");

				auto json_handle = ref.FindMember(Name_tag::name());
				assert(json_handle != ref.MemberEnd());

				using Proxy_object = typename detail::Member_proxy_traits<Member>::template Proxy_category<Member,
						decltype(json_handle->value)>;
				return Proxy_object(json_handle->value);
			}
		};

		struct Key_unfolder
		{
			template <typename Origin, typename... Args>
			auto operator()(Origin& origin, Pack<Args...> pack) const
			{
				return do_call(origin, Args{}...);
			}

			template <typename Origin, typename Json_ref, typename Alloc, typename... Args>
			auto operator()(Json_ref& ref, Alloc& alloc, Pack<Args...> pack) const
			{
				return do_call_static<Origin>(ref, alloc, Args{}...);
			}
		private:
			template <typename Origin, typename K, typename... Ks>
			auto do_call(Origin& origin, K key, Ks... following_keys) const
			{
				auto proxy = origin.find(key);
				return do_call(proxy, std::forward<Ks>(following_keys)...);
			}

			template <typename Origin, typename K>
			auto do_call(Origin& origin, K key) const
			{
				return origin.find(key);
			}

			template <typename Origin, typename Json_ref, typename Alloc, typename K, typename... Ks>
			auto do_call_static(Json_ref& ref, Alloc& alloc, K key, Ks... following_keys) const
			{
				auto proxy = Origin::find(key, ref, alloc);
				return do_call(proxy, std::forward<Ks>(following_keys)...);
			}

			template <typename Origin, typename Json_ref, typename Alloc, typename K>
			auto do_call_static(Json_ref& ref, Alloc& alloc, K key) const
			{
				return Origin::find(key, ref, alloc);
			}
		};

		template <typename Json_ref>
		typename Character_traits<typename Json_ref::Ch>::String_type do_stringify(const Json_ref& ref)
		{
			using namespace rapidjson;
			GenericStringBuffer<typename Json_ref::EncodingType> buffer;
			Writer<decltype(buffer)> writer(buffer);
			ref.Accept(writer);
			return buffer.GetString();
		}
	}

	template <typename Document, typename... Payloads>
	Generic_root<Document, Payloads...>::Generic_root()
	{
		expand<decltype(document()),
				decltype(document().GetAllocator()),
				detail::Build_worker,
				Payloads...>(document(), document().GetAllocator());
	};

	template <typename Document, typename... Payloads>
	Generic_root<Document, Payloads...>::Generic_root(const std::basic_string<typename Document::Ch>& json)
	{
		document().Parse(json);
		structure_check();
	}

	template <typename Document, typename... Payloads>
	Generic_root<Document, Payloads...>::Generic_root(rapidjson::Document&& doc) : Base(std::move(doc))
	{
		structure_check();
	}

	template <typename Document, typename... Payloads>
	void Generic_root<Document, Payloads...>::structure_check()
	{
		if (!document().IsObject())
		{
			throw Bad_structure(std::string("Not a valid json"));
		}
		expand<decltype(document()),
				decltype(document().GetAllocator()),
				detail::Structure_check_worker,
				Payloads...>(document(), document().GetAllocator());
	}

	template <typename Document, typename... Payloads>
	template <typename Json_ref, typename Alloc, typename F, typename T, typename... Ts>
	void Generic_root<Document, Payloads...>::expand(Json_ref& ref, Alloc& alloc, const F& f)
	{
		f.template operator()<Generic_root<Payloads...>, Json_ref, Alloc, T>(ref, alloc);
		expand<Json_ref, Alloc, F, Ts...>(ref, alloc, f);
	}

	template <typename Document, typename... Payloads>
	template <typename Name_tag>
	auto Generic_root<Document, Payloads...>::find(Name_tag)
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag template argument must be a tag class");
		return detail::Finder{}.operator()<Generic_root<Payloads...>,
				decltype(document()),
				decltype(document().GetAllocator()),
				Name_tag,
				Payloads...>(document(), document().GetAllocator());
	}

	template <typename Document, typename... Payloads>
	template <typename Name_tag>
	auto Generic_root<Document, Payloads...>::find(Name_tag) const
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag template argument must be a tag class");
		return detail::Finder{}.operator()<Generic_root<Payloads...>,
				decltype(document()),
				detail::Const_alloc,
				Name_tag,
				Payloads...>(document(), detail::Const_alloc{});
	}

	template <typename Document, typename... Payloads>
	template <typename... K>
	auto Generic_root<Document, Payloads...>::find(Key<K...>)
	{
		return detail::Key_unfolder()(*this, typename Key<K...>::Args{});
	}

	template <typename Document, typename... Payloads>
	template <typename... K>
	auto Generic_root<Document, Payloads...>::find(Key<K...>) const
	{
		return detail::Key_unfolder()(*this, typename Key<K...>::Args{});
	}

	template <typename Document, typename... Payloads>
	template <typename T>
	auto Generic_root<Document, Payloads...>::operator[](T tag)
	{
		return find(tag);
	}

	template <typename Document, typename... Payloads>
	template <typename T>
	auto Generic_root<Document, Payloads...>::operator[](T tag) const
	{
		return find(tag);
	}

	template <typename Name_tag, typename T>
	template <typename Json_ref>
	T Basic_value_field<Name_tag, T>::get(Json_ref& ref)
	{
		return detail::Value_traits<T>::get(ref);
	}

	template <typename Name_tag, typename T>
	template <typename Json_ref, typename Alloc>
	void Basic_value_field<Name_tag, T>::structure_check(Json_ref& ref, Alloc&)
	{
		if (!ref.HasMember(Name_tag::name()))
		{
			throw Bad_structure(std::string("Missing value member: ") + Name_tag::name());
		}
		if (!detail::Value_traits<T>::check(ref[Name_tag::name()]))
		{
			throw Bad_structure("Value of " + std::string(Name_tag::name()) + " is of the wrong type");
		}
	}

	template <typename Name_tag, typename T>
	template <typename Json_ref, typename Alloc>
	void Value_field<Name_tag, T>::build(Json_ref& ref, Alloc& alloc, Param_type value)
	{
		ref.AddMember(rapidjson::StringRef(Name_tag::name()), value, alloc);
	}

	template <typename Name_tag, typename T>
	template <typename Json_ref, typename Alloc>
	void Value_field<Name_tag, T>::set(Json_ref& ref, Alloc& alloc, Param_type value)
	{
		detail::Value_traits<T>::set(ref, alloc, value);
	}

	template <typename Name_tag>
	template <typename Json_ref, typename Alloc>
	void Value_field<Name_tag, const char*>::build(Json_ref& ref, Alloc& alloc, const char* value)
	{
		ref.AddMember(rapidjson::StringRef(Name_tag::name()),
				rapidjson::StringRef(value),
				alloc);
	}

	template <typename Name_tag>
	template <typename Json_ref, typename Alloc>
	void Value_field<Name_tag, const char*>::set(Json_ref& ref, Alloc& alloc, const char* value)
	{
		detail::Value_traits<const char*>::set(ref, alloc, value);
	}

	template <typename Name_tag, typename... Payloads>
	template <typename Json_ref, typename Alloc>
	void Object<Name_tag, Payloads...>::build(Json_ref& ref, Alloc& alloc)
	{
		rapidjson::Value value(rapidjson::kObjectType);
		ref.AddMember(rapidjson::StringRef(Name_tag::name()), value, alloc);
		expand<Json_ref, Alloc, detail::Build_worker, Payloads...>(ref, alloc);
	}

	template <typename Name_tag, typename... Payloads>
	template <typename Json_ref, typename Alloc>
	void Object<Name_tag, Payloads...>::structure_check(Json_ref& ref, Alloc& alloc)
	{
		if (!ref.HasMember(Name_tag::name()))
		{
			throw Bad_structure(std::string("Missing object member: ") + Name_tag::name());
		}
		if (!ref[Name_tag::name()].IsObject())
		{
			throw Bad_structure(std::string(Name_tag::name()) + " is not an object");
		}
		expand<Json_ref, Alloc, detail::Structure_check_worker, Payloads...>(ref, alloc);
	}

	template <typename Name_tag, typename... Payloads>
	template <typename Json_ref, typename Alloc, typename F, typename T, typename... Ts>
	auto Object<Name_tag, Payloads...>::expand(Json_ref& ref, Alloc& alloc, const F& f)
			-> typename std::enable_if<!std::is_same<Name_tag, T>::value>::type
	{
		auto json_handle = ref.FindMember(Name_tag::name());
		assert(json_handle != ref.MemberEnd());
		f.template operator()<Object<Name_tag, Payloads...>, decltype(json_handle->value), Alloc, T>(json_handle->value,
				alloc);
		expand<Json_ref, Alloc, F, Ts...>(ref, alloc, f);
	}

	template <typename Name_tag, typename... Payloads>
	template <typename Name_tag_search, typename Json_ref, typename Alloc>
	auto Object<Name_tag, Payloads...>::find(Name_tag_search, Json_ref& ref, Alloc& alloc)
	{
		static_assert(detail::is_tag<Name_tag>(), "Name tag search template argument must be a tag class");
		return detail::Finder{}
				.operator()<Object<Name_tag, Payloads...>, Json_ref, Alloc, Name_tag_search, Payloads...>(ref, alloc);
	}

	template <typename Name_tag, typename... Payloads>
	template <typename Json_ref, typename Alloc, typename... K>
	auto Object<Name_tag, Payloads...>::find(Key<K...>, Json_ref& ref, Alloc& alloc)
	{
		return detail::Key_unfolder().operator()<Object<Name_tag, Payloads...>>(ref, alloc, typename Key<K...>::Args{});
	}

	template <typename Payload, typename Json_ref, typename Alloc>
	Value_field_proxy<Payload, Json_ref, Alloc>& Value_field_proxy<Payload, Json_ref, Alloc>::operator=(Param_type val)
	{
		set(val);
		return *this;
	}

	template <typename Name_tag>
	template <typename Json_ref, typename Alloc>
	void Array<Name_tag>::build(Json_ref& ref, Alloc& alloc)
	{
		rapidjson::Value value(rapidjson::kArrayType);
		ref.AddMember(rapidjson::StringRef(Name_tag::name()), value, alloc);
	}

	template <typename Name_tag>
	template <typename Json_ref, typename Alloc>
	void Array<Name_tag>::structure_check(Json_ref& ref, Alloc& alloc)
	{
		if (!ref.HasMember(Name_tag::name()))
		{
			throw Bad_structure(std::string("Missing array member: ") + Name_tag::name());
		}
		if (!ref[Name_tag::name()].IsArray())
		{
			throw Bad_structure(std::string(Name_tag::name()) + " is not an array");
		}
	}
}

#endif
