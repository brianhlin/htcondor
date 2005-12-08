/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
  *
  * Condor Software Copyright Notice
  * Copyright (C) 1990-2004, Condor Team, Computer Sciences Department,
  * University of Wisconsin-Madison, WI.
  *
  * This source code is covered by the Condor Public License, which can
  * be found in the accompanying LICENSE.TXT file, or online at
  * www.condorproject.org.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * AND THE UNIVERSITY OF WISCONSIN-MADISON "AS IS" AND ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  * WARRANTIES OF MERCHANTABILITY, OF SATISFACTORY QUALITY, AND FITNESS
  * FOR A PARTICULAR PURPOSE OR USE ARE DISCLAIMED. THE COPYRIGHT
  * HOLDERS AND CONTRIBUTORS AND THE UNIVERSITY OF WISCONSIN-MADISON
  * MAKE NO MAKE NO REPRESENTATION THAT THE SOFTWARE, MODIFICATIONS,
  * ENHANCEMENTS OR DERIVATIVE WORKS THEREOF, WILL NOT INFRINGE ANY
  * PATENT, COPYRIGHT, TRADEMARK, TRADE SECRET OR OTHER PROPRIETARY
  * RIGHT.
  *
  ****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/
#ifndef __EXT_ARRAY_H__
#define __EXT_ARRAY_H__

#include "condor_debug.h"

template <class Element>
class ExtArray 
{
  public:

	// ctors/dtor
	ExtArray (int = 64); // initial size of array
	ExtArray (const ExtArray &);
	~ExtArray ();

	// accessor functions
	Element operator[] (int) const;
	Element &operator[] (int);

	// control functions
	int	getsize (void) const;
	int	getlast (void) const;
	Element *getarray (void);
	void resize (int);
	void truncate (int);
	void fill (Element);
	void setFiller( Element );
	
	// Vector-like functions
	Element set( int, Element );
	Element add( Element );
	Element& getElementAt( int );

  private:

	Element *array;
	int 	size;
	int		last;
	Element filler;
};


template <class Element>
ExtArray<Element>::
ExtArray (int sz)
{
	// create array of required size
	size = sz;
	last = -1;
	array = new Element[size];
	if (!array)
	{
		dprintf (D_ALWAYS, "ExtArray: Out of memory");
		exit (1);
	}
}

template <class Element>
ExtArray<Element>::
ExtArray (const ExtArray &old)
{
	int i;

	// sanity check
	if (&old == this) return;

	// establish new array of required size;
	size = old.size;
	last = old.last;
	array = new Element[size];
	if (!array) 
	{
		dprintf (D_ALWAYS, "ExtArray: Out of memory");
		exit (1);
	}

	// copy elements over
	for (i = 0; i < size; i++) 
	{
		array[i] = old.array[i];
	}
	
	// copy filler element as well
	filler = old.filler;
}


template <class Element>
ExtArray<Element>::
~ExtArray ()
{
	delete [] array;
}


template <class Element>
inline Element ExtArray<Element>::
operator[] (int i) const
{
	// check bounds
	if (i < 0) 
	{
		i = 0;
	}
	else if (i > last) 
	{
		return filler;
	}

	// index array
	return array[i];
}


template <class Element>
inline Element& ExtArray<Element>::
operator[] (int i)
{
	// check bounds ...
	if (i < 0) 
	{
		i = 0;
	}
	else 
	if (i >= size) 
	{
		resize (2*i);
	}

	// index array
	if( i > last ) last = i;
	return array[i];
}

//
// set()
// Set a value for an element at a specific position
// Will return the original value if there is one
//
template <class Element>
Element ExtArray<Element>::
set( int idx, Element elt ) 
{
		//
		// First we check bounds
		// I don't think that it should changing the index if 
		// we are below zero, but I need to keep this backwards
		// compatiable with the old code
		//
	if ( idx < 0 ) {	
		idx = 0;
		//
		// If we are not large enough for this position, increase
		// our size
		//
	} else if ( idx >= this->size ) {
		this->resize( idx * 2 );
	}
		//
		// If this new value is the largest position, we'll 
		// set this value as the new last
		//
	if ( idx > this->last ) {
		this->last = idx;
	}
	
		//
		// Get the old value, then save the new value
		//
	Element orig = this->array[idx];
	this->array[idx] = elt;
	return ( orig );
}

//
// add()
// This is so you can append things to the end of the array
// without having to keep track of your current size
//
template <class Element>
Element ExtArray<Element>::
add( Element elt ) 
{
		//
		// Take the last size and add 1
		// We'll pass this to set who will take care of everything
		// for us
		//
	return ( this->set( this->last + 1, elt ) );
}	

//
// getElementAt()
//
//
template <class Element>
inline Element& ExtArray<Element>::
getElementAt( int idx )
{
		//
		// We will check bounds but not resize the array
		//
	if ( idx < 0 )  {
		idx = 0;
	} else if ( idx >= size ) {
		idx = size;
	}
	return ( array[idx] );
}

template <class Element>
inline int ExtArray<Element>::
getsize (void) const
{
	return size;
}


template <class Element>
inline int ExtArray<Element>::
getlast (void) const
{
	return last;
}

template <class Element>
inline Element *ExtArray<Element>::
getarray (void) 
{
	return array;
}


template <class Element>
void ExtArray<Element>::
resize (int newsz) {
	Element *newarray = new Element[newsz];
	int		index = (size < newsz) ? size : newsz;
	int i;

	// check if memory allocation was successful
	if (!newarray) 
	{
		dprintf (D_ALWAYS, "ExtArray: Out of memory");
		exit (1);
	}

	// if the new array is larger, and a 'filler' value is
	// defined, initialize with the filler
	for (i=index;i<newsz;i++) 
	{
		newarray[i] = filler;
	}

	// copy elements over to new array
	while (--index >= 0) 
	{
		newarray[index] = array[index];
	}

	// use new array
	delete [] array;
	size = newsz;	
	array = newarray;
}


template <class Element>
void ExtArray<Element>::
truncate (int newlast)
{
	last = newlast;
}
		

template <class Element>
void ExtArray<Element>::
fill (Element elt)
{
	int i;
	for (i = 0; i < size; i++)
		array[i] = elt;
	filler = elt;
}

template <class Element>
void ExtArray<Element>::
setFiller( Element elt )
{
	filler = elt;
}

#endif//__EXT_ARRAY_H__
