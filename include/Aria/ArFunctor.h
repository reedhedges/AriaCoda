/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2015 Adept Technology, Inc.
Copyright (C) 2016-2018 Omron Adept Technologies, Inc.

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


*/
#ifndef ARFUNCTOR_H
#define ARFUNCTOR_H

#include "Aria/ariaTypedefs.h"
#include "Aria/ariaOSDef.h"
#include <stdarg.h>
#include <stdio.h>

/// An object which stores (binds) a reference to an object instance, and a
/// function (method) to call with that object instance, usually used for
/// callback functions.  
/**
  This base class and subclasses implements the idea of a pointer to a function
  which is a member of a class, bound to a specific instance of this class. 
  Subclasses are provided for different numbers of
  function arguments, const and non-const types, and compatible base classes
  for global functions (not object methods).    

  To call a bound function with no return type, use invoke(). To receive the
  return type, use invokeR().  

  ArFunctor does not really 
  implement a true "ArFunctor", but it's too late to change the name.

  @note When supplying the function pointer to the constructor,
  make sure to pass the function pointer using "&" (pointer reference
  syntax).  When supplying a method, make sure to qualify the method
  name with the class name as well.  For example:
  @code 
    ArFunctorC<int> example(&obj, &ExampleClass::exampleMethod);
  @endcode
  otherwise you will receive compile errors such as "invalid use of
  member function", or that the function was not declared.

  All the derived
  ArFunctor classes have the ability to invoke the correct function on the correct
  object.

  Because functions have different signatures because they take different
  types of parameters and have different number of parameters, templates
  were used to create the ArFunctors. These are the base classes for the
  ArFunctors. These classes encapsulate everything except for the class
  type that the member function is a member of. This allows someone to
  accept a ArFunctor of type ArFunctor1<int> which has one parameter of type
  'int'. But they never have to know that the function is a member function
  of class 'SomeUnknownType'. These classes are:

  ArFunctor, ArFunctor1, ArFunctor2, ArFunctor3
  ArRetFunctor, ArRetFunctor1, ArRetFunctor2, ArRetFunctor3

  These 8 ArFunctors are the only thing a piece of code that wants a ArFunctor
  will ever need. But these classes are abstract classes and can not be
  instantiated. On the other side, the piece of code that wants to be
  called back will need the ArFunctor classes that know about the class type.
  These ArFunctors are:

  ArFunctorC, ArFunctor1C, ArFunctor2C, ArFunctor3C
  ArRetFunctorC, ArRetFunctor1C, ArRetFunctor2C, ArRetFunctor3C

  These ArFunctors are meant to be instantiated and passed of to a piece of
  code that wants to use them. That piece of code should only know the
  ArFunctor as one of the ArFunctor classes without the 'C' in it.

  Note that you can create these ArFunctorC instances with default
  argument values that are then used when invoked is called without those
  arguments.  
  For example, if you have an interface that
  expects an ArFunctor object (no function arguments), you can instead provide
  an ArFunctor1C instance (since ArFunctor1C derives from ArFunctor), but set a
  parameter value (via the constructor, or the setP1() method.) 

  ArFunctors now have a getName() method, this is useful as an aid to debugging,
  allowing you to display the name of some ArFunctor being used.

  @javanote You can subclass ArFunctor and override invoke().

  @see @ref ArFunctorExample.cpp

  @todo Missing move constructors/assignments and copy constructors. These would be a pain to add to all variations of ArFunctor. ArFunctor will eventually be deprecated and replaced with more generic and standards based function objects.

  @todo Deprecate ArFunctor and replace all uses with std::function or other
    standard C++ object for binding a function and instance.  (Or store a
    std::function or other callable object, with the debugging name and other
    context.)

  @ingroup ImportantClasses
**/
class ArFunctor
{
public:

  
   ArFunctor() = default;

   virtual ~ArFunctor() = default;
  
   /// Call the function (method) on the object
   virtual void invoke() = 0;

   /// Gets a name for this ArFunctor object, for diagnostic purposes
   virtual const char *getName() { return myName.c_str(); }

   /// Sets the name
   virtual void setName(const char *name) { myName = name; }
   virtual void setName(const std::string& name) { myName = name; }

#ifndef SWIG
  /// Sets the name with formatting
  /** @swigomit use setName() */
  virtual void setNameVar(const char *name, ...) 
    { 
      char arg[2048];
      va_list ptr;
      va_start(ptr, name);
      vsnprintf(arg, sizeof(arg), name, ptr);
      arg[sizeof(arg) - 1] = '\0';
      va_end(ptr);
      return setName(arg);
    }
#endif

protected:
  std::string myName;
};

/// Base class for functions with 1 parameter
template<class P1>
class ArFunctor1 : public ArFunctor
{
public:
  virtual ~ArFunctor1()  = default;
  /// The parameter value is default initialized
  virtual void invoke() override = 0;
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) = 0;
};

/// Base class for functions with 2 parameters
template<class P1, class P2>
class ArFunctor2 : public ArFunctor1<P1>
{
public:
  virtual ~ArFunctor2()  = default;
  /// All parameter values are default initialized
  virtual void invoke() override = 0;
  /**
     The second parameter value is default initialized
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) override = 0;
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) = 0;
};

/// Base class for function with 3 parameters
template<class P1, class P2, class P3>
class ArFunctor3 : public ArFunctor2<P1, P2>
{
public:
  virtual ~ArFunctor3()  = default;
  virtual void invoke() override = 0;
  virtual void invoke(P1 p1) override = 0;
  virtual void invoke(P1 p1, P2 p2) override = 0;
  virtual void invoke(P1 p1, P2 p2, P3 p3) = 0;
};



/// Base class for functions with 4 parameters
template<class P1, class P2, class P3, class P4>
class ArFunctor4 : public ArFunctor3<P1, P2, P3>
{
public:
  virtual ~ArFunctor4()  = default;
  virtual void invoke() override = 0;
  virtual void invoke(P1 p1) override = 0;
  virtual void invoke(P1 p1, P2 p2) override = 0;
  virtual void invoke(P1 p1, P2 p2, P3 p3) override = 0;
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) = 0;
};


/// Base class for functions with 4 parameters
template<class P1, class P2, class P3, class P4, class P5>
class ArFunctor5 : public ArFunctor4<P1, P2, P3, P4>
{
public:
  virtual ~ArFunctor5()  = default;
  virtual void invoke() override = 0;
  virtual void invoke(P1 p1) override = 0;
  virtual void invoke(P1 p1, P2 p2) override = 0;
  virtual void invoke(P1 p1, P2 p2, P3 p3) override = 0;
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) override = 0;
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) = 0;
};




/// Base class for functions with a return value
/**
   Code invoking or storing an ArFunctor with a function that returns a value
   should use this base class, and call invokeR() to receive the return value.
   
   For an overall description, see ArFunctor.

   @javanote To create the equivalent of ArRetFunctor<bool>, you can 
      subclass <code>ArRetFunctor_Bool</code> and override <code>invoke(bool)</code>
     
*/
template<class Ret>
class ArRetFunctor : public ArFunctor
{
public:
  virtual ~ArRetFunctor()  = default;
  /// discards any return value
  virtual void invoke() override {
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4834) // disable MSVC warning about invokeR() having NODISCARD
    invokeR();
#pragma warning(pop)
#else
    invokeR();
#endif
  }
  NODISCARD virtual Ret invokeR() = 0;
};

template<class Ret, class P1>
class ArRetFunctor1 : public ArRetFunctor<Ret>
{
public:
  virtual ~ArRetFunctor1()  = default;
  NODISCARD virtual Ret invokeR() override = 0;
  NODISCARD virtual Ret invokeR(P1 p1) = 0;
};

template<class Ret, class P1, class P2>
class ArRetFunctor2 : public ArRetFunctor1<Ret, P1>
{
public:
  virtual ~ArRetFunctor2()  = default;
  NODISCARD virtual Ret invokeR() override = 0;
  NODISCARD virtual Ret invokeR(P1 p1) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2) = 0;
};

/// Base class for ArFunctors with a return value with 3 parameters
template<class Ret, class P1, class P2, class P3>
class ArRetFunctor3 : public ArRetFunctor2<Ret, P1, P2>
{
public:
  virtual ~ArRetFunctor3()  = default;
  NODISCARD virtual Ret invokeR() override = 0;
  NODISCARD virtual Ret invokeR(P1 p1) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) = 0;
};


template<class Ret, class P1, class P2, class P3, class P4>
class ArRetFunctor4 : public ArRetFunctor3<Ret, P1, P2, P3>
{
public:
  virtual ~ArRetFunctor4()  = default;
  NODISCARD virtual Ret invokeR() override = 0;
  NODISCARD virtual Ret invokeR(P1 p1) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) = 0;
};

/// Base class for ArFunctors with a return value with 5 parameters
template<class Ret, class P1, class P2, class P3, class P4, class P5>
class ArRetFunctor5 : public ArRetFunctor4<Ret, P1, P2, P3, P4>
{
public:
  virtual ~ArRetFunctor5()  = default;
  NODISCARD virtual Ret invokeR() override = 0;
  NODISCARD virtual Ret invokeR(P1 p1) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) override = 0;
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) = 0;
};





//////
////// ArFunctors for global functions. C style function pointers.
//////


#ifndef SWIG
/// ArFunctor for a global function with no parameters
/**
   This is a class for global functions. This ties a C style function
   pointer into the ArFunctor class hierarchy as a convience. 
   
   For an overall description of ArFunctor, see ArFunctor.

   @swigomit
*/
class ArGlobalFunctor : public ArFunctor
{
public:
  ArGlobalFunctor(void (*func)()) : myFunc(func) {}
  virtual ~ArGlobalFunctor()  = default;
  virtual void invoke() override {(*myFunc)();}
protected:

  void (*myFunc)();
};

/// ArFunctor for a global function with 1 parameter
template<class P1>
class ArGlobalFunctor1 : public ArFunctor1<P1>
{
public:
  ArGlobalFunctor1(void (*func)(P1)) :
    myFunc(func), myP1() {}
  ArGlobalFunctor1(void (*func)(P1), P1 p1) :
    myFunc(func), myP1(p1) {}
  virtual ~ArGlobalFunctor1()  = default;
  [[deprecated]] virtual void invoke() override {(*myFunc)(myP1);}
  virtual void invoke(P1 p1) override {(*myFunc)(p1);}
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}
protected:
  void (*myFunc)(P1);
  P1 myP1;
};


/// ArFunctor for a global function with 1 parameter which may be a const type
template<class P1>
class ArGlobalFunctor1Const : public ArFunctor1<P1>
{
public:
  ArGlobalFunctor1Const() {}
  ArGlobalFunctor1Const(void (*func)(P1)) :
    myFunc(func) {}
  virtual ~ArGlobalFunctor1Const()  = default;
  virtual void invoke(P1 p1) override {(*myFunc)(p1);}
protected:
  void (*myFunc)(P1);
};


/// ArFunctor for a global function with 2 parameters
template<class P1, class P2>
class ArGlobalFunctor2 : public ArFunctor2<P1, P2>
{
public:
  ArGlobalFunctor2(void (*func)(P1, P2)) :
    myFunc(func), myP1(), myP2() {}
  ArGlobalFunctor2(void (*func)(P1, P2), P1 p1) :
    myFunc(func), myP1(p1), myP2() {}
  ArGlobalFunctor2(void (*func)(P1, P2), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2) {}
  virtual ~ArGlobalFunctor2()  = default;
  [[deprecated]] virtual void invoke() override {(*myFunc)(myP1, myP2);}
  [[deprecated]] virtual void invoke(P1 p1) override {(*myFunc)(p1, myP2);}
  virtual void invoke(P1 p1, P2 p2) override {(*myFunc)(p1, p2);}
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}
protected:
  void (*myFunc)(P1, P2);
  P1 myP1;
  P2 myP2;
};

/// ArFunctor for a global function with 3 parameters
template<class P1, class P2, class P3>
class ArGlobalFunctor3 : public ArFunctor3<P1, P2, P3>
{
public:
  ArGlobalFunctor3(void (*func)(P1, P2, P3)) :
    myFunc(func), myP1(), myP2(), myP3() {}
  ArGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1) :
    myFunc(func), myP1(p1), myP2(), myP3() {}
  ArGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2), myP3() {}
  ArGlobalFunctor3(void (*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}
  virtual ~ArGlobalFunctor3()  = default;
  [[deprecated]] virtual void invoke() override {(*myFunc)(myP1, myP2, myP3);}
  [[deprecated]] virtual void invoke(P1 p1) override {(*myFunc)(p1, myP2, myP3);}
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(*myFunc)(p1, p2, myP3);}
  virtual void invoke(P1 p1, P2 p2, P3 p3) override {(*myFunc)(p1, p2, p3);}
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}
protected:
  void (*myFunc)(P1, P2, P3);
  P1 myP1;
  P2 myP2;
  P3 myP3;
};



/// ArFunctor for a global function with 4 parameters
template<class P1, class P2, class P3, class P4>
class ArGlobalFunctor4 : public ArFunctor4<P1, P2, P3, P4>
{
public:
   ArGlobalFunctor4() = default;
   ArGlobalFunctor4(void (*func)(P1, P2, P3, P4)) : myFunc(func), myP1(), myP2(), myP3(), myP4() {}
   ArGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1) : myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}
   ArGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2) : myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}
   ArGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) : myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}
 ArGlobalFunctor4(void (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}
  virtual ~ArGlobalFunctor4()  = default;
  [[deprecated]] virtual void invoke() override {(*myFunc)(myP1, myP2, myP3, myP4);}
  [[deprecated]] virtual void invoke(P1 p1) override {(*myFunc)(p1, myP2, myP3, myP4);}
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(*myFunc)(p1, p2, myP3, myP4);}
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3) override {(*myFunc)(p1, p2, p3, myP4);}
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) override {(*myFunc)(p1, p2, p3, p4);}
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}
protected:
  void (*myFunc)(P1, P2, P3, P4);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};


/// ArFunctor for a global function with 5 parameters
template<class P1, class P2, class P3, class P4, class P5>
class ArGlobalFunctor5 : public ArFunctor5<P1, P2, P3, P4, P5>
{
public:
  ArGlobalFunctor5() = default;
  ArGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}
  ArGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}
  ArGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}
  ArGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}
  ArGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  ArGlobalFunctor5(void (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}
  virtual ~ArGlobalFunctor5()  = default;
  [[deprecated]] virtual void invoke() override {(*myFunc)(myP1, myP2, myP3, myP4, myP5);}
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(*myFunc)(p1, myP2, myP3, myP4, myP5);}
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(*myFunc)(p1, p2, myP3, myP4, myP5);}
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3) override {(*myFunc)(p1, p2, p3, myP4, myP5);}
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) override {(*myFunc)(p1, p2, p3, p4, myP5);}

  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) override {(*myFunc)(p1, p2, p3, p4, p5);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

  /// Set the default fifth parameter
  /**
     @param p5 default fifth parameter
  */
  [[deprecated]] virtual void setP5(P5 p5) {myP5=p5;}

protected:

  void (*myFunc)(P1, P2, P3, P4, P5);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};

#endif // Omitting ArGlobalFunctor from Swig




//////
////// ArFunctors for global functions, C style function pointers with return
////// return values.
//////

#ifndef SWIG

/// ArFunctor for a global function with return value
/**
   This is a class for global functions which return a value. This ties
   a C style function pointer into the ArFunctor class hierarchy as a
   convience. Code that has a reference to this class and treat it as
   an ArFunctor can use it like any other ArFunctor.
   
   For an overall description of ArFunctor, see ArFunctor.
*/
template<class Ret>
class ArGlobalRetFunctor : public ArRetFunctor<Ret>
{
public:
  ArGlobalRetFunctor() = default;
  /**
     @param func global function pointer
  */
  ArGlobalRetFunctor(Ret (*func)()) : myFunc(func) {}

  virtual ~ArGlobalRetFunctor()  = default;

  /// Invokes the ArFunctor with return value
  NODISCARD virtual Ret invokeR() override {return (*myFunc)();}

protected:

  Ret (*myFunc)();
};


/// ArFunctor for a global function with 1 parameter and return value
template<class Ret, class P1>
class ArGlobalRetFunctor1 : public ArRetFunctor1<Ret, P1>
{
public:
  /// Constructor
  ArGlobalRetFunctor1() = default;

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  ArGlobalRetFunctor1(Ret (*func)(P1)) :
    myFunc(func), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  ArGlobalRetFunctor1(Ret (*func)(P1), P1 p1) :
    myFunc(func), myP1(p1) {}

  virtual ~ArGlobalRetFunctor1()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (*myFunc)(myP1);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1) override {return (*myFunc)(p1);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

protected:

  Ret (*myFunc)(P1);
  P1 myP1;
};

/// ArFunctor for a global function with 2 parameters and return value
/**
   This is a class for global functions which take 2 parameters and return
   a value. This ties a C style function pointer into the ArFunctor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an ArFunctor can use it like any other ArFunctor.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class P1, class P2>
class ArGlobalRetFunctor2 : public ArRetFunctor2<Ret, P1, P2>
{
public:

  /// Constructor
  ArGlobalRetFunctor2() = default;

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  ArGlobalRetFunctor2(Ret (*func)(P1, P2)) :
    myFunc(func), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  ArGlobalRetFunctor2(Ret (*func)(P1, P2), P1 p1) :
    myFunc(func), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArGlobalRetFunctor2(Ret (*func)(P1, P2), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~ArGlobalRetFunctor2()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (*myFunc)(myP1, myP2);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (*myFunc)(p1, myP2);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (*myFunc)(p1, p2);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

protected:

  Ret (*myFunc)(P1, P2);
  P1 myP1;
  P2 myP2;
};

/// ArFunctor for a global function with 2 parameters and return value
/**
   This is a class for global functions which take 2 parameters and return
   a value. This ties a C style function pointer into the ArFunctor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an ArFunctor can use it like any other ArFunctor.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class P1, class P2, class P3>
class ArGlobalRetFunctor3 : public ArRetFunctor3<Ret, P1, P2, P3>
{
public:

  /// Constructor
  ArGlobalRetFunctor3() = default;

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  ArGlobalRetFunctor3(Ret (*func)(P1, P2, P3)) :
    myFunc(func), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  ArGlobalRetFunctor3(Ret (*func)(P1, P2, P3), P1 p1) :
    myFunc(func), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArGlobalRetFunctor3(Ret (*func)(P1, P2, P3), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArGlobalRetFunctor3(Ret (*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~ArGlobalRetFunctor3()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (*myFunc)(myP1, myP2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (*myFunc)(p1, myP2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (*myFunc)(p1, p2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override {return (*myFunc)(p1, p2, p3);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}
  
  

protected:

  Ret (*myFunc)(P1, P2, P3);
  P1 myP1;
  P2 myP2;
  P3 myP3;
};



/// ArFunctor for a global function with 4 parameters and return value
/**
   This is a class for global functions which take 4 parameters and return
   a value. This ties a C style function pointer into the ArFunctor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an ArFunctor can use it like any other ArFunctor.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class P1, class P2, class P3, class P4>
class ArGlobalRetFunctor4 : public ArRetFunctor4<Ret, P1, P2, P3, P4>
{
public:

  /// Constructor
  ArGlobalRetFunctor4() = default;

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  ArGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4)) :
    myFunc(func), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  ArGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1) :
    myFunc(func), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
 */
  ArGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4() {}

	/// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  ArGlobalRetFunctor4(Ret (*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~ArGlobalRetFunctor4()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override {return (*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) override {return (*myFunc)(p1, p2, p3, p4);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}
  
  
  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

protected:

  Ret (*myFunc)(P1, P2, P3, P4);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};


/// ArFunctor for a global function with 4 parameters and return value
/**
   This is a class for global functions which take 4 parameters and return
   a value. This ties a C style function pointer into the ArFunctor class
   hierarchy as a convience. Code that has a reference to this class
   and treat it as an ArFunctor can use it like any other ArFunctor.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class P1, class P2, class P3, class P4, class P5>
class ArGlobalRetFunctor5 : public ArRetFunctor5<Ret, P1, P2, P3, P4, P5>
{
public:

  /// Constructor
  ArGlobalRetFunctor5() = default;

  /// Constructor - supply function pointer
  /**
     @param func global function pointer
  */
  ArGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
  */
  ArGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
 */
  ArGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

	/// Constructor - supply function pointer, default parameters
  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  ArGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}

  /**
     @param func global function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  ArGlobalRetFunctor5(Ret (*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Destructor
  virtual ~ArGlobalRetFunctor5()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override {return (*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) override {return (*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) override {return (*myFunc)(p1, p2, p3, p4, p5);}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}
  
  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

  /// Set the default fifth parameter
  /**
     @param p5 default fifth parameter
  */
  [[deprecated]] virtual void setP5(P5 p5) {myP5=p5;}

protected:

  Ret (*myFunc)(P1, P2, P3, P4, P5);
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};

#endif // omitting ArGlobalRetFunctor from SWIG




//////
//////
//////
//////
//////
//////
////// ArFunctors for member functions
//////
//////
//////
//////
//////
//////


/// ArFunctor for a member function
/**
   This is a class for member functions. This class contains the knowledge
   on how to call a member function on a particular instance of a class.
   This class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class T>
class ArFunctorC : public ArFunctor
{
public:

  /// Constructor
  ArFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctorC(T &obj, void (T::*func)()) : myFunc(func), myObj(&obj) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctorC(T *obj, void (T::*func)()) : myFunc(func), myObj(obj) {}

  /// Destructor
  virtual ~ArFunctorC()  = default;

  /// Copy
  ArFunctorC(const ArFunctorC<T> &rhs) = default;

  /// Assignment
  ArFunctorC& operator=(const ArFunctorC<T> &rhs) = default;
  ArFunctorC &operator=(ArFunctorC<T> &&other) = default;

  /// Move
  ArFunctorC(ArFunctorC<T> &&other) = default;


  /// Invokes the ArFunctor
  virtual void invoke() override {(myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:

  void (T::*myFunc)();
  T* myObj;
};


/// ArFunctor for a member function with 1 parameter
/**
   This is a class for member functions which take 1 parameter. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class T, class P1>
class ArFunctor1C : public ArFunctor1<P1>
{
public:

  /// Constructor
  ArFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor1C(T &obj, void (T::*func)(P1)) :
    myFunc(func), myObj(&obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor1C(T &obj, void (T::*func)(P1), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor1C(T *obj, void (T::*func)(P1)) :
    myFunc(func), myObj(obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor1C(T *obj, void (T::*func)(P1), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1) {}

  /// Destructor
  virtual ~ArFunctor1C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

protected:

  void (T::*myFunc)(P1);
  T* myObj;
  P1 myP1;
};


/// ArFunctor for a member function with 1 parameter, which may be const
/**
   This is a class for member functions which take 1 parameter, which may be a
   const type. This class is like ArFunctor1C, except a default value for P1 
   can't be set. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class T, class P1>
class ArFunctor1CConst : public ArFunctor1<P1>
{
public:
  ArFunctor1CConst() {}
  ArFunctor1CConst(T &obj, void (T::*func)(P1)) :
    myFunc(func), myObj(&obj) {}
  ArFunctor1CConst(T *obj, void (T::*func)(P1)) :
    myFunc(func), myObj(obj) {}
  virtual ~ArFunctor1CConst()  = default;
  virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1);}
  virtual void setThis(T *obj) {myObj=obj;}
  virtual void setThis(T &obj) {myObj=&obj;}
private:
  virtual void invoke() override {} // can't do anything without an argument
protected:
  void (T::* myFunc)(P1);
  T *myObj;
};


/// ArFunctor for a member function with 2 parameters
/**
   This is a class for member functions which take 2 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class T, class P1, class P2>
class ArFunctor2C : public ArFunctor2<P1, P2>
{
public:

  /// Constructor
  ArFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor2C(T &obj, void (T::*func)(P1, P2)) :
    myFunc(func), myObj(&obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor2C(T &obj, void (T::*func)(P1, P2), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor2C(T &obj, void (T::*func)(P1, P2), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor2C(T *obj, void (T::*func)(P1, P2)) :
    myFunc(func), myObj(obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor2C(T *obj, void (T::*func)(P1, P2), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor2C(T *obj, void (T::*func)(P1, P2), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~ArFunctor2C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

protected:
  void (T::* myFunc)(P1, P2);
  T *myObj;
  P1 myP1;
  P2 myP2;
};

/// ArFunctor for a member function with 3 parameters
/**
   This is a class for member functions which take 3 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class T, class P1, class P2, class P3>
class ArFunctor3C : public ArFunctor3<P1, P2, P3>
{
public:

  /// Constructor
  ArFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor3C(T &obj, void (T::*func)(P1, P2, P3)) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor3C(T &obj, void (T::*func)(P1, P2, P3), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor3C(T &obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArFunctor3C(T &obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor3C(T *obj, void (T::*func)(P1, P2, P3)) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor3C(T *obj, void (T::*func)(P1, P2, P3), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor3C(T *obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3() {} 

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArFunctor3C(T *obj, void (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~ArFunctor3C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) override {(myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

protected:

  void (T::* myFunc)(P1, P2, P3);
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
};



/// ArFunctor for a member function with 4 parameters
/**
   This is a class for member functions which take 4 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class T, class P1, class P2, class P3, class P4>
class ArFunctor4C : public ArFunctor4<P1, P2, P3, P4>
{
public:

  /// Constructor
  ArFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4)) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
      @param p4 default fourth parameter
 */
  ArFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4)) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

	
  /// Destructor
  virtual ~ArFunctor4C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3) override {(myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) override {(myObj->*myFunc)(p1, p2, p3, p4);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}


protected:

  void (T::* myFunc)(P1, P2, P3, P4);
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};


/// ArFunctor for a member function with 5 parameters
/**
   This is a class for member functions which take 5 parameters. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class T, class P1, class P2, class P3, class P4, class P5>
class ArFunctor5C : public ArFunctor5<P1, P2, P3, P4, P5>
{
public:

  /// Constructor
  ArFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  ArFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}
  
  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
ArFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
  myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  ArFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

	
  /// Destructor
  virtual ~ArFunctor5C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3) override {(myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) override {(myObj->*myFunc)(p1, p2, p3, p4, myP5);}


  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) override {(myObj->*myFunc)(p1, p2, p3, p4, p5);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

  /// Set the default fifth parameter
  /**
     @param p5 default fifth parameter
  */
  [[deprecated]] virtual void setP5(P5 p5) {myP5=p5;}


protected:

  void (T::* myFunc)(P1, P2, P3, P4, P5);
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};






//////
//////
//////
//////
//////
//////
////// ArFunctors for member functions with return values
//////
//////
//////
//////
//////
//////


/// ArFunctor for a member function with return value
/**
   This is a class for member functions which return a value. This class
   contains the knowledge on how to call a member function on a particular
   instance of a class. This class should be instantiated by code that
   wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class T>
class ArRetFunctorC : public ArRetFunctor<Ret>
{
public:

  /// Constructor
  ArRetFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctorC(T &obj, Ret (T::*func)()) : myFunc(func), myObj(&obj) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctorC(T *obj, Ret (T::*func)()) : myFunc(func), myObj(obj) {}

  /// Destructor - supply function pointer
  virtual ~ArRetFunctorC()  = default;

  /// Invokes the ArFunctor with return value
  NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:

  Ret (T::*myFunc)();
  T* myObj;
};

/// ArFunctor for a member function with return value and 1 parameter
/**
   This is a class for member functions which take 1 parameter and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a ArFunctor to another
   piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class T, class P1>
class ArRetFunctor1C : public ArRetFunctor1<Ret, P1>
{
public:

  /// Constructor
  ArRetFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor1C(T &obj, Ret (T::*func)(P1)) :
    myFunc(func), myObj(&obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor1C(T &obj, Ret (T::*func)(P1), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor1C(T *obj, Ret (T::*func)(P1)) :
    myFunc(func), myObj(obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor1C(T *obj, Ret (T::*func)(P1), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1) {}

  /// Destructor
  virtual ~ArRetFunctor1C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

protected:
  Ret(T::* myFunc)(P1);
  T *myObj;
  P1 myP1;
};

/// ArFunctor for a member function with return value and 2 parameters
/**
   This is a class for member functions which take 2 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a ArFunctor to another
   piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class T, class P1, class P2>
class ArRetFunctor2C : public ArRetFunctor2<Ret, P1, P2>
{
public:

  /// Constructor
  ArRetFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor2C(T &obj, Ret (T::*func)(P1, P2)) :
    myFunc(func), myObj(&obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor2C(T &obj, Ret (T::*func)(P1, P2), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor2C(T &obj, Ret (T::*func)(P1, P2), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor2C(T *obj, Ret (T::*func)(P1, P2)) :
    myFunc(func), myObj(obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor2C(T *obj, Ret (T::*func)(P1, P2), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor2C(T *obj, Ret (T::*func)(P1, P2), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~ArRetFunctor2C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

protected:

  Ret(T::* myFunc)(P1, P2);
  T *myObj;
  P1 myP1;
  P2 myP2;
};

/// ArFunctor for a member function with return value and 3 parameters
/**
   This is a class for member functions which take 3 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a ArFunctor to another
   piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class T, class P1, class P2, class P3>
class ArRetFunctor3C : public ArRetFunctor3<Ret, P1, P2, P3>
{
public:

  /// Constructor
  ArRetFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3)) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3)) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~ArRetFunctor3C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override
    {return (myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj object to call function on
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj object to call function on
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

protected:

  Ret(T::* myFunc)(P1, P2, P3);
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
};




// Start 4


/// ArFunctor for a member function with return value and 4 parameters
/**
   This is a class for member functions which take 4 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a ArFunctor to another
   piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class T, class P1, class P2, class P3, class P4>
class ArRetFunctor4C : public ArRetFunctor4<Ret, P1, P2, P3, P4>
{
public:

  /// Constructor
  ArRetFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4)) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4)) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  ArRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~ArRetFunctor4C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override
    {return (myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) override
    {return (myObj->*myFunc)(p1, p2, p3, p4);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

protected:
  Ret(T::* myFunc)(P1, P2, P3, P4);
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};



/// ArFunctor for a member function with return value and 4 parameters
/**
   This is a class for member functions which take 4 parameters and return
   a value. This class contains the knowledge on how to call a member
   function on a particular instance of a class. This class should be
   instantiated by code that wishes to pass off a ArFunctor to another
   piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.
*/
template<class Ret, class T, class P1, class P2, class P3, class P4, class P5>
class ArRetFunctor5C : public ArRetFunctor5<Ret, P1, P2, P3, P4, P5>
{
public:

  /// Constructor
  ArRetFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  ArRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func member function pointer
  */
  ArRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5)) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
  */
  ArRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  ArRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  ArRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Destructor
  virtual ~ArRetFunctor5C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override
    {return (myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) override
    {return (myObj->*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) override
    {return (myObj->*myFunc)(p1, p2, p3, p4, p5);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

  /// Set the default fifth parameter
  /**
     @param p5 default fifth parameter
  */
  [[deprecated]] virtual void setP5(P5 p5) {myP5=p5;}

protected:

  Ret(T::* myFunc)(P1, P2, P3, P4, P5);
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P4 myP5;
};


/// Swig doesn't like the const ArFunctors
#ifndef SWIG

//////
//////
//////
//////
//////
//////
////// ArFunctors for const member functions
//////
//////
//////
//////
//////
//////


/// ArFunctor for a const member function
/**
   This is a class for const member functions. This class contains the
   knowledge on how to call a const member function on a particular
   instance of a class.  This class should be instantiated by code
   that wishes to pass off a ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  

   @swigomit
*/
template<class T>
class ArConstFunctorC : public ArFunctor
{
public:

  /// Constructor
  ArConstFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctorC(T &obj, void (T::*func)() const) : myFunc(func), myObj(&obj) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctorC(T *obj, void (T::*func)() const) : myFunc(func), myObj(obj) {}

  /// Destructor
  virtual ~ArConstFunctorC()  = default;

  /// Invokes the ArFunctor
  virtual void invoke() override {(myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:

  void (T::*myFunc)() const;
  T* myObj;

};


/// ArFunctor for a const member function with 1 parameter
/**
   This is a class for const member functions which take 1
   parameter. This class contains the knowledge on how to call a const
   member function on a particular instance of a class. This class
   should be instantiated by code that wishes to pass off a ArFunctor to
   another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class T, class P1>
class ArConstFunctor1C : public ArFunctor1<P1>
{
public:

  /// Constructor
  ArConstFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor1C(T &obj, void (T::*func)(P1) const) :
    myFunc(func), myObj(&obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor1C(T &obj, void (T::*func)(P1) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor1C(T *obj, void (T::*func)(P1) const) :
    myFunc(func), myObj(obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor1C(T *obj, void (T::*func)(P1) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1) {}

  /// Destructor
  virtual ~ArConstFunctor1C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

protected:
  void (T::* myFunc)(P1) const;
  T *myObj;
  P1 myP1;
};


/// ArFunctor for a const member function with 2 parameters
/**
   This is a class for const member functions which take 2
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class T, class P1, class P2>
class ArConstFunctor2C : public ArFunctor2<P1, P2>
{
public:

  /// Constructor
  ArConstFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor2C(T &obj, void (T::*func)(P1, P2) const) :
    myFunc(func), myObj(&obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor2C(T &obj, void (T::*func)(P1, P2) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor2C(T &obj, void (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor2C(T *obj, void (T::*func)(P1, P2) const) :
    myFunc(func), myObj(obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor2C(T *obj, void (T::*func)(P1, P2) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor2C(T *obj, void (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~ArConstFunctor2C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

protected:
  void (T::* myFunc)(P1, P2) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
};

/// ArFunctor for a const member function with 3 parameters
/**
   This is a class for const member functions which take 3
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class T, class P1, class P2, class P3>
class ArConstFunctor3C : public ArFunctor3<P1, P2, P3>
{
public:

  /// Constructor
  ArConstFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstFunctor3C(T &obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3() {} 

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstFunctor3C(T *obj, void (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~ArConstFunctor3C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  virtual void invoke(P1 p1, P2 p2, P3 p3) override {(myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

protected:

  void (T::* myFunc)(P1, P2, P3) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
};

/// ArFunctor for a const member function with 4 parameters
/**
   This is a class for const member functions which take 4
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class T, class P1, class P2, class P3, class P4>
class ArConstFunctor4C : public ArFunctor4<P1, P2, P3, P4>
{
public:

  /// Constructor
  ArConstFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4) const) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
      @param p4 default fourth parameter
 */
  ArConstFunctor4C(T &obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4) const) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArConstFunctor4C(T *obj, void (T::*func)(P1, P2, P3, P4), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

	
  /// Destructor
  virtual ~ArConstFunctor4C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3) override {(myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) override {(myObj->*myFunc)(p1, p2, p3, p4);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}


protected:

  void (T::* myFunc)(P1, P2, P3, P4) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};

/// ArFunctor for a const member function with 4 parameters
/**
   This is a class for const member functions which take 4
   parameters. This class contains the knowledge on how to call a
   const member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class T, class P1, class P2, class P3, class P4, class P5>
class ArConstFunctor5C : public ArFunctor5<P1, P2, P3, P4, P5>
{
public:

  /// Constructor
  ArConstFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5) const) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
      @param p4 default fourth parameter
 */
  ArConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  ArConstFunctor5C(T &obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5) const) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  ArConstFunctor5C(T *obj, void (T::*func)(P1, P2, P3, P4, P5), P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

	
  /// Destructor
  virtual ~ArConstFunctor5C()  = default;

  /// Invokes the ArFunctor
  [[deprecated]] virtual void invoke() override {(myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
  */
  [[deprecated]] virtual void invoke(P1 p1) override {(myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2) override {(myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3) override {(myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  [[deprecated]] virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4) override {(myObj->*myFunc)(p1, p2, p3, p4, myP5);}

  /// Invokes the ArFunctor
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  virtual void invoke(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) override {(myObj->*myFunc)(p1, p2, p3, p4, p5);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

  /// Set the default fifth parameter
  /**
     @param p5 default fifth parameter
  */
  [[deprecated]] virtual void setP5(P5 p5) {myP5=p5;}


protected:

  void (T::* myFunc)(P1, P2, P3, P4, P5) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P4 myP5;
};






//////
//////
//////
//////
//////
//////
////// ArFunctors for const member functions with return values
//////
//////
//////
//////
//////
//////


/// ArFunctor for a const member function with return value
/**
   Use this when the function you are targetting is a const method.
   For example, use <tt>ArConstRetFunctorC<double, ArRobot>(&robot,
ArRobot::getX)</tt> to create a ArFunctor targetting the ArRobot method
   declared with: <tt>double getX() const;</tt>.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class Ret, class T>
class ArConstRetFunctorC : public ArRetFunctor<Ret>
{
public:

  // Constructor
  //ArConstRetFunctorC() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctorC(T &obj, Ret (T::*func)() const) : myFunc(func), myObj(&obj) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctorC(T *obj, Ret (T::*func)() const) : myFunc(func), myObj(obj) {}

  /// Destructor - supply function pointer
  virtual ~ArConstRetFunctorC()  = default;

  /// Invokes the ArFunctor with return value
  NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)();}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

protected:

  Ret(T::* myFunc)() const;
  T *myObj;
};

/// ArFunctor for a const member function with return value and 1 parameter
/**
   This is a class for const member functions which take 1 parameter
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class Ret, class T, class P1>
class ArConstRetFunctor1C : public ArRetFunctor1<Ret, P1>
{
public:

  /// Constructor
  ArConstRetFunctor1C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor1C(T &obj, Ret (T::*func)(P1) const) :
    myFunc(func), myObj(&obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor1C(T &obj, Ret (T::*func)(P1) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor1C(T *obj, Ret (T::*func)(P1) const) :
    myFunc(func), myObj(obj), myP1() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor1C(T *obj, Ret (T::*func)(P1) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1) {}

  /// Destructor
  virtual ~ArConstRetFunctor1C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

protected:

  Ret(T::* myFunc)(P1) const;
  T *myObj;
  P1 myP1;
};

/// ArFunctor for a const member function with return value and 2 parameters
/**
   This is a class for const member functions which take 2 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class Ret, class T, class P1, class P2>
class ArConstRetFunctor2C : public ArRetFunctor2<Ret, P1, P2>
{
public:

  /// Constructor
  ArConstRetFunctor2C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor2C(T &obj, Ret (T::*func)(P1, P2) const) :
    myFunc(func), myObj(&obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor2C(T &obj, Ret (T::*func)(P1, P2) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor2C(T &obj, Ret (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor2C(T *obj, Ret (T::*func)(P1, P2) const) :
    myFunc(func), myObj(obj), myP1(), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor2C(T *obj, Ret (T::*func)(P1, P2) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor2C(T *obj, Ret (T::*func)(P1, P2) const, P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2) {}

  /// Destructor
  virtual ~ArConstRetFunctor2C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

protected:

  Ret(T::* myFunc)(P1, P2) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
};

/// ArFunctor for a const member function with return value and 3 parameters
/**
   This is a class for const member functions which take 3 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class Ret, class T, class P1, class P2, class P3>
class ArConstRetFunctor3C : public ArRetFunctor3<Ret, P1, P2, P3>
{
public:

  /// Constructor
  ArConstRetFunctor3C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstRetFunctor3C(T &obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstRetFunctor3C(T *obj, Ret (T::*func)(P1, P2, P3) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3) {}

  /// Destructor
  virtual ~ArConstRetFunctor3C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2, myP3);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override
    {return (myObj->*myFunc)(p1, p2, p3);}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

protected:

  Ret(T::* myFunc)(P1, P2, P3) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
};




// Start 4


/// ArFunctor for a const member function with return value and 4 parameters
/**
   This is a class for const member functions which take 4 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class Ret, class T, class P1, class P2, class P3, class P4>
class ArConstRetFunctor4C : public ArRetFunctor4<Ret, P1, P2, P3, P4>
{
public:

  /// Constructor
  ArConstRetFunctor4C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArConstRetFunctor4C(T &obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  ArConstRetFunctor4C(T *obj, Ret (T::*func)(P1, P2, P3, P4) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4) {}

  /// Destructor
  virtual ~ArConstRetFunctor4C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2, myP3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 second parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override
    {return (myObj->*myFunc)(p1, p2, p3, myP4);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) override
    {return (myObj->*myFunc)(p1, p2, p3, p4);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

protected:

  Ret(T::* myFunc)(P1, P2, P3, P4) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
};




/// ArFunctor for a const member function with return value and 5 parameters
/**
   This is a class for const member functions which take 5 parameters
   and return a value. This class contains the knowledge on how to
   call a member function on a particular instance of a class. This
   class should be instantiated by code that wishes to pass off a
   ArFunctor to another piece of code.
   
   For an overall description of ArFunctors, see ArFunctor.  */
template<class Ret, class T, class P1, class P2, class P3, class P4, class P5>
class ArConstRetFunctor5C : public ArRetFunctor5<Ret, P1, P2, P3, P4, P5>
{
public:

  /// Constructor
  ArConstRetFunctor5C() {}

  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const) :
    myFunc(func), myObj(&obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
  */
  ArConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
  */
  ArConstRetFunctor5C(T &obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myObj(&obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}



  /// Constructor - supply function pointer
  /**
     @param obj object to call function on
     @param func const member function pointer
  */
  ArConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const) :
    myFunc(func), myObj(obj), myP1(), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
  */
  ArConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1) :
    myFunc(func), myObj(obj), myP1(p1), myP2(), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
  */
  ArConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
  */
  ArConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(), myP5() {}

  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
 */
  ArConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5() {}


  /// Constructor - supply function pointer, default parameters
  /**
     @param obj object to call function on
     @param func const member function pointer
     @param p1 default first parameter
     @param p2 default second parameter
     @param p3 default third parameter
     @param p4 default fourth parameter
     @param p5 default fifth parameter
 */
  ArConstRetFunctor5C(T *obj, Ret (T::*func)(P1, P2, P3, P4, P5) const, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) :
    myFunc(func), myObj(obj), myP1(p1), myP2(p2), myP3(p3), myP4(p4), myP5(p5) {}

  /// Destructor
  virtual ~ArConstRetFunctor5C()  = default;

  /// Invokes the ArFunctor with return value
  [[deprecated]] NODISCARD virtual Ret invokeR() override {return (myObj->*myFunc)(myP1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1) override {return (myObj->*myFunc)(p1, myP2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
  */
  [[deprecated]] NODISCARD virtual Ret invokeR(P1 p1, P2 p2) override {return (myObj->*myFunc)(p1, p2, myP3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 second parameter
  */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3) override
    {return (myObj->*myFunc)(p1, p2, p3, myP4, myP5);}

  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4) override
    {return (myObj->*myFunc)(p1, p2, p3, p4, myP5);}


  /// Invokes the ArFunctor with return value
  /**
     @param p1 first parameter
     @param p2 second parameter
     @param p3 third parameter
     @param p4 fourth parameter
     @param p5 fifth parameter
 */
  NODISCARD virtual Ret invokeR(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) override
    {return (myObj->*myFunc)(p1, p2, p3, p4, p5);}


  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T *obj) {myObj=obj;}

  /// Set the 'this' pointer
  /**
     @param obj the 'this' pointer
  */
  virtual void setThis(T &obj) {myObj=&obj;}

  /// Set the default parameter
  /**
     @param p1 default first parameter
  */
  [[deprecated]] virtual void setP1(P1 p1) {myP1=p1;}

  /// Set the default 2nd parameter
  /**
     @param p2 default second parameter
  */
  [[deprecated]] virtual void setP2(P2 p2) {myP2=p2;}

  /// Set the default third parameter
  /**
     @param p3 default third parameter
  */
  [[deprecated]] virtual void setP3(P3 p3) {myP3=p3;}

  /// Set the default fourth parameter
  /**
     @param p4 default fourth parameter
  */
  [[deprecated]] virtual void setP4(P4 p4) {myP4=p4;}

  /// Set the default fifth parameter
  /**
     @param p5 default fifth parameter
  */
  [[deprecated]] virtual void setP5(P5 p5) {myP5=p5;}

protected:

  Ret(T::* myFunc)(P1, P2, P3, P4, P5) const;
  T *myObj;
  P1 myP1;
  P2 myP2;
  P3 myP3;
  P4 myP4;
  P5 myP5;
};


#endif // omitting Const ArFunctors from SWIG



#endif // ARFUNCTOR_H


