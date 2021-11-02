 

// experimenting with some stuff to replace ArFunctor

// note current version requires C++17 for template deduction guide support


template <typename C, typename R, typename... Args>
class ArFunc
{
 public:
  ArFunc(C& obj, R(C::*fp)(Args...)) : myObj(obj), myConstFP(nullptr), myFP(fp) {}
  ArFunc(C& obj, R(C::*fp)(Args...) const) : myObj(obj), myConstFP(fp), myFP(nullptr) {}
  R invoke(Args... args) {
    // TODO this can probably be done better with more clever
    //template programming, or by specializing from a base ArFunc with function
    //signature as a template type parameter rather than runtime check for different
    //function pointers here -- see below.
    if(myConstFP) 
      return (myObj.*myConstFP)(args...); // should this be std::forward<Args>(args)...?
    else
      return (myObj.*myFP)(args...); // should this be std::forward<Args>(args)...?
  }
  R invokeR(Args... args) {
    return invoke(args...); // should this be std::forward<Args>(args)...?
  }
 private:
  C& myObj;
  R(C::*myConstFP)(Args...) const;
  R(C::*myFP)(Args...);
};



// ArFunc specialization for free (global) functions (i.e. not a member
// function/method).  Template deduction guide follows to select this when 
// ArFunc constructor with one argument with function pointer is used.
template <typename R, typename... Args>
class ArFunc <void, R, Args...>
{
public:
  ArFunc(R(*fp)(Args...)) : myFP(fp) {}
  R invoke(Args... args) {
     return (*myFP)(args...); // should this be std::forward<Args>(args)...?
  }
  R invokeR(Args... args) {
    return invoke(args...); // should this be std::forward<Args>(args)...?
  }
private:
  R(*myFP)(Args...);
};

template <typename R, typename... Args> ArFunc(R(*)(Args...)) -> ArFunc<void, R, Args...>; // note this deduction guide requires c++17.

// type aliases for compatibility with Aria "ArFunctor" types
template <class C, typename P1> using ArFunctor1C = ArFunc<C, void, P1>;
template <typename Ret, class C, typename P1> using ArRetFunctor1C = ArFunc<C, Ret, P1>;
template <typename P1> using ArFunctor1 = ArFunc<void, P1>;
template <typename P1> using ArGlobalFunctor1 = ArFunc<void, P1>;

template <class C, typename P1, typename P2> using ArFunctor2C = ArFunc<C, void, P1, P2>;
template <typename Ret, class C, typename P1, typename P2>  using ArRetFunctor2C = ArFunc<C, Ret, P1, P2>;
template <typename P1, typename P2> using ArFunctor2 = ArFunc<void, P1, P2>;
template <typename P1, typename P2> using ArGlobalFunctor2 = ArFunc<void, P1, P2>;

int f(int x) { return x; }
void g(int x) {}

class C {
  public:
  int f(float y) { return 23; }
  int g(float y) const { return 42; }
};

int main()
{
  ArFunc f1(&f);  
  int x = f1.invoke(23);
  ArFunc f2(&g);
  f2.invoke(42);
  C c;
  ArFunc f3(c, &C::f);
  int y = f3.invoke(3.14159f);
  ArFunc<C, int, float> f4(c, &C::f);
  int z = f4.invoke(0.1f);
  ArFunc f6(c, &C::g);
  int w = f6.invoke(33.3333f);
}


#if 0

// here is another way, in which the base template uses a full function signature type instead of separate return and argument types,
// which means we can also specialize for const member functions (rather than keeping separate runtime function pointers).
// but this might make compatibility type aliases with old arfunctor types harder??
template <class C, typename F>
class Foo
{};

template <class C, typename R, typename... Args>
class Foo <C, R(C::*)(Args...)>
{
 public:
  Foo(C& obj, R(C::*fp)(Args...)) : myObj(obj), myFP(fp) {}
  //Foo(C& obj, R(C::*fp)(Args...) const) : myObj(obj), myConstFP(fp), myFP(nullptr) {}
  R invoke(Args... args) {
    return (myObj.*myFP)(args...);
  }
 private:
  C& myObj;
  R(C::*myConstFP)(Args...) const;
  R(C::*myFP)(Args...);
};

template <class C, typename R, typename... Args>
class Foo<C, R(C::*)(Args...) const>
{
public:
  Foo(C& obj, R(C::*fp)(Args...) const) : myObj(obj), myFP(fp) {}
  R invoke(Args... args) {
    return (myObj.*myFP)(args...);
  }
private:
  C& myObj;
  R(C::*myFP)(Args...) const;
};


template <typename R, typename... Args>
class Foo <void, R(*)(Args...)>
{
public:
  Foo(R(*fp)(Args...)) : myFP(fp) {}
  R invoke(Args... args) {
     return (*myFP)(args...);
  }
private:
  R(*myFP)(Args...);
};

template <typename R, typename... Args> Foo(R(*)(Args...)) -> Foo<void, R(*)(Args...)>; // note this deduction guide requires c++17.
template <class C, typename R, typename... Args> Foo(C&, R(C::*)(Args...)) -> Foo<C, R(C::*)(Args...)>;
template <class C, typename R, typename... Args> Foo(C&, R(C::*)(Args...) const) -> Foo<C, R(C::*)(Args...) const>;


int f(int x);
void g(int x);

class C {
  public:
  int f(float y);
  int g(float y) const;
};

int main()
{
  Foo f1(&f);
  int x = f1.invoke(23);
  Foo f2(&g);
  f2.invoke(42);
  C c;
  Foo f3(c, &C::f);
  int y = f3.invoke(3.14159);
  Foo<C, int(C::*)(float)> f4(c, &C::f);
  int z = f4.invoke(0.1);
  //Foo2 f5(c, &C::g);
  //int w = f5.invoke(33.3333);
  Foo f6(c, &C::g);
  int w = f6.invoke(33.3333);
}
#endif


# if 0 

prior attempt


#include <memory> // for shared_ptr if used

// todo how to handle const functions? need to use separate class (like ARIA), or 
// we need to capture the total function type in the constructor, but then also 
// be able to break out Args... and Ret type names for invoke()...

template <typename Ret, typename... Args>
//template <typename FunctionType>
class ArFunc
{
public:
  using FunctionPtrT = Ret(*)(Args...);
  ArFunc(FunctionPtrT fp) : myFP(fp) {}
  //ArFunc(FunctionType *fp) : myFP(fp) {}
  virtual Ret invoke(Args... args) { return (*myFP)(args...); }
private:
  FunctionPtrT myFP;
  //FunctionType *myFP;
};

// todo use type traits to test that C is class?
// todo find some way to make this a specialization of ArFunctor<> so we don't need to
// use two class names?
template <class C, typename Ret, typename... Args>
//template <class C, typename FunctionType>
class ArMemFunc : 
  public virtual ArFunc<Ret, Args...>
  //public virtual ArFunc<FunctionType>
{
private:
public:
  using FunctionPtrT = Ret(C::*)(Args...);
  ArMemFunc(C* obj, FunctionPtrT fp) : ArFunc<Ret, Args...>(nullptr), myFP(fp), myObj(obj) {}
  //ArMemFunc(C* obj, FunctionType *fp) : ArFunc<FunctionType>(fp), myObj(obj) {}
  virtual Ret invoke(Args... args) override { return (myObj->*myFP)(args...);  }
private:
  FunctionPtrT myFP;// todo move to ArFunctor and pass FunctionPtrT as template parameter?
  C* myObj; // todo use shared_ptr?
};

// compatibility
template <class C, typename P1> using ArFunctor1C = ArMemFunc<C, void, P1>;
template <typename Ret, class C, typename P1> using ArRetFunctor1C = ArMemFunc<C, Ret, P1>;
template <typename P1> using ArFunctor1 = ArFunc<void, P1>;
template <typename P1> using ArGlobalFunctor1 = ArFunc<void, P1>;

template <class C, typename P1, typename P2> using ArFunctor2C = ArMemFunc<C, void, P1, P2>;
template <typename Ret, class C, typename P1, typename P2>  using ArRetFunctor2C = ArMemFunc<C, Ret, P1, P2>;
template <typename P1, typename P2> using ArFunctor2 = ArFunc<void, P1, P2>;
template <typename P1, typename P2> using ArGlobalFunctor2 = ArFunc<void, P1, P2>;

template <class C, typename P1> using ArConstFunctor1C = ArMemFunc<C, void, P1>;

int f1(float x, float y);
void f2(float x);

class Test {
 public:
  int m1(float x, float y);
  void m2(float x);
  void m3() const;
};

int main()
{
  Test obj;
  ArMemFunc af1(&obj, &Test::m1);
  int a = af1.invoke(23.23, 3.1415);
  ArMemFunc af2(&obj, &Test::m2);
  af2.invoke(0.1);

  ArFunc af3(&f1);
  int b = af3.invoke(33.3333, 0.0); 
  ArFunc af4(&f2);
  af4.invoke(123.456);
  ArFunc<int, float, float> *f = &af1;
  int c = f->invoke(2.3, 4.9);
    
  ArMemFunc<Test, void, float> af5(&obj, &Test::m2);
  //ArMemFunc<Test, void(float)> af5(&obj, &Test::m2);
  af5.invoke(99.999);
  ArMemFunc<Test, int, float, float> af6(&obj, &Test::m1);
  //ArMemFunc<Test, int(float, float)> af6(&obj, &Test::m1);
  int d = af6.invoke(0.0, 0.0);
  
  ArRetFunctor2C<int, Test, float, float> af7(&obj, &Test::m1);
  int e = af7.invoke(1, 2);

  ArMemFunc<Test, void(void) const> af8(&obj, &Test::m3);
  af8.invoke();
}

#endif
