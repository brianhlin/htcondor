/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
 * CONDOR Copyright Notice
 *
 * See LICENSE.TXT for additional notices and disclaimers.
 *
 * Copyright (c)1990-1998 CONDOR Team, Computer Sciences Department, 
 * University of Wisconsin-Madison, Madison, WI.  All Rights Reserved.  
 * No use of the CONDOR Software Program Source Code is authorized 
 * without the express consent of the CONDOR Team.  For more information 
 * contact: CONDOR Team, Attention: Professor Miron Livny, 
 * 7367 Computer Sciences, 1210 W. Dayton St., Madison, WI 53706-1685, 
 * (608) 262-0856 or miron@cs.wisc.edu.
 *
 * U.S. Government Rights Restrictions: Use, duplication, or disclosure 
 * by the U.S. Government is subject to restrictions as set forth in 
 * subparagraph (c)(1)(ii) of The Rights in Technical Data and Computer 
 * Software clause at DFARS 252.227-7013 or subparagraphs (c)(1) and 
 * (2) of Commercial Computer Software-Restricted Rights at 48 CFR 
 * 52.227-19, as applicable, CONDOR Team, Attention: Professor Miron 
 * Livny, 7367 Computer Sciences, 1210 W. Dayton St., Madison, 
 * WI 53706-1685, (608) 262-0856 or miron@cs.wisc.edu.
****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/

/*
  List Container Class

  Operations provided:
	General
	  Construct		- construct an empty list
	  Destruct		- delete the list, but not the objects contained in it
	  Append		- append one element
	  Display		- primitive display of contents of list - for debugging
	  IsEmpty		- return true if list is empty and false otherwise
	  Number		- return the number of elements in the list
	Scans
	  Rewind		- set scan pointer before beginning first obj in list
	  Next			- advance scan pointer one obj & return ref or ptr to it
	  Current		- return ref or ptr to current object
	  AtEnd			- true only if scan pointer is at end of list
	  DeleteCurrent - remove item at scan pointer from list

  Notes:
	1. The only way to add objects to the list is with Append, the only
	   way to "read" objects from the list is with Next.
	2. Two versions of Append are supplied.  One takes a reference to an
	   object, and is useful for objects allocated on the stack or
	   in global data space.  The other takes a pointer to an object,
	   and is useful for objects allocated on the heap, and other objects
	   which are most natural to deal with as pointers such as strings.
	3. Two versions of Next are also supplied, again one is for dealing
	   with pointers to objects while the other is for references.
	4. The DeleteCurrent operator deletes the element at the scan pointer
	   from the list.  It should only be called when the scan pointer is
	   valid, i.e. not on an empty list or after a rewind.
	5. Effects on the scan pointer of various operators are as follows:
		Rewind - The scan pointer is moved to a position before the first
			element in the list.
		List - The list constructor places the scan in the "rewound" condition.
		Next - The next operator moves the scan pointer one element forward
			and returns a pointer or reference to the element at the new
			location.
		Append - The scan pointer is placed at the newly entered item in the
			list.
		DeleteCurrent - The element at the scan pointer is removed from the
			list, and if possible the scan pointer is moved to the previous
			element.  If as a result of a DeleteCurrent the list becomes
			empty, then the resulting condition of the scan is "rewound".
			The DeleteCurrent operator is invalid if the scan is in the
			"rewound" condition since the pointer doesn't point to any
			element.  After a DeleteCurrent a call to Next will have the
			same result as if the DeleteCurrent had not been called.
	Examples:

			// To build a list containing 'a', 'b', 'c' using references:
		List<char>	foo;
		foo.Append( 'a' );
		foo.Append( 'b' );
		foo.Append( 'c' );

			// To "read" the list:
		char x;
		foo.Rewind();
		while( foo.Next(x) ) {
			cout << "Found element " << x << endl;
		}

			// To remove every element from the list - but not delete it
			// Note the the DeleteCurrent operator has no effect on the
			// subsequent call to Next.
		foo.Rewind();
		while( foo.Next(x) ) {
			cout << "Removing element " << x << endl;
			foo.DeleteCurrent();
		}
		assert( foo.IsEmpty );

			// To build a list containing "alpha", "bravo", "charlie" using
			// pointers.
		List<char> bar;
		bar.Append( "alpha" );
		bar.Append( "bravo" );
		bar.Append( "charlie" );

			// To scan this list
		for( bar.Rewind(); x = bar.Next(); cout << x << endl )
			;
*/

#ifndef LIST_H
#define LIST_H
#include <assert.h>

template <class ObjType> class Item;
template <class ObjType> class ListIterator;

template <class ObjType>
class List {
friend class ListIterator<ObjType>;
public:
		// General
	List();
	virtual ~List();
	bool	Append( ObjType & obj );
	bool	Append( ObjType * obj );

    /// Insert an element before the current element
	/// Warning: if AtEnd(), Insert() will add the element before the
	/// last element, not at the end of the list.  In that case, use
	/// Append() instead
	void	Insert( ObjType & obj );
	void	Insert( ObjType * obj );
	bool	IsEmpty() const;
	int		Number() const;

		// Scans
	void	Rewind();
	bool	Current( ObjType & obj ) const;
	ObjType *Current() const;

	ObjType *   Next ();
    bool        Next (ObjType   & obj);
    inline bool Next (ObjType * & obj) { return (obj = Next()) != NULL; }

	bool	AtEnd() const;
	void	DeleteCurrent();

		// Debugging
#if 0
	void	Display() const;
#endif

private:
	void	RemoveItem( Item<ObjType> * );
	Item<ObjType>	*dummy;
	Item<ObjType>	*current;
	int				num_elem;
};

template <class ObjType>
class Item {
friend class List<ObjType>;
friend class ListIterator<ObjType>;
public:
	Item( ObjType *obj = 0 );
	~Item();
private:
	Item<ObjType>	*next;
	Item<ObjType>	*prev;
	ObjType	*obj;
};

template <class ObjType>
class ListIterator {
public:
	ListIterator( );
	ListIterator( const List<ObjType>& );
	~ListIterator( );

	void Initialize( const List<ObjType>& );
	void ToBeforeFirst( );
	void ToAfterLast( );

	ObjType *   Next ();
	bool        Next (ObjType   & obj);
    inline bool Next (ObjType * & obj) { return (obj = Next()) != NULL; }

	bool Current( ObjType& ) const;
	ObjType* Current( ) const;
	bool Prev( ObjType& );
	ObjType* Prev( );
	
	bool IsBeforeFirst( ) const;
	bool IsAfterLast( ) const;

private:
	const List<ObjType>* list;
	Item<ObjType>* cur;
};
	

/*
  Implementation of the List class begins here.  This is so that every
  file which uses the class has enough information to instantiate the
  the specific instances it needs.  (This is the g++ way of instantiating
  template classes.)
*/

/*
  Each "item" holds a pointer to a single object in the list.
*/
template <class ObjType>
Item<ObjType>::Item( ObjType *obj )
{
	this->next = this;
	this->prev = this;
	this->obj = obj;
	// cout << "Constructed Item" << endl;
}

/*
  Since "items" don't allocate any memory, the default destructor is
  fine.  We just define this one in case we want the debugging printout.
*/
template <class ObjType>
Item<ObjType>::~Item()
{
	// cout << "Destructed Item" << endl;
}

/*
  The empty list contains just a dummy element which has a null pointer
  to an object.
*/
template <class ObjType>
List<ObjType>::List()
{
	dummy = new Item<ObjType>( 0 );
	dummy->next = dummy;
	dummy->prev = dummy;
	current = dummy;
	// cout << "Constructed List" << endl;
	num_elem = 0;
}


template <class ObjType>
int
List<ObjType>::Number() const
{
	return num_elem;
}

/*
  The list is empty if the dummy is the only element.
*/
template <class ObjType>
bool
List<ObjType>::IsEmpty() const
{
	return dummy->next == dummy;
}

/*
  List Destructor - remove and destruct every item, including the dummy.
*/
template <class ObjType>
List<ObjType>::~List()
{
	while( !IsEmpty() ) {
		RemoveItem( dummy->next );
	}
	delete dummy;
	// cout << "Destructed list" << endl;
}

/*
  Append an object given a reference to it.  Since items always
  contain pointers and not references, we generate a pointer to the
  object.  The scan pointer is left at the new item.
*/
template <class ObjType>
bool
List<ObjType>::Append( ObjType & obj )
{
	Item<ObjType>	*item;

	// cout << "Entering Append (reference)" << endl;
	item = new Item<ObjType>( &obj );
    if (item == NULL) return false;
	dummy->prev->next = item;
	item->prev = dummy->prev;
	dummy->prev = item;
	item->next = dummy;
	current = item;
	num_elem++;
    return true;
}

/*
  Append an object given a pointer to it.  The scan pointer is left at
  the new item.
*/
template <class ObjType>
bool
List<ObjType>::Append( ObjType * obj )
{
	Item<ObjType>	*item;

	// cout << "Entering Append (pointer)" << endl;
	item = new Item<ObjType>( obj );
    if (item == NULL) return false;
	dummy->prev->next = item;
	item->prev = dummy->prev;
	dummy->prev = item;
	item->next = dummy;
	current = item;
	num_elem++;
    return true;
}

/* Insert an element before the current element */
template <class ObjType>
void
List<ObjType>::Insert( ObjType& obj )
{
	Item<ObjType>	*item;
	item = new Item<ObjType>( &obj );
	current->prev->next = item;
	item->prev = current->prev;
	current->prev = item;
	item->next = current;
	num_elem++;
}

/* Insert an element before the current element */
template <class ObjType>
void
List<ObjType>::Insert( ObjType * obj )
{
	Item<ObjType>	*item;
	item = new Item<ObjType>( obj );
	current->prev->next = item;
	item->prev = current->prev;
	current->prev = item;
	item->next = current;
	num_elem++;
}

/*
  Primitive display function.  Depends on the "<<" operator being defined
  for the objects in the list.
*/
#if 0
template <class ObjType>
void
List<ObjType>::Display() const
{
	Item<ObjType>	*p = dummy->next;

	cout << "{ ";
	while( p != dummy ) {
		cout << (ObjType)(*p->obj);
		if( p->next == dummy ) {
			cout << " ";
		} else {
			cout << ", ";
		}
		p = p->next;
	}
	cout << "} ";
	if( !IsEmpty() ) {
		cout << "Current = " << (ObjType)(*current->obj);
	}
	cout << endl;
}
#endif

/*
  Move the scan pointer just before the beginning of the list.
*/
template <class ObjType>
void
List<ObjType>::Rewind()
{
	current = dummy;
}

/*
  Generate a reference to the current object in the list.  Returns true
  if there is a current object, and false if the list is empty or the
  scan pointer has been rewound.
*/
template <class ObjType>
bool
List<ObjType>::Current( ObjType & answer ) const
{
	if( IsEmpty() ) {
		return false;
	}

		// scan pointer has been rewound
	if( current == dummy ) {
		return false;
	}

	answer =  (*current->obj);
	return true;
}

/*
  Generate a reference to the next object in the list.  Returns true if
  there is a next object, and false if the list is empty or the scan pointer
  is already at the end.
*/
template <class ObjType>
bool
List<ObjType>::Next( ObjType & answer )
{
	if( AtEnd() ) {
		return false;
	}

	current = current->next;
	answer =  *current->obj;
	return true;
}

/*
  Return a pointer to the current object on the list.  Returns NULL if
  the list is empty or the scan pointer has been rewound.
*/
template <class ObjType>
ObjType *
List<ObjType>::Current() const
{
	if( IsEmpty() ) {
		return 0;
	}

	return current->obj;
}

/*
  Return a pointer to the next object on the list.  Returns NULL if the
  list is empty or the scan pointer is already at the end of the list.
*/
template <class ObjType>
ObjType *
List<ObjType>::Next()
{
	if( AtEnd() ) {
		return 0;
	}

	current = current->next;
	return current->obj;
}

/*
  Return true if the scan pointer is at the end of the list or the list
  is empty, and false otherwise.
*/
template <class ObjType>
bool
List<ObjType>::AtEnd() const
{
	return current->next == dummy;
}

/*
  This private function removes and destructs the item pointer to.
*/
template <class ObjType>
void
List<ObjType>::RemoveItem( Item<ObjType> * item )
{
	assert( item != dummy );

	item->prev->next = item->next;
	item->next->prev = item->prev;
	delete item;
	num_elem--;
}

/*
  Delete the item pointer to by the scan pointer.  Not valid is the
  scan is "rewound".  The pointer is left at the element previous to
  the removed element, or the scan is left in the "rewound" condition
  if the element removed was the first element.
*/
template <class ObjType>
void
List<ObjType>::DeleteCurrent()
{
	assert( current != dummy );
	current = current->prev;
	RemoveItem( current->next );
}

template <class ObjType>
ListIterator<ObjType>::ListIterator( )
{
	list = NULL;
	cur = NULL;
}

template <class ObjType>
ListIterator<ObjType>::ListIterator( const List<ObjType>& obj )
{
	list = &obj;
	cur = obj.dummy;
}

template <class ObjType>
ListIterator<ObjType>::~ListIterator( )
{
}

template <class ObjType> 
void
ListIterator<ObjType>::Initialize( const List<ObjType>& obj )
{
	list = &obj;
	cur = list->dummy;
}

template <class ObjType>
void
ListIterator<ObjType>::ToBeforeFirst( )
{
	if( list ) cur = list->dummy;
}

template <class ObjType>
void
ListIterator<ObjType>::ToAfterLast( )
{
	cur = NULL;
}

template <class ObjType>
bool
ListIterator<ObjType>::Next( ObjType& obj)
{
	if( cur ) {
		if( ( cur = cur->next ) ) {
			obj = *(cur->obj);
			return true;
		}
	} 
	return false;
}
		
template <class ObjType>
ObjType*
ListIterator<ObjType>::Next( )
{
	if( cur ) {
		if( ( cur = cur->next ) ) {
			return( cur->obj );
		}
	}
	return NULL;
}

template <class ObjType>
bool
ListIterator<ObjType>::Current( ObjType& obj ) const
{
	if( list && cur && cur != list->dummy ) {
		obj = *(cur->obj);
		return true;
	}
	return false;
}

template <class ObjType>
ObjType*
ListIterator<ObjType>::Current( ) const
{
	if( list && cur && cur != list->dummy ) {
		return( cur->obj );
	} 
	return NULL;
}


template <class ObjType>
bool
ListIterator<ObjType>::Prev( ObjType& obj ) 
{
	if( cur == NULL ) {
		cur = list->dummy;
	}
	cur = cur->prev;
	if( cur != list->dummy ) {
		obj = *(cur->obj);
		return true;
	}
	return false;
}


template <class ObjType>
ObjType*
ListIterator<ObjType>::Prev( )
{
	if( cur == NULL ) {
		cur = list->dummy;
	}
	cur = cur->prev;
	if( cur != list->dummy ) {
		return( cur->obj );
	}
	return NULL;
}
	
template <class ObjType>
bool
ListIterator<ObjType>::IsBeforeFirst( ) const
{
	return( list && list->dummy == cur );
}

template <class ObjType>
bool
ListIterator<ObjType>::IsAfterLast( ) const
{
	return( list && cur == NULL );
}


#endif /* LIST_H */
