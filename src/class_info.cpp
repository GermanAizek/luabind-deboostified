// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#define LUABIND_BUILDING

#include <luabind/lua_include.hpp>

#include <luabind/luabind.hpp>
#include <luabind/class_info.hpp>

/*
#include <iostream>
#define VERBOSE(X) std::cout << __FILE__ << ":" << __LINE__ << ": " << X << std::endl
*/
#define VERBOSE(X)

namespace luabind {

	LUABIND_API class_info get_class_info(argument const& o)
	{
		lua_State* L = o.interpreter();
		detail::class_rep * crep = NULL;

		o.push(L);
		if(detail::is_class_rep(L, -1)) {
			VERBOSE("OK, got a class rep");
			// OK, o is a class rep, now at the top of the stack
			crep = static_cast<detail::class_rep *>(lua_touserdata(L, -1));
			lua_pop(L, 1);
		} else {

			VERBOSE("Not a class rep");
			detail::object_rep* obj = detail::get_instance(L, -1);

			if(!obj)
			{
				VERBOSE("Not a obj rep");
				class_info result;
				result.name = lua_typename(L, lua_type(L, -1));
				lua_pop(L, 1);
				result.methods = newtable(L);
				result.attributes = newtable(L);
				return result;
			} else {
				lua_pop(L, 1);
				// OK, we were given an object - gotta get the crep.
				crep = obj->crep();
			}
		}
		crep->get_table(L);
		object table(from_stack(L, -1));
		lua_pop(L, 1);

		class_info result;
		result.name = crep->name();
		result.methods = newtable(L);
		result.attributes = newtable(L);

		std::size_t index = 1;

		for(iterator i(table), e; i != e; ++i)
		{
			if(type(*i) != LUA_TFUNCTION)
				continue;

			// We have to create a temporary `object` here, otherwise the proxy
			// returned by operator->() will mess up the stack. This is a known
			// problem that probably doesn't show up in real code very often.
			object member(*i);
			member.push(L);
			detail::stack_pop pop(L, 1);

			if(lua_tocfunction(L, -1) == &detail::property_tag)
			{
				result.attributes[index++] = i.key();
			} else
			{
				result.methods[i.key()] = *i;
			}
		}

		return result;
	}

	LUABIND_API object get_class_names(lua_State* L)
	{
		detail::class_registry* reg = detail::class_registry::get_registry(L);

		luabind::map<type_id, detail::class_rep*> const& classes = reg->get_classes();

		object result = newtable(L);
		std::size_t index = 1;

		for(const auto& cl : classes) {
			result[index++] = cl.second->name();
		}

		return result;
	}

	LUABIND_API void bind_class_info(lua_State* L)
	{
		module(L)
			[
				class_<class_info>("class_info_data")
				.def_readonly("name", &class_info::name)
			.def_readonly("methods", &class_info::methods)
			.def_readonly("attributes", &class_info::attributes),

			def("class_info", &get_class_info),
			def("class_names", &get_class_names)
			];
	}
}

