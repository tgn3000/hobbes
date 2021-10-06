#include <iostream>
#include <sstream>
#include <hobbes/hobbes.H>
#include "example.H"

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

// DEFINE_STRUCT(
//   DynStructTest,
//   (bool,     f),
//   (uint64_t, a),
//   (uint64_t, b)
// );

// typedef short short3[3];
// DEFINE_STRUCT(
//   NM,
//   (unsigned long, sz),
//   (short3,        ss)
// );

// DEFINE_STRUCT(
//   M0,
//   (NM,            c1),
//   (unsigned char, c2)
// );

// typedef NM NM3[3];
// DEFINE_STRUCT(
//   M,
//   (unsigned long, sz),
//   (NM3,           ss)
// );

// typedef char char25[25];
// DEFINE_STRUCT(
//   GenStruct,
//   (int,    f0),
//   (bool,   f1),
//   (double, f2),
//   (char25, f3)
// );

struct Compiler
{
  static hobbes::cc& instance() {
    static hobbes::cc compiler; // known as cc
    return compiler;
  }

  static void init() {
    if(isInitialized) return;
    auto& compiler { instance() };
    bob.x = 42;
    bob.y = 42.2;
    bob.z = "Hello!";
    bob.w = 'c';
    compiler.bind("bob", &bob); ///< binds a Bob object to an instance within the compiler
    compiler.bind("isAlpha", &isAlpha); ///< binds a function

    isInitialized = true;

    // verify some odd alignment cases
    // static M m;
    // x.bind(".align_m", &m);

    // verify binding correctness in general structs (try to cover odd cases here)
    // static GenStruct gs;
    // x.bind("genstruct", &gs);    
  }

  static const Bob& Bob() { return bob; }
  static bool isInitialized;
  static struct Bob bob;
};

template<typename T>
std::string print(const T&) { return ""; }

template<>
std::string print(const Bob& bob) {
  std::stringstream ss;
  ss << "x = " << bob.x << ", y = " << bob.y << ", z = " << bob.z << ", w = " << bob.w;
  return ss.str();
}

bool Compiler::isInitialized = false;
Bob Compiler::bob {
  .x = 42, .y = 42.2, .z = "Hello!", .w = 'c'
};

// static hobbes::cc& c() {
//   static hobbes::cc x;
//     // verify some odd alignment cases
//     static M m;
//     x.bind(".align_m", &m);
//     // verify binding correctness in general structs (try to cover odd cases here)
//     static GenStruct gs;
//     x.bind("genstruct", &gs);
    //   return x;
// }

int main() {
  Compiler::init();
  auto& compiler = Compiler::instance();
  auto& bob = Compiler::Bob();

  RUN_CODE(
    std::cout << std::boolalpha
    /**
     Extract value of a map
     */
    << compiler.compileFn<int()>("{a=1,b=2,c=3}.b")() << ", "
    /**
     Check contents of bob
     * */
    << compiler.compileFn<bool()>("bob.x == 42")() << ", "
    << compiler.compileFn<bool()>("bob.y > 42.0 and bob.y < 43.0")() << ", "
    << compiler.compileFn<bool()>("bob.z == \"Hello!\"")() << ", "
    << compiler.compileFn<bool()>("bob.w == 'c'")() << ", "
    << compiler.compileFn<bool()>("isAlpha(bob)")() << ", "
    << std::endl;
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
  compiler.compileFn<void()>("let _ = [bob.w <- '0' | _ <- [0..10]] in ()")();
  RUN_CODE(
    std::cout << " After assignment: " << print(bob) << ", "
    << compiler.compileFn<bool()>("isAlpha(bob)")();
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

  std::vector<int> xs = {1, 2, 3, 4, 5};
  RUN_CODE(
    std::cout << hobbes::makeStdString(
      compiler.compileFn<
          const hobbes::array<char>* ///< output type
          (const std::vector<int>&) ///< input parameter list
      >("ys", ///< input
        "show(ys)" ///< output is hobbes::array<char>
      )(xs) ///< take in parameters
    );
  );

  RUN_CODE(
    // list comprehension
    std::cout << hobbes::makeStdString(
      compiler.compileFn<const hobbes::array<char>*()>("show([x | x <- [11, 12, 13], x % 3 == 0])")()
    );
  );

//   RUN_CODE(std::cout << compiler.compileFn<int()>(
//       R"foo(match "abc" "three" with
// | _ "two" -> 1
// | "abc" _ -> 2
// | _ "three" -> 3
// | _ _ -> 4)foo")();
//   );

  compiler.define("matchStrings", R"foo(
(\str1 str2.match str1 str2 with
| _ "two" -> 1
| "abc" _ -> 2
| _ "three" -> 3
| _ _ -> 4) :: (str1,str2)->int
)foo");

  RUN_CODE(std::cout << compiler.compileFn<int()>("matchStrings(\"abc\", \"three\")")(););
  RUN_CODE(std::cout << compiler.compileFn<int(const char*, const char*)>("str1", "str2", "matchStrings(str1, str2)")("abc", "three"););

  compiler.define("matchIntegers", R"foo(
(\i j.match i j with
| 0 0 -> "foo"
| 0 1 -> "foobar"
| 1 0 -> "bar"
| 1 1 -> "barbar"
| 2 0 -> "chicken"
| 2 1 -> "chicken bar!"
| _ _ -> "beats me") :: (x,y)->[char]
)foo");
  RUN_CODE(std::cout << compiler.compileFn<const hobbes::array<char>*(int,int)>("i", "j", "matchIntegers(i, j)")(2, 0););
  return 0;
}

// TEST(Structs, Consts) {
//   EXPECT_EQ(c().compileFn<int()>("{a=1,b=2,c=3}.b")(), 2);
//   EXPECT_EQ(c().compileFn<int()>("{a=1,b={c=2,d={e=3,f=4},g=5},h=6}.b.d.f")(), 4);
//   EXPECT_TRUE(c().compileFn<bool()>("(1,\"jimmy\",13L).1 == \"jimmy\"")());
//   EXPECT_TRUE(c().compileFn<bool()>("(if (0 == 0) then (1,2) else (3,4)).0 == 1")());

//   typedef std::pair<unsigned char, double> FWeight;
//   typedef array<FWeight>                   FWeights;
//   EXPECT_EQ(c().compileFn<FWeights*()>("[(0XAE, 1.2),(0XBD, 2.4),(0XFF, 5.9)]")()->data[1].second, 2.4);
// }

// TEST(Structs, Reflect) {
//   EXPECT_EQ(c().compileFn<DynStructTest*()>("{f=true, a=42L, b=100L}")()->a, uint64_t(42));
// }

// TEST(Structs, Functions) {
//   EXPECT_EQ(c().compileFn<int()>("(\\x.{a=1,b=x,c=9})(10).b")(), 10);
//   EXPECT_EQ(c().compileFn<int()>("(\\r.10+r.x)({y=1,x=100})")(), 110);
// }

// TEST(Structs, Bindings) {
//   EXPECT_TRUE(c().compileFn<bool()>("bob.x == 42")());
//   EXPECT_TRUE(c().compileFn<bool()>("bob.y > 42.0 and bob.y < 43.0")());
//   EXPECT_TRUE(c().compileFn<bool()>("bob.z == \"Hello!\"")());
//   EXPECT_TRUE(c().compileFn<bool()>("bob.w == 'c'")());

//   EXPECT_TRUE(c().compileFn<array<char>*()>("show(genstruct)") != 0);
// }

// TEST(Structs, NoDuplicateFieldNames) {
//   // reject outright construction of structs with duplicate field names
//   bool introExn = false;
//   try {
//     c().compileFn<int()>("{x=1, x='c'}.x");
//   } catch (std::exception&) {
//     introExn = true;
//   }
//   EXPECT_TRUE( introExn && "Expected rejection of record introduction with duplicate field names" );

//   // reject roundabout construction of structs with duplicate field names
//   introExn = false;
//   try {
//     c().compileFn<void()>("print(recordAppend({x=1},{x=2}))");
//   } catch (std::exception&) {
//     introExn = true;
//   }
//   EXPECT_TRUE( introExn && "Expected rejection of roundabout record introduction with duplicate field names" );
// }

// TEST(Structs, Assignment) {
//   bool assignExn = false;
//   try {
//     c().compileFn<void()>("bob.x <- 'c'")();
//   } catch (std::exception&) {
//     assignExn = true;
//   }
//   EXPECT_TRUE( assignExn && "Expected char assignment to int field to fail to compile (mistake in assignment compilation?)" );

//   c().compileFn<void()>("bob.x <- 90")();
//   EXPECT_TRUE(c().compileFn<bool()>("bob.x == 90")());

//   c().compileFn<void()>("bob.y <- 0.1")();
//   EXPECT_TRUE(c().compileFn<bool()>("bob.y > 0.0 and bob.y < 1.0")());

//   // assignment between std::string and [char]
//   c().compileFn<void()>("bob.z <- \"Word\"")();
//   EXPECT_TRUE(c().compileFn<bool()>("bob.z == \"Word\"")());

//   // this one will also test correctness of assignment within array comprehensions
//   c().compileFn<void()>("let _ = [bob.w <- 'a' | _ <- [0..10]] in ()")();
//   EXPECT_TRUE(c().compileFn<bool()>("bob.w == 'a'")());
// }

// TEST(Structs, Objects) {
//   // test array construction of opaque pointers inside structures
//   try {
//     c().compileFn<void()>("let s = show([bob.z | _ <- [0..10]]) in ()")();
//   } catch (std::exception& ex) {
//     std::cout << ex.what() << std::endl;
//     EXPECT_TRUE(false && "Array construction over opaque pointer struct members failed.");
//   }

//   // test record/tuple construction of opaque pointers into structures
//   EXPECT_TRUE(c().compileFn<bool()>("(bob.z, bob.z) === (bob.z, bob.z)")());
// }

// TEST(Structs, Prelude) {
//   // test prelude generic struct functions (equality/show)
//   EXPECT_TRUE(c().compileFn<bool()>("(1, 'c', 3L) == (1, 'c', 3L)")());
//   EXPECT_TRUE(c().compileFn<bool()>("show((1,'c',3L)) == \"(1, 'c', 3)\"")());
//   EXPECT_TRUE(c().compileFn<bool()>("{x=1, y='c', z=3L} == {x=1, y='c', z=3L}")());

//   // test structural subtyping conversion for records
//   EXPECT_EQ(c().compileFn<int()>("(convert({w=3,x=0xdeadbeef,y=42S,z=4.45}) :: {y:int,z:double,w:long}).y")(), 42);

//   // test ordering in structs and tuples
//   EXPECT_TRUE(c().compileFn<bool()>("(1,2,3) < (1,2,4)")());
//   EXPECT_TRUE(c().compileFn<bool()>("(1,2,3) <= (1,2,3)")());
//   EXPECT_TRUE(c().compileFn<bool()>("{x=1,y=2} < {x=1,y=3}")());
//   EXPECT_TRUE(c().compileFn<bool()>("{x=1,y=2} <= {x=1,y=2}")());
// }

// // weird alignment tests
// typedef char l1ft[48];
// DEFINE_STRUCT(AlignA,
//   (l1ft, data)
// );

// typedef char ibft[50];
// DEFINE_STRUCT(AlignB,
//   (ibft, data)
// );

// TEST(Structs, LiftTuple) {
//   hobbes::tuple<short, int, std::string> p(42, 314159, "jimmy");
//   EXPECT_TRUE(c().compileFn<bool(const hobbes::tuple<short,int,std::string>&)>("p", "p==(42,314159,\"jimmy\")")(p));
// }

// typedef variant<unit, AlignA> MaybeAlignA;
// typedef variant<unit, AlignB> MaybeAlignB;

// DEFINE_STRUCT(AlignTest,
//   (MaybeAlignA, ma),
//   (MaybeAlignB, mb)
// );
// const AlignTest* makeAT() { return new AlignTest(); }

// struct PadTest {
//   int  x;
//   long y;
// };

// TEST(Structs, Alignment) {
//   c().bind("makeAT", &makeAT);
//   EXPECT_TRUE(true);

//   Record::Members ms;
//   ms.push_back(Record::Member("x", lift<int>::type(c()), 0));
//   ms.push_back(Record::Member("y", lift<long>::type(c()), static_cast<int>(reinterpret_cast<size_t>(&reinterpret_cast<PadTest*>(0)->y))));
//   MonoTypePtr pty(Record::make(Record::withExplicitPadding(ms)));

//   PadTest p;
//   p.x = 1; p.y = 42;
//   c().bind(polytype(pty), "padTest", &p);
//   EXPECT_EQ(makeStdString(c().compileFn<const array<char>*()>("show(padTest)")()), "{x=1, y=42}");
// }

