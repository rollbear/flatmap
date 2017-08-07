#include "flatmap.hpp"
#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <memory>
#include <string>
#include <utility>

using namespace std::string_literals;

namespace {
  template <typename T>
  inline
  constexpr
  const T& as_const(T& t) noexcept { return t;}

  template <typename T>
  void as_const(T&& t) = delete;
}

TEST_CASE("a default constructed unordered_flatmap is empty")
{
  unordered_flatmap<int, std::unique_ptr<int>> map;
  REQUIRE(map.empty());
  REQUIRE(map.size() == 0U);
  REQUIRE(map.begin() == map.end());
  REQUIRE(map.cbegin() == map.cend());
}

TEST_CASE("a value inserted into an unordered_flatmap can be looked up with find")
{
  unordered_flatmap<int, std::string> map;
  map.insert({1, "one"});
  map.insert({3, "three"});
  map.insert({2, "two"});
  auto i = map.find(1);
  REQUIRE(i != map.end());
  REQUIRE(i->second == "one");
  i = map.find(2);
  REQUIRE(i != map.end());
  REQUIRE(i->second == "two");
  i = map.find(3);
  REQUIRE(i != map.end());
  REQUIRE(i->second == "three");
  i = map.find(4);
  REQUIRE(i == map.end());
}

TEST_CASE("clear empties a unordered_flatmap")
{
  unordered_flatmap<int, std::string> map;
  map.insert({1, "one"});
  map.insert({3, "three"});
  map.insert({2, "two"});
  REQUIRE(!map.empty());
  REQUIRE(map.size() == 3U);
  REQUIRE(map.begin() != map.end());
  map.clear();
  REQUIRE(map.empty());
  REQUIRE(map.size() == 0U);
  REQUIRE(map.begin() == map.end());
}
TEST_CASE("a value inserted into an unordered_flatmap can be looked up with operator[] of a compatible key type")
{
  unordered_flatmap<std::string, int> map;
  map.insert({"one"s, 1});
  map.insert({"three"s, 3});
  map.insert({"two"s, 2});
  REQUIRE(map.size() == 3U);
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
  REQUIRE(map.size() == 3U);
}

TEST_CASE("operator[] on an unordered_flatmap with an unknown key constructs a new value")
{
  unordered_flatmap<std::string, int> map;
  map.insert({"one"s, 1});
  map.insert({"three"s, 3});
  REQUIRE(map.size() == 2U);
  map["two"] = 2;
  REQUIRE(map.size() == 3U);
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
}

TEST_CASE("an iterator from unordered_flatmap::find can be used to erase an element")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto i = map.find("one");
  REQUIRE(i != map.cend());
  REQUIRE(i->first == "one");
  REQUIRE(*i->second == 1);
  map.erase(i);
  REQUIRE(map.size() == 2U);
  REQUIRE(map.find("one") == map.end());
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
}

TEST_CASE("an element can be erased from an unordered_flatmap using a matching compatible key")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto count = map.erase("one");
  REQUIRE(count == 1U);
  REQUIRE(map.count("one") == 0U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
}

TEST_CASE("inserting an rvalue to an existing key in an unordered_flatmap does nothing and returns iterator to element")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto [ iterator, inserted ] = map.insert({"three"s, std::make_unique<int>(-3)});
  REQUIRE(!inserted);
  REQUIRE(iterator->first == "three");
  REQUIRE(*iterator->second == 3);
  REQUIRE(map.count("one") == 1U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
}

TEST_CASE("insert rvalue to an non-existing key in an unordered_flatmap inserts and returns iterator to element")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto [ iterator, inserted ] = map.insert({"four"s, std::make_unique<int>(4)});
  REQUIRE(inserted);
  REQUIRE(iterator->first == "four");
  REQUIRE(*iterator->second == 4);
  REQUIRE(map.size() == 4U);
  REQUIRE(map.count("one") == 1U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
  REQUIRE(map.count("four") == 1U);
}

TEST_CASE("insert lvalue to an existing key in an unordered_flatmap does nothing and returns iterator to element")
{
  unordered_flatmap<std::string, int> map{{"one"s, 1}, {"two"s, 2}, {"three"s, 3}};
  auto p = std::pair<const std::string, int>("two"s, -2);
  auto [ iterator, inserted ] = map.insert(p);
  REQUIRE(!inserted);
  REQUIRE(iterator->first == "two");
  REQUIRE(iterator->second == 2);
  REQUIRE(map.size() == 3);
}

TEST_CASE("insert lvalue to a unique key in an unordered_flatmap inserts and returns iterator to element")
{
  unordered_flatmap<std::string, int> map{{"one"s, 1}, {"two"s, 2}, {"three"s, 3}};
  auto p = std::pair<const std::string, int>("four"s, 4);
  auto [ iterator, inserted ] = map.insert(p);
  REQUIRE(inserted);
  REQUIRE(iterator->first == "four");
  REQUIRE(iterator->second == 4);
  REQUIRE(map.size() == 4);
  REQUIRE(map.count("four") == 1);
}

TEST_CASE("erasing using a compatible key that does not match any element in an unordered_flatmap is a no-op returning 0")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto count = map.erase("four");
  REQUIRE(count == 0U);
  REQUIRE(map.count("one") == 1U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
}

TEST_CASE("compatible values can be emplaced into a unordered_flatmap")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  REQUIRE(map.size() == 3U);
}

TEST_CASE("a unordered_flatmap can be constructed from a compatible initializer_list")
{
  unordered_flatmap<std::string, int> strmap({std::make_pair("one", 1), std::make_pair("two", 2), std::make_pair("three", 3)});
  REQUIRE(!strmap.empty());
  REQUIRE(strmap.size() == 3U);
}

TEST_CASE("a unordered_flatmap can be constructed from a compatible iterator range")
{
  std::pair<const char*, unsigned> v[]{{"one", 1}, {"two", 2}, {"three", 3}, {"four", 4}};
  unordered_flatmap<std::string, unsigned long> map(std::begin(v), std::end(v));
  REQUIRE(map.size() == 4U);
  REQUIRE(map["one"] == 1U);
  REQUIRE(map["two"] == 2U);
  REQUIRE(map["three"] == 3U);
  REQUIRE(map["four"] == 4U);
}

TEST_CASE("insert_or_assign into an unordered_flatmap inserts when key is not found")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  auto [iter, inserted] = map.insert_or_assign("four", std::make_unique<int>(4));
  REQUIRE(inserted);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
  REQUIRE(map.size() == 4U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
}

TEST_CASE("insert_or_assign into an unordered_flatmap assigns when key is found")
{
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  auto [iter, inserted] = map.insert_or_assign("three", std::make_unique<int>(-3));
  REQUIRE(!inserted);
  REQUIRE(iter->first == "three");
  REQUIRE(*iter->second == -3);
  REQUIRE(map.size() == 3U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == -3);
}

TEST_CASE("try_emplace into an unordered_flatmap to a matching compatible key does nothing")
{
  struct maker {
    int m;
    operator std::unique_ptr<int>() &&{ m = -m; return std::make_unique<int>(-m);}
  };
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  maker m{33};
  auto [iter, inserted] = map.try_emplace("three", std::move(m));
  REQUIRE(!inserted);
  REQUIRE(iter->first == "three");
  REQUIRE(*iter->second == 3);
  REQUIRE(m.m == 33);
  REQUIRE(map.size() == 3U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
}

TEST_CASE("try_emplace into an unordered_flatmap to a unique compatible key creates new element")
{
  struct maker {
    int m;
    operator std::unique_ptr<int>() &&{ m = -m; return std::make_unique<int>(-m);}
  };
  unordered_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  maker m{4};
  auto [iter, inserted] = map.try_emplace("four", std::move(m));
  REQUIRE(inserted);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
  REQUIRE(m.m == -4);
  REQUIRE(map.size() == 4U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
  REQUIRE(*map["four"] == 4);
}

////
TEST_CASE("a default constructed unordered_split_flatmap is empty")
{
  unordered_split_flatmap<int, std::unique_ptr<int>> map;
  REQUIRE(map.empty());
  REQUIRE(map.size() == 0U);
  REQUIRE(map.begin() == map.end());
  REQUIRE(map.cbegin() == map.cend());
}

TEST_CASE("a value inserted into an unordered_split_flatmap can be looked up with find")
{
  unordered_split_flatmap<int, std::string> map;
  map.insert({1, "one"});
  map.insert({3, "three"});
  map.insert({2, "two"});
  auto i = map.find(1);
  REQUIRE(i != map.end());
  REQUIRE(i->second == "one");
  i = map.find(2);
  REQUIRE(i != map.end());
  REQUIRE(i->second == "two");
  i = map.find(3);
  REQUIRE(i != map.end());
  REQUIRE(i->second == "three");
  i = map.find(4);
  REQUIRE(i == map.end());
}

TEST_CASE("clear empties a unordered_split_flatmap")
{
  unordered_split_flatmap<int, std::string> map;
  map.insert({1, "one"});
  map.insert({3, "three"});
  map.insert({2, "two"});
  REQUIRE(!map.empty());
  REQUIRE(map.size() == 3U);
  REQUIRE(map.begin() != map.end());
  map.clear();
  REQUIRE(map.empty());
  REQUIRE(map.size() == 0U);
  REQUIRE(map.begin() == map.end());
}
TEST_CASE("a value inserted into an unordered_split_flatmap can be looked up with operator[] of a compatible key type")
{
  unordered_split_flatmap<std::string, int> map;
  map.insert({"one"s, 1});
  map.insert({"three"s, 3});
  map.insert({"two"s, 2});
  REQUIRE(map.size() == 3U);
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
  REQUIRE(map.size() == 3U);
}

TEST_CASE("operator[] on an unordered_split_flatmap with an unknown key constructs a new value")
{
  unordered_split_flatmap<std::string, int> map;
  map.insert({"one"s, 1});
  map.insert({"three"s, 3});
  REQUIRE(map.size() == 2U);
  map["two"] = 2;
  REQUIRE(map.size() == 3U);
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
}

TEST_CASE("an iterator from unordered_split_flatmap::find can be used to erase an element")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto i = map.find("one");
  REQUIRE(i != map.cend());
  REQUIRE(i->first == "one");
  REQUIRE(*i->second == 1);
  map.erase(i);
  REQUIRE(map.size() == 2U);
  REQUIRE(map.find("one") == map.end());
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
}

TEST_CASE("an element can be erased from an unordered_split_flatmap using a matching compatible key")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto count = map.erase("one");
  REQUIRE(count == 1U);
  REQUIRE(map.count("one") == 0U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
}

TEST_CASE("inserting an rvalue to an existing key in an unordered_split_flatmap does nothing and returns iterator to element")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto [ iterator, inserted ] = map.insert({"three"s, std::make_unique<int>(-3)});
  REQUIRE(!inserted);
  REQUIRE(iterator->first == "three");
  REQUIRE(*iterator->second == 3);
  REQUIRE(map.count("one") == 1U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
}

TEST_CASE("insert rvalue to an non-existing key in an unordered_split_flatmap inserts and returns iterator to element")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto [ iterator, inserted ] = map.insert({"four"s, std::make_unique<int>(4)});
  REQUIRE(inserted);
  REQUIRE(iterator->first == "four");
  REQUIRE(*iterator->second == 4);
  REQUIRE(map.size() == 4U);
  REQUIRE(map.count("one") == 1U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
  REQUIRE(map.count("four") == 1U);
}

TEST_CASE("insert lvalue to an existing key in an unordered_split_flatmap does nothing and returns iterator to element")
{
  unordered_split_flatmap<std::string, int> map{{"one"s, 1}, {"two"s, 2}, {"three"s, 3}};
  auto p = std::pair<const std::string, int>("two"s, -2);
  auto [ iterator, inserted ] = map.insert(p);
  REQUIRE(!inserted);
  REQUIRE(iterator->first == "two");
  REQUIRE(iterator->second == 2);
  REQUIRE(map.size() == 3);
}

TEST_CASE("insert lvalue to a unique key in an unordered_split_flatmap inserts and returns iterator to element")
{
  unordered_split_flatmap<std::string, int> map{{"one"s, 1}, {"two"s, 2}, {"three"s, 3}};
  auto p = std::pair<const std::string, int>("four"s, 4);
  auto [ iterator, inserted ] = map.insert(p);
  REQUIRE(inserted);
  REQUIRE(iterator->first == "four");
  REQUIRE(iterator->second == 4);
  REQUIRE(map.size() == 4);
  REQUIRE(map.count("four") == 1);
}

TEST_CASE("erasing using a compatible key that does not match any element in an unordered_split_flatmap is a no-op returning 0")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one"s, std::make_unique<int>(1)});
  map.insert({"three"s, std::make_unique<int>(3)});
  map.insert({"two"s, std::make_unique<int>(2)});
  auto count = map.erase("four");
  REQUIRE(count == 0U);
  REQUIRE(map.count("one") == 1U);
  REQUIRE(map.count("two") == 1U);
  REQUIRE(map.count("three") == 1U);
}

TEST_CASE("compatible values can be emplaced into a unordered_split_flatmap")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  REQUIRE(map.size() == 3U);
}

TEST_CASE("a unordered_split_flatmap can be constructed from a compatible initializer_list")
{
  unordered_split_flatmap<std::string, int> strmap({std::make_pair("one", 1), std::make_pair("two", 2), std::make_pair("three", 3)});
  REQUIRE(!strmap.empty());
  REQUIRE(strmap.size() == 3U);
}

TEST_CASE("a unordered_split_flatmap can be constructed from a compatible iterator range")
{
  std::pair<const char*, unsigned> v[]{{"one", 1}, {"two", 2}, {"three", 3}, {"four", 4}};
  unordered_split_flatmap<std::string, unsigned long> map(std::begin(v), std::end(v));
  REQUIRE(map.size() == 4U);
  REQUIRE(map["one"] == 1U);
  REQUIRE(map["two"] == 2U);
  REQUIRE(map["three"] == 3U);
  REQUIRE(map["four"] == 4U);
}

TEST_CASE("insert_or_assign into an unordered_split_flatmap inserts when key is not found")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  auto [iter, inserted] = map.insert_or_assign("four", std::make_unique<int>(4));
  REQUIRE(inserted);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
  REQUIRE(map.size() == 4U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
}

TEST_CASE("insert_or_assign into an unordered_split_flatmap assigns when key is found")
{
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  auto [iter, inserted] = map.insert_or_assign("three", std::make_unique<int>(-3));
  REQUIRE(!inserted);
  REQUIRE(iter->first == "three");
  REQUIRE(*iter->second == -3);
  REQUIRE(map.size() == 3U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == -3);
}

TEST_CASE("try_emplace into an unordered_split_flatmap to a matching compatible key does nothing")
{
  struct maker {
    int m;
    operator std::unique_ptr<int>() &&{ m = -m; return std::make_unique<int>(-m);}
  };
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  maker m{33};
  auto [iter, inserted] = map.try_emplace("three", std::move(m));
  REQUIRE(!inserted);
  REQUIRE(iter->first == "three");
  REQUIRE(*iter->second == 3);
  REQUIRE(m.m == 33);
  REQUIRE(map.size() == 3U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
}

TEST_CASE("try_emplace into an unordered_split_flatmap to a unique compatible key creates new element")
{
  struct maker {
    int m;
    operator std::unique_ptr<int>() &&{ m = -m; return std::make_unique<int>(-m);}
  };
  unordered_split_flatmap<std::string, std::unique_ptr<int>> map;
  map.emplace("one", std::make_unique<int>(1));
  map.emplace("three", std::make_unique<int>(3));
  map.emplace("two", std::make_unique<int>(2));
  maker m{4};
  auto [iter, inserted] = map.try_emplace("four", std::move(m));
  REQUIRE(inserted);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
  REQUIRE(m.m == -4);
  REQUIRE(map.size() == 4U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
  REQUIRE(*map["four"] == 4);
}
////
TEST_CASE("a default constructed flatmap is empty")
{
  flatmap<int,int> map;
  REQUIRE(map.empty());
  REQUIRE(map.size() == 0U);
  REQUIRE(map.begin() == map.end());
}

TEST_CASE("rvalues inserted into a flatmap are stored in increasing key order")
{
  flatmap<int, int> map;
  {
    auto [iter, inserted] = map.insert({2, -2});
    REQUIRE(inserted);
    REQUIRE(iter->first == 2);
    REQUIRE(iter->second == -2);
  }
  {
    auto [iter, inserted] = map.insert({1, -1});
    REQUIRE(inserted);
    REQUIRE(iter->first == 1);
    REQUIRE(iter->second == -1);
  }
  {
    auto [iter, inserted] = map.insert({3,-3});
    REQUIRE(inserted);
    REQUIRE(iter->first == 3);
    REQUIRE(iter->second == -3);
  }
  auto i = map.begin();
  REQUIRE(i->first == 1);
  ++i;
  REQUIRE(i->first == 2);
  i++;
  REQUIRE(i->first == 3);
  ++i;
  REQUIRE(i == map.end());
}

TEST_CASE("rvalues inserted into a flatmap with greater<> comparator are stored in decreasing key order")
{
  flatmap<int, int,std::greater<>> map;
  {
    auto [iter, inserted] = map.insert({2, -2});
    REQUIRE(inserted);
    REQUIRE(iter->first == 2);
    REQUIRE(iter->second == -2);
  }
  {
    auto [iter, inserted] = map.insert({1, -1});
    REQUIRE(inserted);
    REQUIRE(iter->first == 1);
    REQUIRE(iter->second == -1);
  }
  {
    auto [iter, inserted] = map.insert({3,-3});
    REQUIRE(inserted);
    REQUIRE(iter->first == 3);
    REQUIRE(iter->second == -3);
  }
  auto i = map.begin();
  REQUIRE(i->first == 3);
  ++i;
  REQUIRE(i->first == 2);
  i++;
  REQUIRE(i->first == 1);
  ++i;
  REQUIRE(i == map.end());

}


TEST_CASE("rvalue with colliding key inserted into flatmap is not stored and returns false")
{
  flatmap<int, int> map;
  map.insert({1,-1});
  map.insert({2,-2});
  map.insert({3,-3});
  {
    auto [ iter, inserted ] = map.insert({1,1});
    REQUIRE(!inserted);
    REQUIRE(iter->first == 1);
    REQUIRE(iter->second == -1);
  }
  {
    auto [ iter, inserted ] = map.insert({2,2});
    REQUIRE(!inserted);
    REQUIRE(iter->first == 2);
    REQUIRE(iter->second == -2);
  }
  {
    auto [ iter, inserted ] = map.insert({3,3});
    REQUIRE(!inserted);
    REQUIRE(iter->first == 3);
    REQUIRE(iter->second == -3);
  }
  REQUIRE(map.size() == 3U);
}

TEST_CASE("find a matching key type from a flatmap returns an iterator to found element")
{
  flatmap<std::string, int> map;
  map.insert({"three", 3});
  map.insert({"two", 2});
  map.insert({"one", 1});
  {
    auto i = map.find("one");
    REQUIRE(i != map.end());
    REQUIRE(i->first == "one");
    REQUIRE(i->second == 1);
  }
  {
    auto i = map.find("two");
    REQUIRE(i != map.end());
    REQUIRE(i->first == "two");
    REQUIRE(i->second == 2);
  }
  {
    auto i = map.find("three");
    REQUIRE(i != map.end());
    REQUIRE(i->first == "three");
    REQUIRE(i->second == 3);
  }
  {
    auto i = map.find("twoo");
    REQUIRE(i == map.end());
  }
}

TEST_CASE("a flatmap can be populated from an initializer_list in the constructor")
{
  flatmap<std::string, int> map{
    {"one", 1},
    {"two", 2},
    {"three", 3}
  };
  {
    auto i = map.find("one");
    REQUIRE(as_const(map).find("one") == i);
    REQUIRE(i != map.cend());
    REQUIRE(i->first == "one");
    REQUIRE(i->second == 1);
  }
  {
    auto i = map.find("two");
    REQUIRE(as_const(map).find("two") == i);
    REQUIRE(i != map.cend());
    REQUIRE(i->first == "two");
    REQUIRE(i->second == 2);
  }
  {
    auto i = map.find("three");
    REQUIRE(as_const(map).find("three") == i);
    REQUIRE(i != map.cend());
    REQUIRE(i->first == "three");
    REQUIRE(i->second == 3);
  }
}

TEST_CASE("the value of a known key can be looked up from a flat map using operator[]")
{
  flatmap<std::string, int> map{
    {"one", 1},
    {"two", 2},
    {"three", 3}
  };
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
  REQUIRE(map.size() == 3U);
}

TEST_CASE("un unknown key used in operator[] for a flat map default constructs a value for the key")
{
  flatmap<std::string, int> map{
    {"one", 1},
    {"two", 2},
    {"three", 3}
  };
  map["four"] = 4;
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
  REQUIRE(map["four"] == 4);
  REQUIRE(map.size() == 4U);
}

TEST_CASE("emplace to a flatmap with a known key and rvalue moves the parameters and returns an iterator to the known element")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(-2);
  auto [ iter, inserted ] = map.emplace("two", std::move(i));
  REQUIRE(!inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "two");
  REQUIRE(*iter->second == 2);
}

TEST_CASE("emplace to a flatmap with a unique key and rvalue moves the parameters into a new object and returns true")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(4);
  auto [ iter, inserted ] = map.emplace("four", std::move(i));
  REQUIRE(inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
}

TEST_CASE("insert_or_assign to a flatmap with a known key and an rvalue move assigns and returns false")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(-2);
  auto [ iter, inserted ] = map.insert_or_assign("two", std::move(i));
  REQUIRE(!inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "two");
  REQUIRE(*iter->second == -2);

}

TEST_CASE("insert_or_assign to a flatmap with a unique key and rvalue moves the parameters into a new object and returns true")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(4);
  auto [ iter, inserted ] = map.insert_or_assign("four", std::move(i));
  REQUIRE(inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
}

TEST_CASE("try_emplace to a flatmap with a known key and rvalue ignores parameters and returns an iterator to the known element")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(-2);
  auto [ iter, inserted ] = map.try_emplace("two", std::move(i));
  REQUIRE(!inserted);
  REQUIRE(i);
  REQUIRE(*i == -2);
  REQUIRE(iter->first == "two");
  REQUIRE(*iter->second == 2);
}

TEST_CASE("try_emplace to a flatmap with a unique key and rvalue moves the parameters into a new object and returns true")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(4);
  auto [ iter, inserted ] = map.try_emplace("four", std::move(i));
  REQUIRE(inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
}

TEST_CASE("range based for on a flatmap traverses element in sorted order")
{
  std::string keys[] { "four", "one", "three", "two"};
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  auto i = std::begin(keys);
  for (auto& x : map)
  {
    REQUIRE(x.first == *i);
    ++i;
  }
  REQUIRE(i == std::end(keys));
}

TEST_CASE("count on unknown key in flatmap returns 0")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  REQUIRE(map.count("five") == 0U);
  REQUIRE(as_const(map).count("five") == 0U);
}

TEST_CASE("count on known key in flatmap returns 1")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  REQUIRE(map.count("three") == 1U);
  REQUIRE(as_const(map).count("four") == 1U);
}

TEST_CASE("erase an unkown key from a flatmap is a noop returning 0")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  auto rv = map.erase("five");
  REQUIRE(rv == 0U);
  REQUIRE(map.size() == 4U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
  REQUIRE(*map["four"] == 4);
}

TEST_CASE("erase a kown key from a flatmap removes the element and returns 1")
{
  flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  auto rv = map.erase("three");
  REQUIRE(rv == 1U);
  REQUIRE(map.size() == 3U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(map["three"] == nullptr);
  REQUIRE(*map["four"] == 4);
}

TEST_CASE("a flatmap is constructible from iterators of compatible types")
{
  std::pair<const char*, unsigned> v[] { {"one", 1U}, {"two", 2U}, {"three", 3U}};
  flatmap<std::string, int> map(std::begin(v), std::end(v));
  REQUIRE(map.size() == 3);
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
}
////

TEST_CASE("a default constructed split_flatmap is empty")
{
  split_flatmap<int,int> map;
  REQUIRE(map.empty());
  REQUIRE(map.size() == 0U);
  REQUIRE(map.begin() == map.end());
}

TEST_CASE("rvalues inserted into a split_flatmap are stored in increasing key order")
{
  split_flatmap<int, int> map;
  {
    auto [iter, inserted] = map.insert({2, -2});
    REQUIRE(inserted);
    REQUIRE(iter->first == 2);
    REQUIRE(iter->second == -2);
  }
  {
    auto [iter, inserted] = map.insert({1, -1});
    REQUIRE(inserted);
    REQUIRE(iter->first == 1);
    REQUIRE(iter->second == -1);
  }
  {
    auto [iter, inserted] = map.insert({3,-3});
    REQUIRE(inserted);
    REQUIRE(iter->first == 3);
    REQUIRE(iter->second == -3);
  }
  auto i = map.begin();
  REQUIRE(i->first == 1);
  ++i;
  REQUIRE(i->first == 2);
  i++;
  REQUIRE(i->first == 3);
  ++i;
  REQUIRE(i == map.end());
}

TEST_CASE("rvalues inserted into a split_flatmap with greater<> comparator are stored in decreasing key order")
{
  split_flatmap<int, int,std::greater<>> map;
  {
    auto [iter, inserted] = map.insert({2, -2});
    REQUIRE(inserted);
    REQUIRE(iter->first == 2);
    REQUIRE(iter->second == -2);
  }
  {
    auto [iter, inserted] = map.insert({1, -1});
    REQUIRE(inserted);
    REQUIRE(iter->first == 1);
    REQUIRE(iter->second == -1);
  }
  {
    auto [iter, inserted] = map.insert({3,-3});
    REQUIRE(inserted);
    REQUIRE(iter->first == 3);
    REQUIRE(iter->second == -3);
  }
  auto i = map.begin();
  REQUIRE(i->first == 3);
  ++i;
  REQUIRE(i->first == 2);
  i++;
  REQUIRE(i->first == 1);
  ++i;
  REQUIRE(i == map.end());

}


TEST_CASE("rvalue with colliding key inserted into split_flatmap is not stored and returns false")
{
  split_flatmap<int, int> map;
  map.insert({1,-1});
  map.insert({2,-2});
  map.insert({3,-3});
  {
    auto [ iter, inserted ] = map.insert({1,1});
    REQUIRE(!inserted);
    REQUIRE(iter->first == 1);
    REQUIRE(iter->second == -1);
  }
  {
    auto [ iter, inserted ] = map.insert({2,2});
    REQUIRE(!inserted);
    REQUIRE(iter->first == 2);
    REQUIRE(iter->second == -2);
  }
  {
    auto [ iter, inserted ] = map.insert({3,3});
    REQUIRE(!inserted);
    REQUIRE(iter->first == 3);
    REQUIRE(iter->second == -3);
  }
  REQUIRE(map.size() == 3U);
}

TEST_CASE("find a matching key type from a split_flatmap returns an iterator to found element")
{
  split_flatmap<std::string, int> map;
  map.insert({"three", 3});
  map.insert({"two", 2});
  map.insert({"one", 1});
  {
    auto i = map.find("one");
    REQUIRE(i != map.end());
    REQUIRE(i->first == "one");
    REQUIRE(i->second == 1);
  }
  {
    auto i = map.find("two");
    REQUIRE(i != map.end());
    REQUIRE(i->first == "two");
    REQUIRE(i->second == 2);
  }
  {
    auto i = map.find("three");
    REQUIRE(i != map.end());
    REQUIRE(i->first == "three");
    REQUIRE(i->second == 3);
  }
  {
    auto i = map.find("twoo");
    REQUIRE(i == map.end());
  }
}

TEST_CASE("a split_flatmap can be populated from an initializer_list in the constructor")
{
  split_flatmap<std::string, int> map{
    {"one", 1},
    {"two", 2},
    {"three", 3}
  };
  {
    auto i = map.find("one");
    REQUIRE(as_const(map).find("one") == i);
    REQUIRE(i != map.cend());
    REQUIRE(i->first == "one");
    REQUIRE(i->second == 1);
  }
  {
    auto i = map.find("two");
    REQUIRE(as_const(map).find("two") == i);
    REQUIRE(i != map.cend());
    REQUIRE(i->first == "two");
    REQUIRE(i->second == 2);
  }
  {
    auto i = map.find("three");
    REQUIRE(as_const(map).find("three") == i);
    REQUIRE(i != map.cend());
    REQUIRE(i->first == "three");
    REQUIRE(i->second == 3);
  }
}

TEST_CASE("the value of a known key can be looked up from a split_flatmap using operator[]")
{
  split_flatmap<std::string, int> map{
    {"one", 1},
    {"two", 2},
    {"three", 3}
  };
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
  REQUIRE(map.size() == 3U);
}

TEST_CASE("un unknown key used in operator[] for a split_flatmap default constructs a value for the key")
{
  split_flatmap<std::string, int> map{
    {"one", 1},
    {"two", 2},
    {"three", 3}
  };
  map["four"] = 4;
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
  REQUIRE(map["four"] == 4);
  REQUIRE(map.size() == 4U);
}

TEST_CASE("emplace to a split_flatmap with a known key and rvalue moves the parameters and returns an iterator to the known element")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(-2);
  auto [ iter, inserted ] = map.emplace("two", std::move(i));
  REQUIRE(!inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "two");
  REQUIRE(*iter->second == 2);
}

TEST_CASE("emplace to a split_flatmap with a unique key and rvalue moves the parameters into a new object and returns true")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(4);
  auto [ iter, inserted ] = map.emplace("four", std::move(i));
  REQUIRE(inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
}

TEST_CASE("insert_or_assign to a split_flatmap with a known key and an rvalue move assigns and returns false")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(-2);
  auto [ iter, inserted ] = map.insert_or_assign("two", std::move(i));
  REQUIRE(!inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "two");
  REQUIRE(*iter->second == -2);

}

TEST_CASE("insert_or_assign to a split_flatmap with a unique key and rvalue moves the parameters into a new object and returns true")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(4);
  auto [ iter, inserted ] = map.insert_or_assign("four", std::move(i));
  REQUIRE(inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
}

TEST_CASE("try_emplace to a split_flatmap with a known key and rvalue ignores parameters and returns an iterator to the known element")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(-2);
  auto [ iter, inserted ] = map.try_emplace("two", std::move(i));
  REQUIRE(!inserted);
  REQUIRE(i);
  REQUIRE(*i == -2);
  REQUIRE(iter->first == "two");
  REQUIRE(*iter->second == 2);
}

TEST_CASE("try_emplace to a split_flatmap with a unique key and rvalue moves the parameters into a new object and returns true")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  auto i = std::make_unique<int>(4);
  auto [ iter, inserted ] = map.try_emplace("four", std::move(i));
  REQUIRE(inserted);
  REQUIRE(!i);
  REQUIRE(iter->first == "four");
  REQUIRE(*iter->second == 4);
}

TEST_CASE("range based for on a split_flatmap traverses element in sorted order")
{
  std::string keys[] { "four", "one", "three", "two"};
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  auto i = std::begin(keys);
  for (auto x : map)
  {
    REQUIRE(x.first == *i);
    ++i;
  }
  REQUIRE(i == std::end(keys));
}

TEST_CASE("count on unknown key in split_flatmap returns 0")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  REQUIRE(map.count("five") == 0U);
  REQUIRE(as_const(map).count("five") == 0U);
}

TEST_CASE("count on known key in split_flatmap returns 1")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  REQUIRE(map.count("three") == 1U);
  REQUIRE(as_const(map).count("four") == 1U);
}

TEST_CASE("erase an unkown key from a split_flatmap is a noop returning 0")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  auto rv = map.erase("five");
  REQUIRE(rv == 0U);
  REQUIRE(map.size() == 4U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(*map["three"] == 3);
  REQUIRE(*map["four"] == 4);
}

TEST_CASE("erase a kown key from a split_flatmap removes the element and returns 1")
{
  split_flatmap<std::string, std::unique_ptr<int>> map;
  map.insert({"one", std::make_unique<int>(1)});
  map.insert({"two", std::make_unique<int>(2)});
  map.insert({"three", std::make_unique<int>(3)});
  map.insert({"four", std::make_unique<int>(4)});
  auto rv = map.erase("three");
  REQUIRE(rv == 1U);
  REQUIRE(map.size() == 3U);
  REQUIRE(*map["one"] == 1);
  REQUIRE(*map["two"] == 2);
  REQUIRE(map["three"] == nullptr);
  REQUIRE(*map["four"] == 4);
}

TEST_CASE("a split_flatmap is constructible from iterators of compatible types")
{
  std::pair<const char*, unsigned> v[] { {"one", 1U}, {"two", 2U}, {"three", 3U}};
  split_flatmap<std::string, int> map(std::begin(v), std::end(v));
  REQUIRE(map.size() == 3);
  REQUIRE(map["one"] == 1);
  REQUIRE(map["two"] == 2);
  REQUIRE(map["three"] == 3);
}
