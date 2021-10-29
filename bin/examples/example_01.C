#include <iostream>
#include <sstream>
#include <hobbes/hobbes.H>
#include "example.H"

#define PRINT_RESULT(FUNCTION, COMMAND, PARAMETERS) \
  COMMAND << " = " << compiler.compileFn<FUNCTION>(COMMAND)(PARAMETERS)
#define PRINT_RESULT0(COMMAND, FUNCTION, ARGUMENTS, PARAMETERS) \
  COMMAND << " = " << compiler.compileFn<FUNCTION>(ARGUMENTS)(PARAMETERS)
#define CONCATENATE(...) __VA_ARGS__

using HobbesCharArray = const hobbes::array<char>*;

template<typename T>
std::string print(const T&) { return ""; }

/**
 Use the macro `DEFINE_STRUCT` to define hobbes cc bindable struct/class.
*/
DEFINE_STRUCT(
  Bob,
  (int,         x),
  (double,      y),
  (std::string, z),
  (char,        w)
);
bool isAlpha(const Bob& bob) { return bob.w >= 'a' && bob.w <= 'z'; }
void incrementBobW(int i, Bob& bob) {
  RUN_CODE(std::cout << "calling incrementBobW with i=" << i;);
  bob.w += 1;
}

struct Compiler
{
  static hobbes::cc& instance() {
    static hobbes::cc compiler; // known as cc
    return compiler;
  }

  static void init() {
    if(_isInitialized) return;
    auto& compiler { instance() };
    _bob.x = 42;
    _bob.y = 42.2;
    _bob.z = "Hello!";
    _bob.w = 'c';
    compiler.bind("bob", &_bob); ///< binds a Bob object to an instance within the compiler
    compiler.bind("isAlpha", &isAlpha); ///< binds a function
    compiler.bind("incrementBobW", &incrementBobW); ///< binds a function
    _isInitialized = true;
  }

  static const Bob& bob() { return _bob; }

private:  
  static bool _isInitialized;
  static struct Bob _bob;
};

bool Compiler::_isInitialized = false;
Bob Compiler::_bob {
  .x = 42, .y = 42.2, .z = "Hello!", .w = 'c'
};

// specialized template
template<>
std::string print(const Bob& bob) {
  std::stringstream ss;
  ss << "x = " << bob.x << ", y = " << bob.y << ", z = " << bob.z << ", w = " << bob.w;
  return ss.str();
}

int main() {
  Compiler::init();
  auto& compiler = Compiler::instance();
  auto& bob = Compiler::bob();

  RUN_CODE(
    std::cout << std::boolalpha
    /**
     * Evaluate expressions
     */
    << PRINT_RESULT(int(), "{a=1, b=2, c=3}.b",) << '\n'
    << PRINT_RESULT(int(), "{a=1, b={ c=2, d={ e=3,f=4 }, g=5}, h=6}.b.d.f",) << '\n'
    << PRINT_RESULT(HobbesCharArray(), "(1,\"jimmy\",13L).1",) << '\n'
    << PRINT_RESULT(int(), "(if (0 == 0) then (1,2) else (3,4)).0",) << '\n'

    /**
     Check contents of bob
     * */
    << PRINT_RESULT(bool(), "bob.x == 42",) << '\n'
    << PRINT_RESULT(bool(), "bob.y > 42.0 and bob.y < 43.0",) << '\n'
    << PRINT_RESULT(bool(), "bob.z == \"Hello!\"",) << '\n'
    << PRINT_RESULT(bool(), "bob.w == 'c'",) << '\n'
    << PRINT_RESULT(bool(), "isAlpha(bob)",) << '\n' ///< call the binded function isAlpha
    << std::endl;

    // compiler.compileFn<void()>("print(isAlpha)")() ///< print type of the function isAlpha
  );
  RUN_CODE(
    std::cout << "Before assignment: " << print(bob);
  );  

  /**
   * Assignments
  */
  compiler.compileFn<void()>("bob.x <- 90")();
  compiler.compileFn<void()>("bob.y <- 0.1")();
  compiler.compileFn<void()>("bob.z <- \"Word\"")();
  compiler.compileFn<void()>("bob.w <- '0'")();
  compiler.compileFn<int()>("let _ = [incrementBobW(i, bob) | i <- [0..2]] in 1")(); ///< match [incrementBobW(i, bob) | i <- [0..2]] with | _ -> 1, hence return int
  RUN_CODE(
    std::cout << " After assignment: " << print(bob) << ", "
    << PRINT_RESULT(bool(), "isAlpha(bob)",);
  );

  /**
   * Handling exception
  */
  try {
    compiler.compileFn<void()>("bob.x <- 'c'")();
  }
  catch (const std::exception& e) {
    RUN_CODE(std::cout << "Error message: " << e.what(););
  }

  compiler.define("add1", "(\\x.x+1)::(x)->int"); ///< define a function
  RUN_CODE(std::cout << compiler.compileFn<int(int)>("x", "add1(x)")(3););

  RUN_CODE(std::cout << "Before executing script, bob.x = " << bob.x;);
  compiler.compileFn<void()>(R"foo(
// body of a script
do {
  // this is a comment
  tmp = bob.x * 2; // use "=" to mean "define and assign"
  bob.x <- add1(tmp); // use "<-" to assign
}
)foo")();
  RUN_CODE(std::cout << " After executing script, bob.x = " << bob.x;);

  std::vector<int> xs = {1, 2, 3, 4, 5};
  RUN_CODE(
    std::cout << hobbes::makeStdString(
      compiler.compileFn<
          HobbesCharArray ///< output type
          (const std::vector<int>&) ///< input parameter list
      >("ys", ///< input
        "show(ys)" ///< output of show function is hobbes::array<char>
      )(xs) ///< take in parameters
    );
  );

  RUN_CODE(
    // list comprehension
    std::cout << hobbes::makeStdString(
      compiler.compileFn<HobbesCharArray()>("show([x | x <- [0..30], x % 3 == 0, x % 2 == 0])")()
    );
  );

  compiler.define("matchStrings", R"foo(
(\str1 str2.match str1 str2 with
| _ "two" -> 1
| "abc" _ -> 2
| _ "three" -> 3
| _ _ -> 4) :: (str1,str2)->int
)foo");

  RUN_CODE(std::cout << PRINT_RESULT(int(), "matchStrings(\"abc\", \"three\")",););
  // equivalent to `compiler.compileFn<int()>("matchStrings(\"abc\", \"three\")")()`
  RUN_CODE(std::cout << PRINT_RESULT0("matchStrings(\"aaa\", \"three\")", int(const char*, const char*), CONCATENATE("str1", "str2", "matchStrings(str1, str2)"), CONCATENATE("aaa", "three")););
  // equivalent to compiler.compileFn<int(const char*, const char*)>("str1", "str2", "matchStrings(str1, str2)")("aaa", "three")

  compiler.define("matchIntegers", R"foo(
(\i j.(
  match i j with
  | 0 0 -> "foo"
  | 0 1 -> "foobar"
  | 1 0 -> "bar"
  | 1 1 -> "barbar"
  | 2 0 -> "chicken"
  | 2 1 -> "chicken bar!"
  | _ _ -> "beats me"
)
++ " for " ++ show([i, j])) :: (i,j)->[char]
)foo");
  RUN_CODE(std::cout << "matchIntegers(2, 0) = " << compiler.compileFn<HobbesCharArray(int,int)>("i", "j", "matchIntegers(i, j)")(2, 0););

  compiler.define("extractSubstring", R"foo(
(\str.(
match str with
  | 'f(?<var>u*)bar' -> var   // extract repeated u's into a variable `var`
  | 'f(?<var>o*)bar' -> var   // extract repeated o's into a variable `var`
  | _                -> "???" // not matched
)) :: (str)->[char]
)foo");
  auto extractSubstring = compiler.compileFn<HobbesCharArray(const char*)>("str", "extractSubstring(str)");
  RUN_CODE(
    std::cout << "extractSubstring(\"foobar\") = " << extractSubstring("foobar") << ", "
    << "extractSubstring(\"fuuuubar\") = " << extractSubstring("fuuuubar") // << ", "
  );

  compiler.define("extractEntry", R"foo(
(\i j.match [(1,2),(i,j),(5,6)] with
| [_, (3, x), _] where j >= 5 -> x
| _                           -> -1) :: (i,j)->int
)foo");
  RUN_CODE(
    auto extractEntry = compiler.compileFn<int(int,int)>("i", "j", "extractEntry(i, j)");
    std::cout 
    << "extractEntry(3, 4) = " << extractEntry(3, 4) << ", "
    << "extractEntry(3, 5) = " << extractEntry(3, 5) << ", "
    << "extractEntry(7, 8) = " << extractEntry(7, 8); // << ", "
  );

  RUN_CODE(std::cout << PRINT_RESULT(int(), "let (x, y) = (1, 2) in x + y",););
  /**
   * Equivalent of
   match (1, 2) with
   | (x, y) -> x + y
  */

  compile(
    &compiler,
    compiler.readModule(R"foo(
bob = 42
type BT = (TypeOf `bob` x) => x
type BTI = (TypeOf `newPrim()::BT` x) => x
frank :: BTI
frank = 3
)foo"));
  RUN_CODE(
      RUN_CODE(std::cout << PRINT_RESULT(int(), "frank",););
  );

  return 0;
}

