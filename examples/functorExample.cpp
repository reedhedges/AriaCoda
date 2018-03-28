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

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/


#include <string>
#include "Aria.h"


/** @example functorExample.cpp  Illustrates the use of functors
 *  
 *  @sa @ref functors in the ARIA overview
 */

/*
  This is a class that has some callback methods. Functors which refer to these
  callbacks will be passed to the DriverClass.  
*/
class CallbackContainer
{
public:

  void callback1();
  void callback2(int i);
  bool callback3(const char *str);
  int callback4() const;
};

void CallbackContainer::callback1()
{
  printf("CallbackContainer::callback1 called.\n");
}

void CallbackContainer::callback2(int i)
{
  printf("CallbackContainer::callback2 called with argument of '%d'\n", i);
}

bool CallbackContainer::callback3(const char *str)
{
  printf("CallbackContainer::callback3 called with argument of '%s'.\n", str);
  return(true);
}

int CallbackContainer::callback4() const
{
  printf("CallbackContainer::callback4 called.\n");
  return 23;
}

/* 
 * Functors can also invoke global functions.
 */

void globalCallback()
{
  printf("globalCallback() called.\n");
}


/*
  This is a "driver" class. It takes three functors of different types and
  will invoke the three functors. This is a typical use of
  functors: to pass information or event notifications between loosely
  coupled objects.
*/
class DriverClass
{
public:

  void invokeFunctors();

  void setCallback1(ArFunctor *func) {myFunc1=func;}
  void setCallback2(ArFunctor1<int> *func) {myFunc2=func;}
  void setCallback3(ArRetFunctor1<bool, const char *> *func) {myFunc3=func;}
  void setCallback4(ArRetFunctor<int> *func) { myFunc4 = func; }


protected:

  ArFunctor *myFunc1;
  ArFunctor1<int> *myFunc2;
  ArRetFunctor1<bool, const char *> *myFunc3;
  ArRetFunctor<int> *myFunc4;
};

void DriverClass::invokeFunctors()
{
  bool ret;

  printf("Invoking functor1... ");
  myFunc1->invoke();

  printf("Invoking functor2... ");
  myFunc2->invoke(23);
	 

  /*
     For functors with return values, use invorkeR() instead of invoke()
     to get the return value.  The invoke() function can also be used to invoke
     the functor, but the return value is lost.  (And is a possible source
     of memory leaks if you were supposed to free a pointer returned.)
  */
  printf("Invoking functor3... ");
  ret=myFunc3->invokeR("This is a string argument");
  if (ret)
    printf("\t-> functor3 returned 'true'\n");
  else
    printf("\t-> functor3 returned 'false'\n");

  printf("Invoking functor4... ");
  int i = myFunc4->invokeR();
  printf("\t-> functor4 returned %d\n", i);
}

int main()
{
  CallbackContainer cb;
  DriverClass driver;

  ArFunctorC<CallbackContainer> functor1(cb, &CallbackContainer::callback1);
  ArFunctor1C<CallbackContainer, int> functor2(cb, &CallbackContainer::callback2);
  ArRetFunctor1C<bool, CallbackContainer, const char *>
    functor3(cb, &CallbackContainer::callback3);
  ArConstRetFunctorC<int, CallbackContainer> functor4(cb, &CallbackContainer::callback4);

  driver.setCallback1(&functor1);
  driver.setCallback2(&functor2);
  driver.setCallback3(&functor3);
  driver.setCallback4(&functor4);

  driver.invokeFunctors();

  /* You can make functors that target global functions too. */
  ArGlobalFunctor globalFunctor(&globalCallback);
  printf("Invoking globalFunctor... ");
  globalFunctor.invoke();

  /* You can also include the values of arguments in an ArFunctor object, if you
   * want to use the same value in every invocation of the functor.
   */
  ArFunctor1C<CallbackContainer, int> functor5(cb, &CallbackContainer::callback2, 42);
  printf("Invoking functor with constant argument... ");
  functor5.invoke();

  /* Functors can be downcast to parent interface classes, as long as their invocation
   * does not require arguments.
   */
  ArFunctor* baseFunctor = &functor4;
  printf("Invoking downcast functor... ");
  baseFunctor->invoke();


  return(0);
}
