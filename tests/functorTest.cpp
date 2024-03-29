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
#include "Aria/Aria.h"


/*
  TestClass - This class has 6 functions which are used to test out the
  functors.
*/

class TestClass
{
public:

  void function();
  bool function_invoked = false;

  void function(int arg1);
  bool function_int_invoked = false;

  void function(bool arg1, std::string arg2);
  bool function_bool_string_invoked = false;

  bool retFunction();
  bool retFunction_bool_invoked = false;

  char * retFunction(int arg1);
  bool retFunction_charp_int_invoked = false;

  double retFunction(bool arg1, std::string arg2);
  bool retFunction_double_bool_string_invoked = false;
};


void TestClass::function()
{
  printf("TestClass::function()\n");
  function_invoked = true;
}

void TestClass::function(int arg1)
{
  printf("TestClass::function(int arg1=%d)\n", arg1);
  function_int_invoked = true;
}

void TestClass::function(bool arg1, std::string arg2)
{
  printf("TestClass::function(bool arg1=%d, std::string arg2='%s')\n", arg1, arg2.c_str());
  function_bool_string_invoked = true;
}

bool TestClass::retFunction()
{
  printf("bool TestClass::retFunction() (returning true)\n");
  retFunction_bool_invoked = true;
  return(true);
}

char * TestClass::retFunction(int arg1)
{
  printf("char * TestClass::retFunction(int arg1=%d)\n", arg1);
  retFunction_charp_int_invoked = true;
  return("Hello");
}

double TestClass::retFunction(bool arg1, std::string arg2)
{
  printf("double TestClass::retFunction(bool arg1=%d, std::string arg2='%s')\n",
	 arg1, arg2.c_str());
  retFunction_double_bool_string_invoked = true;
  return(4.62);
}


/*
  Here are 6 global functions to test out the functors for non-member funtions.
TODO save argument values and test later
*/
bool global_function_invoked = false;
void function()
{
  printf("global function()\n");
  global_function_invoked = true;
}

bool global_function_int_invoked = false;
void function(int arg1)
{
  printf("global function(int arg1=%d)\n", arg1);
  global_function_int_invoked = true;
}

bool global_function_bool_string_invoked = false;
void function(bool arg1, std::string arg2)
{
  printf("global function(bool arg1=%d, std::string arg2='%s')\n",
	 arg1, arg2.c_str());
  global_function_bool_string_invoked = true;
}

bool global_retFunction_bool_invoked = false;
bool retFunction()
{
  printf("global bool retFunction() (returning true)\n");
  global_retFunction_bool_invoked = true;
  return(true);
}

bool global_retFunction_charp_int_invoked = false;
char * retFunction(int arg1)
{
  printf("global char* retFunction(int arg1=%d) (returning \"Hello\")\n", arg1);
  global_retFunction_charp_int_invoked = true;
  return("Hello");
}

bool global_retFunction_double_bool_string_invoked = false;
double retFunction(bool arg1, std::string arg2)
{
  printf("global double retFunction(bool arg1=%d, std::string arg2='%s') (returnig 4.62)\n",
	 arg1, arg2.c_str());
  global_retFunction_double_bool_string_invoked = true;
  return(4.62);
}


/*
  Test functors with class member funtions
*/

// Direct invocation of the functors with supplying parameters.
void testDirect()
{
  TestClass test;
  ArFunctorC<TestClass> functor(test, &TestClass::function);
  ArFunctor1C<TestClass, int> functor1(test, &TestClass::function, 1);
  ArFunctor2C<TestClass, bool, std::string> functor2(test,
						     &TestClass::function,
						     false, "default arg");

  printf("\n****** Testing direct invocation using ArFunctor::invoke(...)\n");

  puts("> Should see TestClass::function()...");
  functor.invoke();
  assert(test.function_invoked);

  puts("> Should see TestClass::function(1)...");
  functor1.invoke();
  assert(test.function_int_invoked);

  test.function_int_invoked = false;
  puts("> Should see TestClass::function(5)...");
  functor1.invoke(5);
  assert(test.function_int_invoked);

  puts("> Should see TestClass::function(true, \"argument 1\")...");
  functor2.invoke(true, "argument 1");
  assert(test.function_bool_string_invoked);
  // todo test argument values too
}

// Invocation of a base ArFunctor pointer to a functor. Because the pointer
// is of type ArFunctor, the parameters can not be supplied. The default
// parameters, which are supplied when the functor is constructed, are used.
void testBase()
{
  TestClass test;
  ArFunctor *fptr;
  ArFunctorC<TestClass> functor(test, &TestClass::function);
  ArFunctor1C<TestClass, int> functor1(test, &TestClass::function, 1);
  ArFunctor2C<TestClass, bool, std::string> functor2(test,
						     &TestClass::function,
						     false, "default arg");

  printf("\n****** Testing base invocation\n");

  fptr=&functor;
  puts("> Should see TestClass::function()...");
  fptr->invoke();
  assert(test.function_invoked);

  fptr=&functor1;
  puts("> Should see TestClass::function(1)...");
  fptr->invoke();
  assert(test.function_int_invoked);

  fptr=&functor2;
  puts("> Should see TestClass::function(false, \"default arg\")...");
  fptr->invoke();
  assert(test.function_bool_string_invoked);
  // todo test argument values too
}

// Invocation of pointers which supply the parameter type. Full invocation
// with paramters is posesible in this fashion with out knowing the class
// that the functor refers to.
void testParams()
{
  TestClass test;
  ArFunctorC<TestClass> functor(test, &TestClass::function);
  ArFunctor1C<TestClass, int> functor1(test, &TestClass::function);
  ArFunctor2C<TestClass, bool, std::string> functor2(test,
						     &TestClass::function);
  ArFunctor *fptr;
  ArFunctor1<int> *fptr1;
  ArFunctor2<bool, std::string> *fptr2;

  printf("\n****** Testing pointer invocation\n");

  fptr=&functor;
  puts("> Should see TestClass::function()...");
  fptr->invoke();
  assert(test.function_invoked);

  fptr1=&functor1;
  puts("> Should see TestClass::function(2)...");
  fptr1->invoke(2);
  assert(test.function_int_invoked);

  fptr2=&functor2;
  puts("> Should see TestClass::function(true, \"argument 2\")...");
  fptr2->invoke(true, "argument 2");
  assert(test.function_bool_string_invoked);
  // todo test argument values too
}


void setFunctorPtr(ArFunctor *f)
{
  // example interface only , does nothing
}

void setIntFunctorPtr(ArFunctor1<int> *f)
{
  // example interface only , does nothing
}

// It is possible to supply a more specialized ArFunctor class to a function
// that takes a plant ArFunctor pointer, since it will be
// implicitly cast to that parent class
// examples here only, no tests:
void testDowncast()
{
  ArRetFunctor1C<char*, TestClass, int> f;
  ArFunctor* y = &f;
  setFunctorPtr(&f);
  setFunctorPtr(y);
}

/*
  Test functors with return values, ArRetFunctor
*/

// Direct invocation of the functors with return values and supplying
// parameters. It is not posesible to have the operator() for functors with
// return values. This is due to limitations of C++ and different C++
// compilers where you can not overload return values in all cases.
void testReturnDirect()
{
  TestClass test;
  ArRetFunctorC<bool, TestClass> functor(test, &TestClass::retFunction);
  ArRetFunctor1C<char*, TestClass, int>
    functor1(test, &TestClass::retFunction, 1);
  ArRetFunctor2C<double, TestClass, bool, std::string>
    functor2(test, &TestClass::retFunction, false, "default arg");
  bool bret;
  char *cret;
  double dret;

  //bret=test.retFunction();
  //cret=test.retFunction(4);
  //dret=test.retFunction(true, "foof");

  printf("\n****** Testing direct invocation with return\n");
  puts("> TestClass::retFunction() should return true...");
  bret=functor.invokeR();
  assert(test.retFunction_bool_invoked);
  assert(bret);

  printf("Returned: %d\n", bret);
  puts("> TestClass::retFunction(5) should return \"Hello\"...");
  cret=functor1.invokeR(5);
  assert(test.retFunction_charp_int_invoked);
  assert(strcmp(cret, "Hello") == 0);

  printf("Returned: %s\n", cret);
  puts("> TestClass::retFunction(true, \"argument 1\") should return 4.62...");
  dret=functor2.invokeR(true, "argument 1");
  printf("Returned: %e\n", dret);
  assert(test.retFunction_double_bool_string_invoked);
  assert(dret == 4.62);
  // todo test argument values too
}

void testReturnBase()
{
  TestClass test;
  ArRetFunctorC<bool, TestClass> functor(test, &TestClass::retFunction);
  ArRetFunctor1C<char*, TestClass, int>
    functor1(test, &TestClass::retFunction, 1);
  ArRetFunctor2C<double, TestClass, bool, std::string>
    functor2(test, &TestClass::retFunction, false, "default arg");
  ArRetFunctor<bool> *fBoolPtr;
  ArRetFunctor<char*> *fCharPtr;
  ArRetFunctor<double> *fDoublePtr;
  bool bret = false;
  char *cret;
  double dret;

  printf("\n****** Testing base invocation with return\n");
  fBoolPtr=&functor;
  puts("> TestClass::retFunction() should return true");
  bret=fBoolPtr->invokeR();
  assert(test.retFunction_bool_invoked);
  assert(bret);
  printf("Returned: %d\n", bret);
  fCharPtr=&functor1;
  puts("> TestClass::retFunction(1) should return \"Hello\"");
  cret=fCharPtr->invokeR();
  assert(test.retFunction_charp_int_invoked);
  assert(strcmp(cret, "Hello") == 0);
  printf("Returned: %s\n", cret);
  fDoublePtr=&functor2;
  puts("> TestClass::retFunction(false, \"default arg\" should return 4.62");
  dret=fDoublePtr->invokeR();
  printf("Returned: %e\n", dret);
  assert(test.retFunction_double_bool_string_invoked);
  assert(dret == 4.62);

  // todo test argument values too
}

void testReturnParams()
{
  TestClass test;
  ArRetFunctorC<bool, TestClass> functor(test, &TestClass::retFunction);
  ArRetFunctor1C<char*, TestClass, int>
    functor1(test, &TestClass::retFunction, 1);
  ArRetFunctor2C<double, TestClass, bool, std::string>
    functor2(test, &TestClass::retFunction, false, "default arg");
  ArRetFunctor<bool> *fBoolPtr;
  ArRetFunctor1<char*, int> *fCharPtr;
  ArRetFunctor2<double, bool, std::string> *fDoublePtr;
  bool bret;
  char *cret;
  double dret;

  printf("\n****** Testing pointer invocation with return\n");
  fBoolPtr=&functor;
  puts("> TestClass::retFunction() should return true");
  bret=fBoolPtr->invokeR();
  printf("Returned: %d\n", bret);
  fCharPtr=&functor1;
  puts("> TestClass::retFunction(7) should return \"Hello\"");
  cret=fCharPtr->invokeR(7);
  printf("Returned: %s\n", cret);
  fDoublePtr=&functor2;
  puts("> TestClass::retFunction(false, \"argument 3\") should return 4.62...");
  dret=fDoublePtr->invokeR(false, "argument 3");
  printf("Returned: %e\n", dret);
  // todo test argument values too
}


/*
  Test global functors, ArGlobalFunctor.
*/

// Direct invocation of the global functors with supplying parameters.
void testGlobalDirect()
{
  ArGlobalFunctor functor(&function);
  ArGlobalFunctor1<int> functor1(&function, 1);
  ArGlobalFunctor2<bool, std::string> functor2(&function,
					       false, "default arg");

  global_function_invoked = global_function_int_invoked = global_function_bool_string_invoked = false;

  printf("\n****** Testing global direct invocation\n");
  puts("> Should see function()...");
  functor.invoke();
  assert(global_function_invoked);
  puts("> Should see function(5)...");
  functor1.invoke(5);
  assert(global_function_int_invoked);
  puts("> Should see function(true, \"argument 1\")...");
  functor2.invoke(true, "argument 1");
  assert(global_function_bool_string_invoked);
}

// Invocation of a base ArFunctor pointer to a global functor. Because the
// pointer is of type ArFunctor, the parameters can not be supplied. The
// default parameters, which are supplied when the functor is constructed,
// are used.
void testGlobalBase()
{
  ArFunctor *fptr;
  ArGlobalFunctor functor(function);
  ArGlobalFunctor1<int> functor1(function, 1);
  ArGlobalFunctor2<bool, std::string> functor2(function, false,
						"default arg");

  global_function_invoked = global_function_int_invoked = global_function_bool_string_invoked = false;

  printf("\n****** Testing global base invocation\n");
  fptr=&functor;
  puts("> Should see function()...");
  fptr->invoke();
  assert(global_function_invoked);
  fptr=&functor1;
  puts("> Should see function(1)...");
  fptr->invoke();
  assert(global_function_int_invoked);
  fptr=&functor2;
  puts("> Should see function(false, \"default arg\")...");
  fptr->invoke();
  assert(global_function_bool_string_invoked);
}

// Invocation of pointers which supply the parameter type. Full invocation
// with paramters is posesible in this fashion with out knowing the class
// that the functor refers to.
void testGlobalParams()
{
  ArGlobalFunctor functor(function);
  ArGlobalFunctor1<int> functor1(function, 1);
  ArGlobalFunctor2<bool, std::string> functor2(function, false,
						"default arg");
  ArFunctor *fptr;
  ArFunctor1<int> *fptr1;
  ArFunctor2<bool, std::string> *fptr2;

  global_function_invoked = global_function_int_invoked = global_function_bool_string_invoked = false;

  printf("\n****** Testing global pointer invocation\n");
  fptr=&functor;
  puts("> Should see function()...");
  fptr->invoke();
  assert(global_function_invoked);

  fptr1=&functor1;
  puts("> Should see function(2)...");
  fptr1->invoke(2);
  assert(global_function_int_invoked);

  fptr2=&functor2;
  puts("> Should see function(true, \"argument 2\")...");
  fptr2->invoke(true, "argument 2");
  assert(global_function_bool_string_invoked);

  // todo check argument values received correctly too
}


/*
  Test global functors with return, ArGlobalRetFunctor.
*/

// Direct invocation of the global functors with supplying parameters.
void testGlobalReturnDirect()
{
  ArGlobalRetFunctor<bool> functor(&retFunction);
  ArGlobalRetFunctor1<char*, int> functor1(&retFunction, 1);
  ArGlobalRetFunctor2<double, bool, std::string>
    functor2(&retFunction, false, "default arg");
  bool bret;
  char *cret;
  double dret;

  global_retFunction_bool_invoked = global_retFunction_charp_int_invoked = global_retFunction_double_bool_string_invoked = false;

  printf("\n****** Testing global direct invocation with return\n");
  puts("> bool retFunction() should return true...");
  bret=functor.invokeR();
  printf("Returned: %d\n", bret);
  assert(global_retFunction_bool_invoked);

  puts("> char* retFunction(5) should return \"Hello\"...");
  cret=functor1.invokeR(5);
  printf("Returned: %s\n", cret);
  assert(global_retFunction_charp_int_invoked);

  puts("> double retFunction(true, \"argument 1\") should return 4.62...");
  dret=functor2.invokeR(true, "argument 1");
  printf("Returned: %e\n", dret);
  assert(global_retFunction_double_bool_string_invoked);

  // todo test return values
  // todo check argument values received correctly too
}

// Invocation of a base ArFunctor pointer to a global functor. Because the
// pointer is of type ArFunctor, the parameters can not be supplied. The
// default parameters, which are supplied when the functor is constructed,
// are used.
void testGlobalReturnBase()
{
  ArGlobalRetFunctor<bool> functor(retFunction);
  ArGlobalRetFunctor1<char*, int> functor1(retFunction, 1);
  ArGlobalRetFunctor2<double, bool, std::string>
    functor2(retFunction, false, "default arg");
  ArRetFunctor<bool> *fBoolPtr;
  ArRetFunctor<char*> *fCharPtr;
  ArRetFunctor<double> *fDoublePtr;
  bool bret;
  char *cret;
  double dret;
  global_retFunction_bool_invoked = global_retFunction_charp_int_invoked = global_retFunction_double_bool_string_invoked = false;

  printf("\n****** Testing global base invocation with return\n");
  fBoolPtr=&functor;
  puts("> bool retFunction() should return true...");
  bret=fBoolPtr->invokeR();
  printf("Returned: %d\n", bret);
  assert(global_retFunction_bool_invoked);

  fCharPtr=&functor1;
  puts("> char* retFunction(1) should return \"Hello\"...");
  cret=fCharPtr->invokeR();
  printf("Returned: %s\n", cret);
  assert(global_retFunction_charp_int_invoked);

  fDoublePtr=&functor2;
  puts("> double retFunction(false, \"default arg\") should return 4.62...");
  dret=fDoublePtr->invokeR();
  printf("Returned: %e\n", dret);
  assert(global_retFunction_double_bool_string_invoked);

  // todo test return values
  // todo check argument values received correctly too
}

// Invocation of pointers which supply the parameter type. Full invocation
// with paramters is posesible in this fashion with out knowing the class
// that the functor refers to.
void testGlobalReturnParams()
{
  ArGlobalRetFunctor<bool> functor(retFunction);
  ArGlobalRetFunctor1<char*, int> functor1(retFunction, 1);
  ArGlobalRetFunctor2<double, bool, std::string>
    functor2(retFunction, false, "default arg");
  ArRetFunctor<bool> *fBoolPtr;
  ArRetFunctor1<char*, int> *fCharPtr;
  ArRetFunctor2<double, bool, std::string> *fDoublePtr;
  bool bret;
  char *cret;
  double dret;
  global_retFunction_bool_invoked = global_retFunction_charp_int_invoked = global_retFunction_double_bool_string_invoked = false;

  printf("\n****** Testing global pointer invocation with return\n");
  fBoolPtr=&functor;
  puts("> bool retFunction() should return true...");
  bret=fBoolPtr->invokeR();
  printf("Returned: %d\n", bret);
  assert(global_retFunction_bool_invoked);

  fCharPtr=&functor1;
  puts("> char* retFunction(7) should return \"Hello\"...");
  cret=fCharPtr->invokeR(7);
  printf("Returned: %s\n", cret);
  assert(global_retFunction_charp_int_invoked);

  fDoublePtr=&functor2;
  puts("> double retFunction(false, \"argument 3\") should return 4.62...");
  dret=fDoublePtr->invokeR(false, "argument 3");
  printf("Returned: %e\n", dret);
  assert(global_retFunction_double_bool_string_invoked);
  // todo test argument values and return values
}


ArFunctor1C<TestClass, int> copy;
void copyFunc(const ArFunctor1C<TestClass, int>& ref)
{
   copy = ref;
   puts("Made new copy of functor passed to copyFunc() as reference. Invoking with no arg. Should get passed last set P1.");
   copy.invoke();
  // todo test
}

void testCopy()
{
  TestClass t;
  ArFunctor1C<TestClass, int> f1(&t, &TestClass::function);
  ArFunctor1<int>& ref = f1;
  ArFunctor1C<TestClass, int> copy = f1;
  puts("Invoking original functor with arg 1:");
  f1.invoke(1);
  assert(t.function_int_invoked);
  t.function_int_invoked = false;

  puts("Invoking reference with arg 1:");
  ref.invoke(1);
  assert(t.function_int_invoked);
  t.function_int_invoked = false;

  puts("Invoking copy with arg 1:");
  copy.invoke(1);
  assert(t.function_int_invoked);
  t.function_int_invoked = false;

  puts("Setting P1 in copy to 2 and invoking. Should get passed 2:");
  copy.setP1(2);
  copy.invoke();
  assert(t.function_int_invoked);
  t.function_int_invoked = false;

  puts("Invoking original functor with no arg, should get passed 0:");
  f1.invoke();
  assert(t.function_int_invoked);
  t.function_int_invoked = false;

  puts("Setting P1 in original to 3 and invoking with no arg, should get passed 3:");
  f1.setP1(3);
  f1.invoke();
  assert(t.function_int_invoked);
  t.function_int_invoked = false;

  copyFunc(f1);

  // todo test argument values passed
}


// main(). Drives this example by creating an instance of the TestClass and
// instances of functors. Then the functors are invoked.
int main()
{
  testDirect();
  testBase();
  testParams();
  testReturnDirect();
  testReturnBase();
  testReturnParams();
  testGlobalDirect();
  testGlobalBase();
  testGlobalParams();
  testGlobalReturnDirect();
  testGlobalReturnBase();
  testGlobalReturnParams();

  ArGlobalFunctor2<bool, std::string> f(&function);
  global_function_bool_string_invoked = false;
  f.setP2("hello");
  f.invoke(true);
  assert(global_function_bool_string_invoked);

  testCopy();

  return(0);
}
