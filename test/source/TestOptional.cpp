/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "EASTLTest.h"
#include <EASTL/type_traits.h>
#include <EASTL/sort.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/optional.h>
#include <EASTL/unique_ptr.h>


/////////////////////////////////////////////////////////////////////////////
struct IntStruct
{
	IntStruct(int in) : data(in) {}
	int data;
};

bool operator<(const IntStruct& lhs, const IntStruct& rhs)
	{ return lhs.data < rhs.data; }
bool operator==(const IntStruct& lhs, const IntStruct& rhs)
	{ return lhs.data == rhs.data; }


/////////////////////////////////////////////////////////////////////////////
struct trivial_test
{
	trivial_test() { constructor_ran = true; }
	trivial_test(const trivial_test&) { constructor_ran = true; }
	trivial_test(trivial_test&&) { constructor_ran = true; was_moved = true; }
	trivial_test& operator =(const trivial_test&) { assignment_ran = true; return *this; }
	trivial_test& operator =(trivial_test&&) { assignment_ran = true; was_moved = true; return *this; }
	static bool constructor_ran;
	static bool assignment_ran;
	static bool was_moved;
	static void reset()
	{
		constructor_ran = false;
		assignment_ran = false;
		was_moved = false;
	}
};
bool trivial_test::constructor_ran = false;
bool trivial_test::assignment_ran = false;
bool trivial_test::was_moved = false;

/////////////////////////////////////////////////////////////////////////////
struct non_trivial_test
{
	non_trivial_test() { constructor_ran = true; }
	non_trivial_test(const non_trivial_test&) { constructor_ran = true; }
	non_trivial_test(non_trivial_test&&) { constructor_ran = true; was_moved = true; }
	non_trivial_test& operator =(const non_trivial_test&) { assignment_ran = true; return *this; }
	non_trivial_test& operator =(non_trivial_test&&) { assignment_ran = true; was_moved = true; return *this; }
	virtual ~non_trivial_test() { destructor_ran = true; }
	static bool destructor_ran;
	static bool constructor_ran;
	static bool assignment_ran;
	static bool was_moved;
	static void reset()
	{
		destructor_ran = false;
		constructor_ran = false;
		assignment_ran = false;
		was_moved = false;
	}
};
bool non_trivial_test::constructor_ran = false;
bool non_trivial_test::assignment_ran = false;
bool non_trivial_test::was_moved = false;
bool non_trivial_test::destructor_ran = false;

/////////////////////////////////////////////////////////////////////////////
struct destructor_test
{
	~destructor_test() { destructor_ran = true; }
	static bool destructor_ran;
	static void reset() { destructor_ran = false; }
};
bool destructor_test::destructor_ran = false;

/////////////////////////////////////////////////////////////////////////////
struct move_test
{
    move_test() = default;
	move_test(move_test&& other)      { was_moved = true; }
	move_test& operator=(move_test&&) { was_moved = true; return *this;}

	// issue a compiler error is container tries to copy
    move_test(move_test const &other)  = delete;
	move_test& operator=(const move_test&) = delete;  

    static bool was_moved;
};

bool move_test::was_moved = false;

/////////////////////////////////////////////////////////////////////////////
template <typename T>
class forwarding_test
{
	eastl::optional<T> m_optional;

public:
	forwarding_test() : m_optional() {}
	forwarding_test(T&& t) : m_optional(t) {}
	~forwarding_test() { m_optional.reset(); }

	template <typename U>
	T GetValueOrDefault(U&& def) const
	{
		return m_optional.value_or(eastl::forward<U>(def));
	}
};

/////////////////////////////////////////////////////////////////////////////
// TestOptional
//
int TestOptional()
{
	using namespace eastl;
	int nErrorCount = 0;
	#if defined(EASTL_OPTIONAL_ENABLED) && EASTL_OPTIONAL_ENABLED
	{
		{
			VERIFY( (is_same<optional<int>::value_type, int>::value));
			VERIFY( (is_same<optional<short>::value_type, short>::value));
			VERIFY(!(is_same<optional<short>::value_type, long>::value));
			VERIFY( (is_same<optional<const short>::value_type, const short>::value));
			VERIFY( (is_same<optional<volatile short>::value_type, volatile short>::value));
			VERIFY( (is_same<optional<const volatile short>::value_type, const volatile short>::value));

			VERIFY(is_empty<nullopt_t>::value);
			#if EASTL_TYPE_TRAIT_is_literal_type_CONFORMANCE
				VERIFY(is_literal_type<nullopt_t>::value);
			#endif

			#if EASTL_TYPE_TRAIT_is_trivially_destructible_CONFORMANCE
				VERIFY(is_trivially_destructible<int>::value);
				VERIFY(is_trivially_destructible<Internal::optional_storage<int>>::value);
				VERIFY(is_trivially_destructible<optional<int>>::value);
				VERIFY(is_trivially_destructible<optional<int>>::value == is_trivially_destructible<int>::value);
			#endif

			{
				struct NotTrivialDestructible { ~NotTrivialDestructible() {} };
				VERIFY(!is_trivially_destructible<NotTrivialDestructible>::value);
				VERIFY(!is_trivially_destructible<optional<NotTrivialDestructible>>::value);
				VERIFY(!is_trivially_destructible<Internal::optional_storage<NotTrivialDestructible>>::value);
				VERIFY(is_trivially_destructible<optional<NotTrivialDestructible>>::value == is_trivially_destructible<NotTrivialDestructible>::value);
			}
		}

		{
			optional<int> o;  
			VERIFY(!o);
			VERIFY(o.value_or(0x8BADF00D) == (int)0x8BADF00D);
			o = 1024;
			VERIFY(static_cast<bool>(o));
			VERIFY(o.value_or(0x8BADF00D) == 1024);
			VERIFY(o.value() == 1024);
			
			// Test reset
			o.reset();
			VERIFY(!o);
			VERIFY(o.value_or(0x8BADF00D) == (int)0x8BADF00D);
		}

		{
			optional<int> o(nullopt);  
			VERIFY(!o);
			VERIFY(o.value_or(0x8BADF00D) == (int)0x8BADF00D);
		}

		{
			optional<int> o = {};  
			VERIFY(!o);
			VERIFY(o.value_or(0x8BADF00D) == (int)0x8BADF00D);
		}

		{
			optional<int> o(42);  
			VERIFY(bool(o));
			VERIFY(o.value_or(0x8BADF00D) == 42);
			o = nullopt;
			VERIFY(!o);
			VERIFY(o.value_or(0x8BADF00D) == (int)0x8BADF00D);
		}

		{
			optional<int> o(42);
			VERIFY(static_cast<bool>(o));
			VERIFY(o.value_or(0x8BADF00D) == 42);
			VERIFY(o.value() == 42);
		}

		{
			auto o = make_optional(42);
			VERIFY((is_same<decltype(o), optional<int>>::value));
			VERIFY(static_cast<bool>(o));
			VERIFY(o.value_or(0x8BADF00D) == 42);
			VERIFY(o.value() == 42);
		}

		{
			int a = 42;
			auto o = make_optional(a);
			VERIFY((is_same<decltype(o)::value_type, int>::value));
			VERIFY(o.value() == 42);
		}

		{
			// test make_optional stripping refs/cv-qualifers
			int a = 42;
			const volatile int& intRef = a;
			auto o = make_optional(intRef);
			VERIFY((is_same<decltype(o)::value_type, int>::value));
			VERIFY(o.value() == 42);
		}

		{
			int a = 10;
			const volatile int& aRef = a;
			auto o = eastl::make_optional(aRef);
			VERIFY(o.value() == 10);
		}

		{
			{
				struct local { int payload1; };
				auto o = eastl::make_optional<local>(42);
				VERIFY(o.value().payload1 == 42);
			}
			{
				struct local { int payload1; int payload2; };
				auto o = eastl::make_optional<local>(42, 43);
				VERIFY(o.value().payload1 == 42);
				VERIFY(o.value().payload2 == 43);
			}

			{
				struct local
				{
					local(std::initializer_list<int> ilist)
					{
						payload1 = ilist.begin()[0];
						payload2 = ilist.begin()[1];
					}

					int payload1;
					int payload2;
				};

				auto o = eastl::make_optional<local>({42, 43});
				VERIFY(o.value().payload1 == 42);
				VERIFY(o.value().payload2 == 43);
			}
		}

		{
			optional<int> o1(42), o2(24);
			VERIFY(o1.value() == 42);
			VERIFY(o2.value() == 24);
			VERIFY(*o1 == 42);
			VERIFY(*o2 == 24);
			o1 = eastl::move(o2);
			VERIFY(*o2 == 24);
			VERIFY(*o1 == 24);
			VERIFY(o2.value() == 24);
			VERIFY(o1.value() == 24);
			VERIFY(bool(o1));
			VERIFY(bool(o2));
		}

		{
			struct local { int payload; };
			optional<local> o = local{ 42 }; 
			VERIFY(o->payload == 42);
		}

		{
			struct local
			{
				int test() const { return 42; }
			};

			{
				const optional<local> o = local{};
				VERIFY(o->test() == 42);
				VERIFY((*o).test() == 42);
				VERIFY(o.value().test() == 42);
				VERIFY(bool(o));
			}

			{
				optional<local> o = local{};
				VERIFY(bool(o));
				o = nullopt;
				VERIFY(!bool(o));

				VERIFY(o.value_or(local{}).test() == 42);
				VERIFY(!bool(o));
			}
		}
	}

    {
        move_test t;
        optional<move_test> o(eastl::move(t));
        VERIFY(move_test::was_moved);
    }

	{
        forwarding_test<float>ft(1.f);
        float val = ft.GetValueOrDefault(0.f);
        VERIFY(val == 1.f);
	}

	#if EASTL_VARIADIC_TEMPLATES_ENABLED 
	{
		struct vec3
		{
			vec3(std::initializer_list<float> ilist) { auto* p = ilist.begin();  x = *p++; y = *p++; z = *p++; }
			vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}  // testing variadic template constructor overload 
			float x = 0, y = 0, z = 0;
		};

		{
			optional<vec3> o{ in_place, 4.f, 5.f, 6.f };
			VERIFY(o->x == 4 && o->y == 5 && o->z == 6);
		}

		{
			optional<vec3> o{ in_place, {4.f, 5.f, 6.f} };
			VERIFY(o->x == 4 && o->y == 5 && o->z == 6);
		}

		{
			optional<string> o(in_place, {'a', 'b', 'c'});
			VERIFY(o == string("abc"));
		}

		// http://en.cppreference.com/w/cpp/utility/optional/emplace
		{
			optional<vec3> o;
			o.emplace(42.f, 42.f, 42.f);
			VERIFY(o->x == 42.f && o->y == 42.f && o->z == 42.f);
		}

		{
			optional<vec3> o;
			o.emplace({42.f, 42.f, 42.f});
			VERIFY(o->x == 42.f && o->y == 42.f && o->z == 42.f);
		}

		{
			optional<int> o;
			o.emplace(42);
			VERIFY(*o == 42);
		}
	}
	#endif


	// swap
	{
		{
			optional<int> o1 = 42, o2 = 24;
			VERIFY(*o1 == 42);
			VERIFY(*o2 == 24);
			o1.swap(o2);
			VERIFY(*o1 == 24);
			VERIFY(*o2 == 42);
		}

		{
			optional<int> o1 = 42, o2 = 24;
			VERIFY(*o1 == 42);
			VERIFY(*o2 == 24);
			swap(o1, o2);
			VERIFY(*o1 == 24);
			VERIFY(*o2 == 42);
		}
	}

	{
		optional<IntStruct> o(in_place, 10);
		optional<IntStruct> e;

		VERIFY(o < IntStruct(42));
		VERIFY(!(o < IntStruct(2)));
		VERIFY(!(o < IntStruct(10)));
		VERIFY(e < o);
		VERIFY(e < IntStruct(10));

		VERIFY(o > IntStruct(4));
		VERIFY(!(o > IntStruct(42)));

		VERIFY(o >= IntStruct(4));
		VERIFY(o >= IntStruct(10));
		VERIFY(IntStruct(4) <= o);
		VERIFY(IntStruct(10) <= o);

		VERIFY(o == IntStruct(10));
		VERIFY(o->data == IntStruct(10).data);

		VERIFY(o != IntStruct(11));
		VERIFY(o->data != IntStruct(11).data);
		
		VERIFY(e == nullopt);
		VERIFY(nullopt == e);

		VERIFY(o != nullopt);
		VERIFY(nullopt != o);
		VERIFY(nullopt < o);
		VERIFY(o > nullopt);
		VERIFY(!(nullopt > o));
		VERIFY(!(o < nullopt));
		VERIFY(nullopt <= o);
		VERIFY(o >= nullopt);
	}

	// hash 
	{
		{
			// verify that the hash an empty eastl::optional object is zero.
			typedef hash<optional<int>> hash_optional_t;
			optional<int> e;
			VERIFY(hash_optional_t{}(e) == 0);
		}

		{
			// verify that the hash is the same as the hash of the underlying type
			const char* const pMessage = "Electronic Arts Canada";
			typedef hash<optional<string>> hash_optional_t;
			optional<string> o = string(pMessage);
			VERIFY(hash_optional_t{}(o) == hash<string>{}(pMessage));
		}
	}

	// sorting
	{
		vector<optional<int>> v = {{122}, {115}, nullopt, {223}}; 
		sort(begin(v), end(v));
		vector<optional<int>> sorted = {nullopt, 115, 122, 223};

		VERIFY(v == sorted);
	}

	// Test constructor call for non trivial class
	{
		static_assert(!is_trivially_destructible_v<non_trivial_test>, "non_trivial_test<> must not trivial");
		optional<non_trivial_test> o1 = non_trivial_test();
		non_trivial_test::reset();
		optional<non_trivial_test> o2(o1);
		VERIFY(non_trivial_test::constructor_ran);
	}

	// test destructors being called.
	{
		destructor_test::reset();
		{
			optional<destructor_test> o = destructor_test{};
		}
		VERIFY(destructor_test::destructor_ran);

		destructor_test::reset();
		{
			optional<destructor_test> o;
		} 
		// destructor shouldn't be called as object wasn't constructed.
		VERIFY(!destructor_test::destructor_ran);


		destructor_test::reset();
		{
			optional<destructor_test> o = {};
		} 
		// destructor shouldn't be called as object wasn't constructed.
		VERIFY(!destructor_test::destructor_ran);

		destructor_test::reset();
		{
			optional<destructor_test> o = nullopt; 
		} 
		// destructor shouldn't be called as object wasn't constructed.
		VERIFY(!destructor_test::destructor_ran);
	}

	// test assignments called destructor & assignment operator or just constructor, depending
	// if optional has been previously initialized or not.
	{
		static_assert(is_trivially_destructible_v<trivial_test>, "class_test must be trivial");
		// Assignment a value to a already initialized optional
		{
			eastl::optional<trivial_test> o = trivial_test{};
			trivial_test other;
			trivial_test::reset();
			o = other;
			VERIFY(trivial_test::assignment_ran);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o = non_trivial_test{};
			non_trivial_test other;
			non_trivial_test::reset();
			o = other;
			VERIFY(non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);

		// Assignment a value to empty optional
		{
			eastl::optional<trivial_test> o;
			trivial_test other;
			trivial_test::reset();
			o = other;
			VERIFY(trivial_test::constructor_ran);
			VERIFY(!trivial_test::assignment_ran);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o;
			non_trivial_test other;
			non_trivial_test::reset();
			o = other;
			VERIFY(non_trivial_test::constructor_ran);
			VERIFY(!non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);

		// Assignment an optional non empty to a already initialized optional
		{
			eastl::optional<trivial_test> o = trivial_test{};
			eastl::optional<trivial_test> other = trivial_test{};
			trivial_test::reset();
			o = other;
			VERIFY(trivial_test::assignment_ran);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o = non_trivial_test{};
			eastl::optional<non_trivial_test> other = non_trivial_test{};
			non_trivial_test::reset();
			o = other;
			VERIFY(non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);

		// Assignment an optional non empty to empty optional
		{
			eastl::optional<trivial_test> o;
			eastl::optional<trivial_test> other = trivial_test{};
			trivial_test::reset();
			o = other;
			VERIFY(trivial_test::constructor_ran);
			VERIFY(!trivial_test::assignment_ran);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o;
			eastl::optional<non_trivial_test> other = non_trivial_test{};
			non_trivial_test::reset();
			o = other;
			VERIFY(non_trivial_test::constructor_ran);
			VERIFY(!non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);

		// Move an value to a already initialized optional
		{
			eastl::optional<trivial_test> o = trivial_test{};
			eastl::optional<trivial_test> other = trivial_test{};
			trivial_test::reset();
			o = std::move(other);
			VERIFY(trivial_test::assignment_ran);
			VERIFY(trivial_test::was_moved);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o = non_trivial_test{};
			eastl::optional<non_trivial_test> other = non_trivial_test{};
			non_trivial_test::reset();
			o = std::move(other);
			VERIFY(non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			VERIFY(non_trivial_test::was_moved);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);

		// Move an value to empty optional
		{
			eastl::optional<trivial_test> o;
			trivial_test other{};
			trivial_test::reset();
			o = std::move(other);
			VERIFY(trivial_test::constructor_ran);
			VERIFY(!trivial_test::assignment_ran);
			VERIFY(trivial_test::was_moved);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o;
			non_trivial_test other{};
			non_trivial_test::reset();
			o = std::move(other);
			VERIFY(non_trivial_test::constructor_ran);
			VERIFY(!non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			VERIFY(non_trivial_test::was_moved);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);

		// Move an optional non empty to a already initialized optional
		{
			eastl::optional<trivial_test> o = trivial_test{};
			trivial_test other{};
			trivial_test::reset();
			o = std::move(other);
			VERIFY(trivial_test::assignment_ran);
			VERIFY(trivial_test::was_moved);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o = non_trivial_test{};
			non_trivial_test other{};
			non_trivial_test::reset();
			o = std::move(other);
			VERIFY(non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			VERIFY(non_trivial_test::was_moved);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);

		// Move an optional non empty to empty optional
		{
			eastl::optional<trivial_test> o;
			eastl::optional<trivial_test> other = trivial_test{};
			trivial_test::reset();
			o = std::move(other);
			VERIFY(trivial_test::constructor_ran);
			VERIFY(!trivial_test::assignment_ran);
			VERIFY(trivial_test::was_moved);
			trivial_test::reset();
		}

		{
			eastl::optional<non_trivial_test> o;
			eastl::optional<non_trivial_test> other = non_trivial_test{};
			non_trivial_test::reset();
			o = std::move(other);
			VERIFY(non_trivial_test::constructor_ran);
			VERIFY(!non_trivial_test::assignment_ran);
			VERIFY(!non_trivial_test::destructor_ran);
			VERIFY(non_trivial_test::was_moved);
			non_trivial_test::reset();
		}
		VERIFY(non_trivial_test::destructor_ran);
	}

	// optional rvalue tests
	{
		VERIFY(*optional<int>(1)                          == 1);
		VERIFY( optional<int>(1).value()                  == 1);
		VERIFY( optional<int>(1).value_or(0xdeadf00d)     == 1);
		VERIFY( optional<int>().value_or(0xdeadf00d)      == 0xdeadf00d);
		VERIFY( optional<int>(1).has_value()              == true);
		VERIFY( optional<int>().has_value()               == false);
		VERIFY( optional<IntStruct>(in_place, 10)->data   == 10);

	}

	// alignment type tests
	{
		static_assert(alignof(optional<Align16>) == alignof(Align16), "optional alignment failure");
		static_assert(alignof(optional<Align32>) == alignof(Align32), "optional alignment failure");
		static_assert(alignof(optional<Align64>) == alignof(Align64), "optional alignment failure");
	}

	{
		// user reported regression that failed to compile
		struct local_struct
		{
			local_struct() {}
			~local_struct() {}
		};
		static_assert(!eastl::is_trivially_destructible_v<local_struct>, "");

		{
			local_struct ls;
			eastl::optional<local_struct> o{ls};
		}
		{
			const local_struct ls;
			eastl::optional<local_struct> o{ls};
		}
	}

	{
		{
			// user regression
			eastl::optional<eastl::string> o = eastl::string("Hello World");
			eastl::optional<eastl::string> co;

			co = o; // force copy-assignment

			VERIFY( o.value().data() != co.value().data());
			VERIFY( o.value().data() == eastl::string("Hello World"));
			VERIFY(co.value().data() == eastl::string("Hello World"));
		}
		{
			// user regression
			struct local
			{
				eastl::unique_ptr<int> ptr;

				local(const local&) = delete;
				local(local&&) = default;
				local& operator=(const local&) = delete;
				local& operator=(local&&) = default;
			};

			eastl::optional<local> o1 = local{eastl::make_unique<int>(42)};
			eastl::optional<local> o2;

			o2 = eastl::move(o1);

			VERIFY(!!o1 == true);
			VERIFY(!!o2 == true);
			VERIFY(o2->ptr.get() != nullptr);
		}
	}

    #endif // EASTL_OPTIONAL_ENABLED
	return nErrorCount;
}

