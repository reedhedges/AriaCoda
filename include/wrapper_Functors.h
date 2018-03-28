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

#ifndef ARIA_wrapper_Functors_h
#define ARIA_wrapper_Functors_h

/* For Python, define ArFunctor subclasses to hold Python C library
 * callable function objects.  These are used internally by the 
 * wrapper library, and typemaps convert target-language function 
 * objects to these ArFunctor subclasses-- you only need to pass
 * the function to Aria when in the C++ API you would pass a Functor.
 *
 * For Java, define subclasses of ArFunctor for various argument types,
 * since you can't access template classes in Java.  Then you can 
 * further subclass within Java and pass that object to Aria.
 */

#include "ArFunctor.h"

/* Functors for Python: */

#ifdef SWIGPYTHON

class ArPyFunctor : public virtual ArFunctor 
{
protected:
  PyObject* pyFunction;
public:
  ArPyFunctor(PyObject* _m) : pyFunction(_m) {
    Py_INCREF(pyFunction);
  }

  virtual void invoke() { 
    PyObject* r = PyObject_CallObject(pyFunction, NULL);
    if(!r) {
      fputs("** ArPyFunctor: Error calling Python function: ", stderr);
      PyErr_Print();
    }
  }

  virtual ~ArPyFunctor() {
    Py_DECREF(pyFunction);
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};


/* Return type could be generalized if we find a way to convert any Python type
 * returned by the Python function to whatever the templatized return type is 
 * required by the C++ functor.  This _Bool version just checks boolean value of
 * return from python function.
 */
class ArPyRetFunctor_Bool : 
  public virtual ArRetFunctor<bool>,
  public virtual ArPyFunctor
{
public:
  ArPyRetFunctor_Bool(PyObject* _m) : ArRetFunctor<bool>(), ArPyFunctor(_m) {
  }

  virtual bool invokeR() {
    PyObject* r = PyObject_CallObject(pyFunction, NULL);  
    if(!r) {
      fputs("** ArPyRetFunctor_Bool: Error calling Python function: ", stderr);
      PyErr_Print();
    }
    return(r == Py_True);
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};



class ArPyFunctor1_String : 
  public virtual ArFunctor1<const char*>,
  public virtual ArPyFunctor
{
public:
  ArPyFunctor1_String(PyObject* _m) : ArFunctor1<const char*>(), ArPyFunctor(_m) {
  }

  virtual void invoke(const char* arg) {
    if(!arg) {
      Py_FatalError("ArPyFunctor1_String invoked with null argument!");
      // TODO invoke with "None" value
      return;
    }
    //printf("ArPyFunctor1_String invoked with \"%s\"\n", arg);
    PyObject *s = PyString_FromString(arg);
    if(!s) {
      PyErr_Print();
      Py_FatalError("ArPyFunctor1_String: Error converting argument to Python string value");
      return;
    }
    PyObject* r = PyObject_CallFunctionObjArgs(pyFunction, s, NULL);  
    if(!r) {
      fputs("** ArPyFunctor1_String: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
    Py_DECREF(s);
  }

  virtual void invoke() {
    fputs("** ArPyFunctor1_String: invoke: No argument supplied?", stderr);
    Py_FatalError("ArPyFunctor1_String invoked with no arguments!");
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};

#ifdef ARIA_WRAPPER
class ArPyPacketHandlerFunctor : 
  public virtual ArRetFunctor1<bool, ArRobotPacket*>,
  public virtual ArPyFunctor
{
public:
  ArPyPacketHandlerFunctor(PyObject* _m) : ArRetFunctor1<bool, ArRobotPacket*>(), ArPyFunctor(_m) {}
  virtual bool invokeR(ArRobotPacket* pkt) {
    if(!pkt) { 
      Py_FatalError("ArPyPacketHandlerFunctor invoked with null argument!");
      return false;
    }
    PyObject *po = SWIG_NewPointerObj((void*)pkt, SWIGTYPE_p_ArRobotPacket, 0); //PyObject_FromPointer(arg);
    PyObject *r = PyObject_CallFunctionObjArgs(this->pyFunction, po, NULL);
    if(!r) {
      fputs("** ArPyPacketHandlerFunctor: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
    Py_DECREF(po);
    return (r == Py_True);
  }
  virtual bool invokeR() {
    fputs("** ArPyPacketHandlerFunctor: invokeR: No argument supplied", stderr);
    Py_FatalError("ArPyPacketHandlerFunctor invoked with no arguments!");
    return false;
  }
  virtual void invoke() {
    fputs("** ArPyPacketHandlerFunctor: invoke: No argument supplied?", stderr);
    Py_FatalError("ArPyPacketHandlerFunctor invoked with no arguments!");
  }
  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};
#endif

// XXX TODO supply reference/pointer in constructor to Python library conversion function to convert to Python
// type (e.g. ...FromInt, FromLong, FromInt, etc.)
template <typename T1>
class ArPyFunctor1 : 
  public virtual ArFunctor1<T1>,
  public virtual ArPyFunctor
{
public:
  ArPyFunctor1(PyObject* pyfunc) : ArFunctor1<T1>(), ArPyFunctor(pyfunc) {
  }

  virtual void invoke(T1 arg) {
    puts("ArPyFunctor1<> invoked");
    fflush(stdout);
    PyObject* r = PyObject_CallFunctionObjArgs(pyFunction, arg, NULL);  
    if(!r) {
      fputs("** ArPyFunctor1: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
  }

  virtual void invoke() {
    fputs("** ArPyFunctor1: invoke: No argument supplied?", stderr);
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};

template <typename T1, typename T2>
class ArPyFunctor2 : 
  public virtual ArFunctor2<T1, T2>,
  public virtual ArPyFunctor
{
public:
  ArPyFunctor2(PyObject* _m) : ArFunctor2<T1, T2>(), ArPyFunctor(_m) {
  }

  virtual void invoke(T1 arg1, T2 arg2) {
    PyObject* r = PyObject_CallFunctionObjArgs(pyFunction, arg1, arg2, NULL);  
    if(!r) {
      fputs("** ArPyFunctor2: invoke: Error calling Python function: ", stderr);
      PyErr_Print();
    }
  }

  virtual void invoke() {
    fputs("** ArPyFunctor2: invoke: No argument supplied?", stderr);
    Py_FatalError("ArPyFunctor2 invoked with no arguments!");
  }

  virtual void invoke(T1 arg1) {
    fputs("** ArPyFunctor2: invoke: No argument supplied?", stderr);
    Py_FatalError("ArPyFunctor2 invoked with not enough arguments!");
  }

  virtual const char* getName() {
    return (const char*) PyString_AsString(PyObject_Str(pyFunction));
  }
};

#endif // PYTHON




#endif // wrapperFunctors.h

