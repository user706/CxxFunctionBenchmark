// bslstl_sharedptr.h                                                 -*-C++-*-
#ifndef INCLUDED_BSLSTL_SHAREDPTR
#define INCLUDED_BSLSTL_SHAREDPTR

#ifndef INCLUDED_BSLS_IDENT
#include <bsls_ident.h>
#endif
BSLS_IDENT("$Id$ $CSID$")

//@PURPOSE: Provide a generic reference-counted shared pointer wrapper.
//
//@CLASSES:
//  bsl::enable_shared_from_this: base class to allow shared ownership of self
//  bsl::shared_ptr: shared pointer
//  bsl::weak_ptr: "weak" reference to reference-counted shared object
//  bslstl::SharedPtrUtil: shared pointer utility functions
//  bslstl::SharedPtrNilDeleter: no-op deleter
//
//@SEE_ALSO: bslma_managedptr, bslma_sharedptrrep
//
//@DESCRIPTION: This component implements a thread-safe, generic,
// reference-counted "smart pointer" to support "shared ownership" of objects
// of (template parameter) 'ELEMENT_TYPE'.  Shared pointers implement a form of
// the "envelope/letter" idiom.  For each shared object, a representation that
// manages the number of references to it is created.  Many shared pointers can
// simultaneously refer to the same shared object by storing a reference to the
// same representation.  Shared pointers also implement the "construction is
// acquisition, destruction is release" idiom.  When a shared pointer is
// created it increments the number of shared references to the shared object
// that was specified to its constructor (or was referred to by a shared
// pointer passed to the copy constructor).  When a shared pointer is assigned
// to or destroyed, then the number of shared references to the shared object
// is decremented.  When all references to the shared object are released, both
// the representation and the object are destroyed.  'bsl::shared_ptr' emulates
// the interface of a native pointer.  The shared object may be accessed
// directly using the '->' operator, or the dereference operator (operator '*')
// can be used to obtain a reference to the shared object.
//
// This component also provides a mechanism, 'bsl::weak_ptr', used to create
// weak references to reference-counted shared ('bsl::shared_ptr') objects.  A
// weak reference provides conditional access to a shared object managed by a
// 'bsl::shared_ptr', but, unlike a shared (or "strong") reference, does not
// affect the shared object's lifetime.  An object having even one shared
// reference to it will not be destroyed, but an object having only weak
// references would have been destroyed when the last shared reference was
// released.
//
// A weak pointer can be constructed from another weak pointer or a
// 'bsl::shared_ptr'.  To access the shared object referenced by a weak pointer
// clients must first obtain a shared pointer to that object using the 'lock'
// method.  If the shared object has been destroyed (as indicated by the
// 'expired' method), then 'lock' returns a shared pointer in the default
// constructed (empty) state.
//
// This component also provides a mechanism, 'bsl::enable_shared_from_this',
// which can be used to create a type that participates in its own ownership
// through the reference-counting of a 'shared_ptr'.
//
// This component also provides a functor, 'bslstl::SharedPtrNilDeleter', which
// may used to create a shared pointer that takes no action when the last
// shared reference is destroyed.
//
// This component also provides a utility class, 'bslstl::SharedPtrUtil', which
// provides several functions that are frequently used with shared pointers.
//
//
///Thread Safety
///-------------
// This section qualifies the thread safety of 'bsl::shared_ptr' objects and
// 'bsl::weak_ptr' objects themselves rather than the thread safety of the
// objects being referenced.
//
// It is safe to access or modify two distinct 'shared_ptr' (or
// 'bsl::weak_ptr') objects simultaneously, each from a separate thread, even
// if they share ownership of a common object.  It is safe to access a single
// 'bsl::shared_ptr' (or 'bsl::weak_ptr') object simultaneously from two or
// more separate threads, provided no other thread is simultaneously modifying
// the object.  It is not safe to access or modify a 'bsl::shared_ptr' (or
// 'bsl::weak_ptr') object in one thread while another thread modifies the same
// object.
//
// It is safe to access, modify, copy, or delete a shared pointer (or weak
// pointer) in one thread, while other threads access or modify other shared
// pointers and weak pointers pointing to or managing the same object (the
// reference count is managed using atomic operations).  However, there is no
// guarantee regarding the safety of accessing or modifying the object
// *referred* *to* by the shared pointer simultaneously from multiple threads.
//
///Shared and Weak References
///--------------------------
// There are two types of references to shared objects:
//
// 1) A shared reference allows users to share the ownership of an object and
// control its lifetime.  A shared object is destroyed only when the last
// shared reference to it is released.  A shared reference to an object can be
// obtained by creating a 'shared_ptr' referring to it.
//
// 2) A weak reference provides users conditional access to an object without
// sharing its ownership (or affecting its lifetime).  A shared object can be
// destroyed even if there are weak references to it.  A weak reference to an
// object can be obtained by creating a 'weak_ptr' referring to the object from
// a 'shared_ptr' referring to that object.
//
///In-place/Out-of-place Representations
///-------------------------------------
// 'shared_ptr' provides two types of representations: an out-of-place
// representation, and an in-place representation.  Out-of-place
// representations are used to refer to objects that are constructed externally
// to their associated representations.  Out-of-place objects are provided to a
// shared pointer by passing their address along with the deleter that should
// be used to destroy the object when all references to it have been released.
// In-place objects can be constructed directly within a shared pointer
// representation (see 'createInplace').
//
// Below we provide a diagram illustrating the differences between the two
// representations for a shared pointer to an 'int'.  First we create an 'int'
// object on the heap, initialized to 10, and pass its address to a shared
// pointer constructor, resulting in an out-of-place representation for the
// shared object:
//..
//  bslma::NewDeleteAllocator nda;
//  int *value = new (nda) int(10);
//  shared_ptr<int> outOfPlaceSharedPtr(value, &nda);
//..
// Next we create an in-place representation of a shared 'int' object that is
// also initialized to 10:
//..
//  shared_ptr<int> inPlaceSharedPtr;
//  inPlaceSharedPtr.createInplace(&nda, 10);
//..
// The memory layouts of these two representations are shown below (where
// 'd_ptr_p' refers to the shared object and 'd_rep_p' refers to the
// representation):
//..
//  Out-of-Place Representation                 In-Place Representation
//  ----------------------------              ----------------------------
//
//  +------------+                            +------------+
//  |            |                            |            |
//  |  d_ptr_p ------>+-----------+           |  d_ptr_p ---------+
//  |            |    |     10    |           |            |      |
//  |            |    +-----------+           |            |      |
//  |            |                            |            |      |
//  |  d_rep_p ------>+-----------+           |  d_rep_p ------>+-v---------+
//  |            |    | reference |           |            |    |+---------+|
//  |            |    |  counts   |           |            |    ||    10   ||
//  +------------+    +-----------+           +------------+    |+---------+|
//                                                              | reference |
//                                                              |  counts   |
//                                                              +-----------+
//..
// An out-of-place representation is generally less efficient than an in-place
// representation since it usually requires at least two allocations (one to
// construct the object and one to construct the shared pointer representation
// for the object).
//
// Creating an in-place shared pointer does not require the template parameter
// type to inherit from a special class (such as
// 'bsl::enable_shared_from_this'); in that case, 'shared_ptr' supports up to
// fourteen arguments that can be passed directly to the object's constructor.
// For in-place representations, both the object and the representation can be
// constructed in one allocation as opposed to two, effectively creating an
// "intrusive" reference counter.  Note that the size of the allocation is
// determined at compile-time from the combined footprint of the object and of
// the reference counts.  It is also possible to create shared pointers to
// buffers whose sizes are determined at runtime, although such buffers consist
// of raw (uninitialized) memory.
//
///Weak Pointers using "in-place" or Pooled Shared Pointer Representations
///- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// A weak pointer that is not in the empty state shares a common representation
// (used to refer to the shared object) with the shared (or other weak) pointer
// from which it was constructed, and holds this representation until it is
// either destroyed or reset.  This common representation is not destroyed and
// deallocated (although the shared object itself may have been destroyed)
// until all weak references to that common representation have been released.
//
// Due to this behavior the *memory* *footprint* of shared objects that are
// constructed "in-place" in the shared pointer representation (see above) is
// not deallocated until all weak references to that shared object are
// released.  Note that a shared object is always destroyed when the last
// shared reference to it is released.  Also note that the same behavior
// applies if the shared object were obtained from a class that pools shared
// pointer representations (for example, 'bcec_SharedObjectPool').
//
// For example suppose we have a class with a large memory footprint:
//..
//  class ClassWithLargeFootprint {
//      // This class has a large memory footprint.
//
//      // TYPES
//      enum { BUFFER_SIZE = 1024 };
//          // The size of the buffer owned by this 'class'.
//
//      // DATA
//      char d_buffer[BUFFER_SIZE];
//
//      // ...
//  };
//..
// We then create an "in-place" shared pointer to an object of
// 'ClassWithLargeFootprint' using the 'createInplace' method of 'shared_ptr'.
// The 'sp' shared pointer representation of 'sp' will create a
// 'ClassWithLargeFootprint' object "in-place":
//..
//  shared_ptr<ClassWithLargeFootprint> sp;
//  sp.createInplace();
//..
// Next we construct a weak pointer from this (in-place) shared pointer:
//..
//  weak_ptr<ClassWithLargeFootprint> wp(sp);
//..
// Now releasing all shared references to the shared object (using the 'reset'
// function) causes the object's destructor to be called, but the
// representation is not destroyed (and the object's footprint is not
// deallocated) until 'wp' releases its weak reference:
//..
//  sp.reset(); // The object's footprint is not deallocated until all weak
//              // references to it are released.
//
//  wp.reset(); // The release of the *last* weak reference results in the
//              // destruction and deallocation of the representation and the
//              // object's footprint.
//..
// If a shared object has a large footprint, and the client anticipates there
// will be weak references to it, then an out-of-place shared pointer
// representation may be preferred because it destroys the shared object and
// deallocates its footprint when the last *shared* reference is released,
// regardless of whether there are any outstanding weak references to the same
// representation.
//
///Correct Usage of the Allocator Model
///------------------------------------
// Note that once constructed, there is no difference in type, usage, or
// efficiency between in-place and out-of-place shared pointers, except that an
// in-place shared pointer will exhibit greater locality of reference and
// faster destruction (because there is only one allocated block).  Also note
// that an object created with an allocator needs to have this allocator
// specified as its last constructor argument, but this allocator may be
// different from the one passed as the first argument to 'createInplace'.
//
// For example, consider the following snippet of code:
//..
//  bslma::Allocator *allocator1, *allocator2;
//  // ...
//  shared_ptr<bsl::string> ptr;
//  ptr.createInplace(allocator1, bsl::string("my string"), allocator2);
//..
// Here 'allocator1' is used to obtain the shared pointer representation and
// the in-place 'bsl::string' object, and 'allocator2' is used by the
// 'bsl::string' object (having the value "my string") for its memory
// allocations.  Typically, both allocators will be the same, and so the same
// allocator will need to be specified twice.
//
///Deleters
///--------
// When the last shared reference to a shared object is released, the object is
// destroyed using the "deleter" provided when the associated shared pointer
// representation was created.  'shared_ptr' supports two kinds of "deleter"
// objects, which vary in how they are invoked.  A "function-like" deleter is
// any language entity that can be invoked such that the expression
// 'deleterInstance(objectPtr)' is a valid expression.  A "factory" deleter is
// any language entity that can be invoked such that the expression
// 'deleterInstance.deleteObject(objectPtr)' is a valid expression, where
// 'deleterInstance' is an instance of the "deleter" object, and 'objectPtr' is
// a pointer to the shared object.  Factory deleters are a BDE extension to the
// ISO C++ Standard Library specification for 'shared_ptr'.  In summary:
//..
//  Deleter                     Expression used to destroy 'objectPtr'
//  - - - - - - - - - - - -     - - - - - - - - - - - - - - - - - - -
//  "function-like"             deleterInstance(objectPtr);
//  "factory"                   deleterInstance.deleteObject(objectPtr);
//..
// The following are examples of function-like deleters that delete an object
// of 'my_Type':
//..
//  void deleteObject(my_Type *object);
//      // Delete the specified 'object'.
//
//  void releaseObject(my_Type *object);
//      // Release the specified 'object'.
//
//  struct FunctionLikeDeleterObject {
//      // This 'struct' provides an 'operator()' that can be used to delete a
//      // 'my_Type' object.
//
//      void operator()(my_Type *object);
//          // Destroy the specified 'object'.
//  };
//..
// The following, on the other hand is an example of a factory deleter:
//..
//  class my_Factory {
//
//     // . . .
//
//     // MANIPULATORS
//     my_Type *createObject(bslma::Allocator *basicAllocator = 0);
//         // Create a 'my_Type' object.  Optionally specify a
//         // 'basicAllocator' used to supply memory.  If 'basicAllocator' is
//         // 0, the currently installed default allocator is used.
//
//     void deleteObject(my_Type *object);
//         // Delete the specified 'object'.
//  };
//
//  class my_Allocator : public bslma::Allocator { /* ... */ };
//..
// Note that 'deleteObject' is provided by all 'bslma' allocators and by any
// object that implements the 'bcema_Deleter' protocol.  Thus, any of these
// objects can be used as a factory deleter.  The purpose of this design is to
// allow 'bslma' allocators and factories to be used seamlessly as deleters.
//
// The selection of which expression is used by 'shared_ptr' to destroy a
// shared object is based on how the deleter is passed to the shared pointer
// object: Deleters that are passed by *address* are assumed to be factory
// deleters (unless they are function pointers), while those that are passed by
// *value* are assumed to be function-like.  Note that if the wrong interface
// is used for a deleter, i.e., if a function-like deleter is passed by
// pointer, or a factory deleter is passed by value, and the expression used to
// delete the object is invalid, a compiler diagnostic will be emitted
// indicating the error.
//
// In general, deleters should have defined behavior when called with a null
// pointer.  In all cases, throwing an exception out of a copy constructor for
// a deleter will yield undefined behavior.
//
// The following are examples of constructing shared pointers with the
// addresses of factory deleters:
//..
//  my_Factory factory;
//  my_Type *myPtr1 = factory.createObject();
//  shared_ptr<my_Type> mySharedPtr1(myPtr1, &factory, 0);
//
//  bdema_SequentialAllocator sa;
//  my_Type *myPtr2 = new (sa) my_Type(&sa);
//  shared_ptr<my_Type> mySharedPtr2(myPtr2, &sa);
//..
// Note that the deleters are passed *by address* in the above examples.
//
// The following are examples of constructing shared pointers with
// function-like deleters:
//..
//  my_Type *getObject(bslma::Allocator *basicAllocator = 0);
//
//  my_Type *myPtr3 = getObject();
//  shared_ptr<my_Type> mySharedPtr3(myPtr3, &deleteObject);
//
//  my_Type *myPtr4 = getObject();
//  FunctionLikeDeleterObject deleter;
//  shared_ptr<my_Type> mySharedPtr4(myPtr4, deleter, &sa);
//..
// Note that 'deleteObject' is also passed by address, but 'deleter' is passed
// by value in the above examples.  Function-like deleter objects (passed by
// value) are stored by value in the representation and therefore *must* be
// copy-constructible.  Note that even though the deleter may be passed by
// reference, it is a copy (owned by the shared pointer representation) that is
// invoked and thus the 'deleterInstance' is not required, nor assumed, to be
// non-modifiable.  (For the example above, note that 'operator()' is
// intentionally *not* defined 'const'.)
//
///Aliasing
///--------
// 'shared_ptr' supports a powerful "aliasing" feature.  That is, a shared
// pointer can be constructed to refer to a shared object of a certain type
// while the shared pointer representation it holds refers to a shared object
// of any (possibly different) type.  All references are applied to the
// "aliased" shared object referred to by the representation and is used for
// reference counting.  This "aliased" shared object is passed to the deleter
// upon destruction of the last instance of that shared pointer.  Consider the
// following snippet of code:
//..
//  class Event { /* ... */ };
//  void getEvents(bsl::list<Event> *list);
//
//  void enqueueEvents(bcec_Queue<shared_ptr<Event> > *queue)
//  {
//      bsl::list<Event> eventList;
//      getEvents(&eventList);
//      for (bsl::list<Event>::iterator it = eventList.begin();
//           it != eventList.end();
//           ++it) {
//          shared_ptr<Event> e;
//          e.createInplace(0, *it);  // Copy construct the event into a new
//                                    // shared ptr.
//          queue->pushBack(e);
//      }
//  }
//..
// In the above example, 'getEvents' loads into the provided 'bsl::list' a
// sequence of event objects.  The 'enqueueEvents' function constructs an empty
// list and calls 'getEvents' to fill the list with 'Event' objects.  Once the
// event list is filled, each event item is pushed as a shared pointer
// (presumably because events are "expensive" to construct and may be
// referenced simultaneously from multiple threads) onto the provided queue.
// Since the individual event items are contained by value within the list,
// pointers to them cannot be passed if it cannot be guaranteed that they will
// not live beyond the lifetime of the list itself.  Therefore, an expensive
// copy operation is required to create individually-managed instances of each
// of the list items.  The 'createInplace' operation is used to reduce the
// number of required allocations, but this might still be too expensive.  Now
// consider the following alternate implementation of 'enqueueEvents' using the
// 'shared_ptr' aliasing feature:
//..
//  void enqueueEvents(bcec_Queue<shared_ptr<Event> > *queue)
//  {
//      shared_ptr<bsl::list<Event> > eventList;
//      eventList.createInplace(0);  // Construct a shared pointer
//                                   // to the event list containing
//                                   // all of the events.
//      getEvents(eventList.get());
//
//      for (bsl::list<Event>::iterator it = eventList->begin();
//           it != eventList->end();
//           ++it) {
//          // Push each event onto the queue as an alias of the 'eventList'
//          // shared pointer.  When all the alias references have been
//          // released, the event list will be destroyed deleting all the
//          // events at once.
//
//          queue->pushBack(shared_ptr<Event>(eventList, &*it));
//      }
//  }
//..
// In the implementation above, we create a single shared pointer to the
// 'Event' list, 'eventList', and use that to create 'Event' shared pointers
// that are aliased to 'eventList'.  The lifetime of each 'Event' object is
// then tied to the 'eventList' and it will not be destroyed until the
// 'eventList' is destroyed.
//
///Type Casting
///------------
// A 'shared_ptr' object of a given type can be implicitly or explicitly cast
// to a 'shared_ptr' of another type.
//
///Implicit Casting
/// - - - - - - - -
// As with native pointers, a shared pointer to a derived type can be directly
// assigned to a shared pointer to a base type.  In other words, if the
// following statements are valid:
//..
//  class A { virtual void foo(); };  // polymorphic type
//  class B : public A {};
//  B *bp = 0;
//  A *ap = bp;
//..
// then the following statements:
//..
//  shared_ptr<B> spb;
//  shared_ptr<A> spa;
//  spa = spb;
//..
// and:
//..
//  shared_ptr<B> spb;
//  shared_ptr<A> spa(spb);
//..
// are also valid.  Note that in all of the above cases, the destructor of 'B'
// will be invoked when the object is destroyed even if 'A' does not provide a
// virtual destructor.
//
///Explicit Casting
/// - - - - - - - -
// Through "aliasing", a shared pointer of any type can be explicitly cast to a
// shared pointer of any other type using any legal cast expression.  For
// example, to statically cast a shared pointer to type 'A' ('shared_ptr<A>')
// to a shared pointer to type 'B' ('shared_ptr<B>'), one can simply do the
// following:
//..
//  shared_ptr<A> spa;
//  shared_ptr<B> spb(spa, static_cast<B *>(spa.get()));
//..
// or even the less safe C-style cast:
//..
//  shared_ptr<A> spa;
//  shared_ptr<B> spb(spa, (B *)(spa.get()));
//..
// For convenience, several utility functions are provided to perform common
// C++ casts.  Dynamic casts, static casts, and 'const' casts are all provided.
// Explicit casting is supported through the 'bslstl::SharedPtrUtil' utility.
// The following example demonstrates the dynamic casting of a shared pointer
// to type 'A' ('shared_ptr<A>') to a shared pointer to type 'B'
// ('shared_ptr<B>'):
//..
//  bslma::NewDeleteAllocator nda;
//  shared_ptr<A> sp1(new (nda) A(), &nda);
//  shared_ptr<B> sp2 = bslstl::SharedPtrUtil::dynamicCast<B>(sp1);
//  shared_ptr<B> sp3;
//  bslstl::SharedPtrUtil::dynamicCast(&sp3, sp1);
//  shared_ptr<B> sp4;
//  sp4 = bslstl::SharedPtrUtil::dynamicCast<B>(sp1);
//..
// To test if the cast succeeded, simply test if the target shared pointer
// refers to a non-null value (assuming the source was not null, of course):
//..
//  if (sp2) {
//      // The cast succeeded.
//  } else {
//      // The cast failed.
//  }
//..
// As previously stated, the shared object will be destroyed correctly
// regardless of how it is cast.
//
///Converting to and from 'BloombergLP::bslma::ManagedPtr'
///-------------------------------------------------------
// A 'shared_ptr' can be converted to a 'BloombergLP::bslma::ManagedPtr' while
// still retaining proper reference counting.  When a shared pointer is
// converted to a 'BloombergLP::bslma::ManagedPtr', the number of references to
// the shared object is incremented.  When the managed pointer is destroyed (if
// not transferred to another managed pointer first), the number of references
// will be decremented.  If the number of references reaches zero, then the
// shared object will be destroyed.  The 'managedPtr' function can be used to
// create a managed pointer from a shared pointer.
//
// A 'shared_ptr' also can be constructed from a
// 'BloombergLP::bslma::ManagedPtr'.  The resulting shared pointer takes over
// the management of the object and will use the deleter from the original
// 'BloombergLP::bslma::ManagedPtr' to destroy the managed object when all the
// references to that shared object are released.
//
///Weak Pointers using "in-place" or Pooled Shared Pointer Representations
///-----------------------------------------------------------------------
// A weak pointer that is not in the empty state shares a common representation
// (used to refer to the shared object) with the shared (or other weak) pointer
// from which it was constructed, and holds this representation until it is
// either destroyed or reset.  This common representation is not destroyed and
// deallocated (although the shared object itself may have been destroyed)
// until all weak references to that common representation have been released.
//
// Due to this behavior the memory footprint of shared objects that are
// constructed "in-place" in the shared pointer representation (refer to the
// component-level documentation of 'bsl::shared_ptr' for more information on
// shared pointers with "in-place" representations) is not deallocated until
// all weak references to that shared object are released.  Note that a shared
// object is always destroyed when the last shared reference to it is released.
// Also note that the same behavior is applicable if the shared objects were
// obtained from a class that pools shared pointer representations (for
// example, 'bcec_SharedObjectPool').
//
// For example suppose we have a class with a large memory footprint:
//..
//  class ClassWithLargeFootprint {
//      // This class has a large memory footprint.
//
//      // TYPES
//      enum { BUFFER_SIZE = 1024 };
//          // The size of the buffer owned by this 'class'.
//
//      // DATA
//      char d_buffer[BUFFER_SIZE];
//
//      // ...
//  };
//..
// We then create an "in-place" shared pointer to an object of
// 'ClassWithLargeFootprint' using the 'createInplace' method of
// 'bsl::shared_ptr'.  The 'sp' shared pointer representation of 'sp' will
// create a 'ClassWithLargeFootprint' object "in-place":
//..
//  bsl::shared_ptr<ClassWithLargeFootprint> sp;
//  sp.createInplace();
//..
// Next we construct a weak pointer from this (in-place) shared pointer:
//..
//  bsl::weak_ptr<ClassWithLargeFootprint> wp(sp);
//..
// Now releasing all shared references to the shared object (using the 'reset'
// function) causes the object's destructor to be called, but the
// representation is not destroyed (and the object's footprint is not
// deallocated) until 'wp' releases its weak reference:
//..
//  sp.reset(); // The object's footprint is not deallocated until all weak
//              // references to it are released.
//
//  wp.reset(); // The release of the *last* weak reference results in the
//              // destruction and deallocation of the representation and the
//              // object's footprint.
//..
// If a shared object has a large footprint, and the client anticipates there
// will be weak references to it, then it may be advisable to create an
// out-of-place shared pointer representation, which destroys the shared object
// and deallocates its footprint when the last *shared* reference to it is
// released, regardless of whether there are any outstanding weak references to
// the same representation.
//
///C++ Standard Compliance
///-----------------------
// This component provides an (extended) standard-compliant implementation of
// 'std::shared_ptr' and 'std::weak_ptr' (section 20.7.2, [util.smartptr], of
// the ISO C++11 standard)).  However, it does not support the atomic shared
// pointer interface, nor provide the C++17 interface for 'shared_ptr' of an
// array type.  When using a C++03 compiler, its interface is limited to the
// set of operations that can be implemented by an implementation of the C++03
// language, e,g., there are no exception specifications, nor 'constexpr'
// constructors, and move operations are emulated with 'bslmf::MovableRef'.
//
// In addition to the standar interface, this component supports allocators
// following the 'bslma::Allocator' protocol in addition to the C++ Standard
// Allocators (section 17.6.3.5, [allocator.requirements]), supports "factory"
// style deleters in addition to function-like deleters, and interoperation
// with 'bslma::ManagedPtr' smart pointers.
//
///Usage
///-----
// The following examples demonstrate various features and uses of shared
// pointers.
//
///Example 1: Basic Usage
/// - - - - - - - - - - -
// The following example demonstrates the creation of a shared pointer.  First,
// we declare the type of object that we wish to manage:
//..
//  class MyUser {
//      // DATA
//      bsl::string d_name;
//      int         d_id;
//
//    public:
//      // CREATORS
//      MyUser(bslma::Allocator *alloc = 0) : d_name(alloc), d_id(0) {}
//      MyUser(const bsl::string& name, int id, bslma::Allocator *alloc = 0)
//      : d_name(name, alloc)
//      , d_id(id)
//      {
//      }
//      MyUser(const MyUser& original, bslma::Allocator *alloc = 0)
//      : d_name(original.d_name, alloc)
//      , d_id(original.d_id)
//      {
//      }
//
//      // MANIPULATORS
//      void setName(const bsl::string& name) { d_name = name; }
//      void setId(int id) { d_id = id; }
//
//      // ACCESSORS
//      const bsl::string& name() const { return d_name; }
//      int id() const { return d_id; }
//  };
//..
// The 'createUser' utility function (below) creates a 'MyUser' object using
// the provided allocator and returns a shared pointer to the newly-created
// object.  Note that the shared pointer's internal representation will also be
// allocated using the same allocator.  Also note that if 'allocator' is 0, the
// currently-installed default allocator is used.
//..
//  shared_ptr<MyUser> createUser(bsl::string       name,
//                                     int               id,
//                                     bslma::Allocator *allocator = 0)
//  {
//      allocator = bslma::Default::allocator(allocator);
//      MyUser *user = new (*allocator) MyUser(name, id, allocator);
//      return shared_ptr<MyUser>(user, allocator);
//  }
//..
// Since the 'createUser' function both allocates the object and creates the
// shared pointer, it can benefit from the in-place facilities to avoid an
// extra allocation.  Again, note that the representation will also be
// allocated using the same allocator (see the section "Correct Usage of the
// Allocator Model" above).  Also note that if 'allocator' is 0, the
// currently-installed default allocator is used.
//..
//  shared_ptr<MyUser> createUser2(bsl::string       name,
//                                      int               id,
//                                      bslma::Allocator *allocator = 0)
//  {
//      shared_ptr<MyUser> user;
//      user.createInplace(allocator, name, id, allocator);
//      return user;
//  }
//..
// Note that the shared pointer allocates both the reference count and the
// 'MyUser' object in a single region of memory (which is the memory that will
// eventually be deallocated), but refers to the 'MyUser' object only.
//
///Using Custom Deleters
///- - - - - - - - - - -
// The following examples demonstrate the use of custom deleters with shared
// pointers.
//
///Example 2: Nil Deleters
/// -  -  -  -  -  -  -  -
// There are cases when an interface calls for an object to be passed as a
// shared pointer, but the object being passed is not owned by the caller
// (e.g., a pointer to a static variable).  In these cases, it is possible to
// create a shared pointer specifying 'bslstl::SharedPtrNilDeleter' as the
// deleter.  The deleter function provided by 'bslstl::SharedPtrNilDeleter' is
// a no-op and does not delete the object.  The following example demonstrates
// the use of 'shared_ptr' using a 'bslstl::SharedPtrNilDeleter'.  The code
// uses the 'MyUser' class defined in Example 1.  In this example, an
// asynchronous transaction manager is implemented.  Transactions are enqueued
// into the transaction manager to be processed at some later time.  The user
// associated with the transaction is passed as a shared pointer.  Transactions
// can originate from the "system" or from "users".
//
// We first declare the transaction manager and transaction info classes:
//..
//  class MyTransactionInfo {
//      // Transaction Info...
//  };
//
//  class MyTransactionManager {
//
//      // PRIVATE MANIPULATORS
//      int enqueueTransaction(shared_ptr<MyUser>  user,
//                             const MyTransactionInfo& transaction);
//    public:
//      // CLASS METHODS
//      static MyUser *systemUser(bslma::Allocator *basicAllocator = 0);
//
//      // MANIPULATORS
//      int enqueueSystemTransaction(const MyTransactionInfo& transaction);
//
//      int enqueueUserTransaction(const MyTransactionInfo& transaction,
//                                 shared_ptr<MyUser>  user);
//
//  };
//..
// The 'systemUser' class method returns the same 'MyUser' object and should
// not be destroyed by its users:
//..
//  MyUser *MyTransactionManager::systemUser(
//                                      bslma::Allocator */* basicAllocator */)
//  {
//      static MyUser *systemUserSingleton;
//      if (!systemUserSingleton) {
//          // instantiate singleton in a thread-safe manner passing
//          // 'basicAllocator'
//
//          // . . .
//      }
//      return systemUserSingleton;
//  }
//..
// For enqueuing user transactions, simply proxy the information to
// 'enqueueTransaction'.
//..
//  inline
//  int MyTransactionManager::enqueueUserTransaction(
//                                  const MyTransactionInfo& transaction,
//                                  shared_ptr<MyUser>  user)
//  {
//      return enqueueTransaction(user, transaction);
//  }
//..
// For system transactions, we must use the 'MyUser' objected returned from the
// 'systemUser' 'static' method.  Since we do not own the returned object, we
// cannot directly construct a 'shared_ptr' object for it: doing so would
// result in the singleton being destroyed when the last reference to the
// shared pointer is released.  To solve this problem, we construct a
// 'shared_ptr' object for the system user using a nil deleter.  When the last
// reference to the shared pointer is released, although the deleter will be
// invoked to destroy the object, it will do nothing.
//..
//  int MyTransactionManager::enqueueSystemTransaction(
//                                        const MyTransactionInfo& transaction)
//  {
//      shared_ptr<MyUser> user(systemUser(),
//                                   bslstl::SharedPtrNilDeleter(),
//                                   0);
//      return enqueueTransaction(user, transaction);
//  }
//..
//
///Example 3: Basic Weak Pointer Usage
///- - - - - - - - - - - - - - - - - -
// This example illustrates the basic syntax needed to create and use a
// 'bsl::weak_ptr'.  Suppose that we want to construct a weak pointer that
// refers to an 'int' managed by a shared pointer.  Next we define the shared
// pointer and assign a value to the shared 'int':
//..
//  bsl::shared_ptr<int> intPtr;
//  intPtr.createInplace(bslma::Default::allocator());
//  *intPtr = 10;
//  assert(10 == *intPtr);
//..
// Next we construct a weak pointer to the 'int':
//..
//  bsl::weak_ptr<int> intWeakPtr(intPtr);
//  assert(!intWeakPtr.expired());
//..
// 'bsl::weak_ptr' does not provide direct access to the shared object being
// referenced.  To access and manipulate the 'int' from the weak pointer, we
// have to obtain a shared pointer from it:
//..
//  bsl::shared_ptr<int> intPtr2 = intWeakPtr.lock();
//  assert(intPtr2);
//  assert(10 == *intPtr2);
//
//  *intPtr2 = 20;
//  assert(20 == *intPtr);
//  assert(20 == *intPtr2);
//..
// We remove the weak reference to the shared 'int' by calling the 'reset'
// method:
//..
//  intWeakPtr.reset();
//  assert(intWeakPtr.expired());
//..
// Note that resetting the weak pointer does not affect the shared pointers
// referencing the 'int' object:
//..
//  assert(20 == *intPtr);
//  assert(20 == *intPtr2);
//..
// Now, we construct another weak pointer referencing the shared 'int':
//..
//  bsl::weak_ptr<int> intWeakPtr2(intPtr);
//  assert(!intWeakPtr2.expired());
//..
// Finally 'reset' all shared references to the 'int', which will cause the
// weak pointer to become "expired"; any subsequent attempt to obtain a shared
// pointer from the weak pointer will return a shared pointer in the default
// constructed (empty) state:
//..
//  intPtr.reset();
//  intPtr2.reset();
//  assert(intWeakPtr2.expired());
//  assert(!intWeakPtr2.lock());
//..
//
///Example 4: Breaking Cyclical Dependencies
///- - - - - - - - - - - - - - - - - - - - -
// Weak pointers are frequently used to break cyclical dependencies between
// objects that store references to each other via a shared pointer.  Consider
// for example a simplified news alert system that sends news alerts to users
// based on keywords that they register for.  The user information is stored in
// the 'User' class and the details of the news alert are stored in the 'Alert'
// class.  The class definitions for 'User' and 'Alert' are provided below
// (with any code not relevant to this example elided):
//..
//  class Alert;
//
//  class User {
//      // This class stores the user information required for listening to
//      // alerts.
//
//      bsl::vector<bsl::shared_ptr<Alert> > d_alerts;  // alerts user is
//                                                      // registered for
//
//      // ...
//
//    public:
//      // MANIPULATORS
//      void addAlert(const bsl::shared_ptr<Alert>& alertPtr)
//      {
//          // Add the specified 'alertPtr' to the list of alerts being
//          // monitored by this user.
//
//          d_alerts.push_back(alertPtr);
//      }
//
//      // ...
//  };
//..
// Now we define an alert class, 'Alert':
//..
//  class Alert {
//      // This class stores the alert information required for sending
//      // alerts.
//
//      bsl::vector<bsl::shared_ptr<User> > d_users;  // users registered
//                                                    // for this alert
//
//    public:
//      // MANIPULATORS
//      void addUser(const bsl::shared_ptr<User>& userPtr)
//      {
//          // Add the specified 'userPtr' to the list of users monitoring this
//          // alert.
//
//          d_users.push_back(userPtr);
//      }
//
//      // ...
//  };
//
//..
// Even though we have released 'alertPtr' and 'userPtr' there still exists a
/// cyclic reference between the two objects, so none of the objects are
// destroyed.
//
// We can break this cyclical dependency we define a modified alert class
// 'ModifiedAlert' that stores a weak pointer to a 'ModifiedUser' object.
// Below is the definition for the 'ModifiedUser' class that is identical to
// the 'User' class, the only difference being that it stores shared pointer to
// 'ModifiedAlert's instead of 'Alert's:
//..
//  class ModifiedAlert;
//
//  class ModifiedUser {
//      // This class stores the user information required for listening to
//      // alerts.
//
//      bsl::vector<bsl::shared_ptr<ModifiedAlert> > d_alerts;// alerts user is
//                                                            // registered for
//
//      // ...
//
//    public:
//      // MANIPULATORS
//      void addAlert(const bsl::shared_ptr<ModifiedAlert>& alertPtr)
//      {
//          // Add the specified 'alertPtr' to the list of alerts being
//          // monitored by this user.
//
//          d_alerts.push_back(alertPtr);
//      }
//
//      // ...
//  };
//..
// Now we define the 'ModifiedAlert' class:
//..
//  class ModifiedAlert {
//      // This class stores the alert information required for sending
//      // alerts.
//
//..
// Note that the user is stored by a weak pointer instead of by a shared
// pointer:
//..
//      bsl::vector<bsl::weak_ptr<ModifiedUser> > d_users;  // users registered
//                                                          // for this alert
//
//    public:
//      // MANIPULATORS
//      void addUser(const bsl::weak_ptr<ModifiedUser>& userPtr)
//      {
//          // Add the specified 'userPtr' to the list of users monitoring this
//          // alert.
//
//          d_users.push_back(userPtr);
//      }
//
//      // ...
//  };
//..
//
///Example 5: Caching
/// - - - - - - - - -
// Suppose we want to implement a peer to peer file sharing system that allows
// users to search for files that match specific keywords.  A simplistic
// version of such a system with code not relevant to the usage example elided
// would have the following parts:
//
// a) A peer manager class that maintains a list of all connected peers and
// updates the list based on incoming peer requests and disconnecting peers.
// The following would be a simple interface for the Peer and PeerManager
// classes:
//..
//  class Peer {
//      // This class stores all the relevant information for a peer.
//
//      // ...
//  };
//
//  class PeerManager {
//      // This class acts as a manager of peers and adds and removes peers
//      // based on peer requests and disconnections.
//
//      // DATA
//..
// The peer objects are stored by shared pointer to allow peers to be passed to
// search results and still allow their asynchronous destruction when peers
// disconnect.
//..
//      bsl::map<int, bsl::shared_ptr<Peer> > d_peers;
//
//      // ...
//  };
//..
// b) A peer cache class that stores a subset of the peers that are used for
// sending search requests.  The cache may select peers based on their
// connection bandwidth, relevancy of previous search results, etc.  For
// brevity the population and flushing of this cache is not shown:
//..
//  class PeerCache {
//      // This class caches a subset of all peers that match certain criteria
//      // including connection bandwidth, relevancy of previous search
//      // results, etc.
//
//..
// Note that the cached peers are stored as a weak pointer so as not to
// interfere with the cleanup of Peer objects by the PeerManager if a Peer goes
// down.
//..
//      // DATA
//      bsl::list<bsl::weak_ptr<Peer> > d_cachedPeers;
//
//    public:
//      // TYPES
//      typedef bsl::list<bsl::weak_ptr<Peer> >::const_iterator PeerConstIter;
//
//      // ...
//
//      // ACCESSORS
//      PeerConstIter begin() const { return d_cachedPeers.begin(); }
//      PeerConstIter end() const   { return d_cachedPeers.end(); }
//  };
//..
// c) A search result class that stores a search result and encapsulates a peer
// with the file name stored by the peer that best matches the specified
// keywords:
//..
//  class SearchResult {
//      // This class provides a search result and encapsulates a particular
//      // peer and filename combination that matches a specified set of
//      // keywords.
//
//..
// The peer is stored as a weak pointer because when the user decides to select
// a particular file to download from this peer, the peer might have
// disconnected.
//..
//      // DATA
//      bsl::weak_ptr<Peer> d_peer;
//      bsl::string         d_filename;
//
//    public:
//      // CREATORS
//      SearchResult(const bsl::weak_ptr<Peer>& peer,
//                   const bsl::string&         filename)
//      : d_peer(peer)
//      , d_filename(filename)
//      {
//      }
//
//      // ...
//
//      // ACCESSORS
//      const bsl::weak_ptr<Peer>& peer() const { return d_peer; }
//      const bsl::string& filename() const { return d_filename; }
//  };
//..
// d) A search function that takes a list of keywords and returns available
// results by searching the cached peers:
//..
//  ;
//
//  void search(bsl::vector<SearchResult>       */* results */,
//              const PeerCache&                 peerCache,
//              const bsl::vector<bsl::string>&  /* keywords */)
//  {
//      for (PeerCache::PeerConstIter iter = peerCache.begin();
//           iter != peerCache.end();
//           ++iter) {
//..
// First we check if the peer is still connected by acquiring a shared pointer
// to the peer.  If the acquire operation succeeds, then we can send the peer a
// request to send back the file best matching the specified keywords:
//..
//          bsl::shared_ptr<Peer> peerSharedPtr = iter->lock();
//          if (peerSharedPtr) {
//
//              // Search the peer for file best matching the specified
//              // keywords and if a file is found add the returned
//              // SearchResult object to result.
//
//              // ...
//          }
//      }
//  }
//..
// e) A download function that downloads a file selected by the user:
//..
//  void download(const SearchResult& result)
//  {
//      bsl::shared_ptr<Peer> peerSharedPtr = result.peer().lock();
//      if (peerSharedPtr) {
//          // Download the result.filename() file from peer knowing that
//          // the peer is still connected.
//      }
//  }
//..

// Prevent 'bslstl' headers from being included directly in 'BSL_OVERRIDES_STD'
// mode.  Doing so is unsupported, and is likely to cause compilation errors.
#if defined(BSL_OVERRIDES_STD) && !defined(BSL_STDHDRS_PROLOGUE_IN_EFFECT)
#error "include <bsl_memory.h> instead of <bslstl_sharedptr.h> in \
BSL_OVERRIDES_STD mode"
#endif

#ifndef INCLUDED_BSLSCM_VERSION
#include <bslscm_version.h>
#endif

#ifndef INCLUDED_BSLSTL_HASH
#include <bslstl_hash.h>
#endif

#ifndef INCLUDED_BSLSTL_PAIR
#include <bslstl_pair.h>
#endif

#ifndef INCLUDED_BSLSTL_REFERENCEWRAPPER
#include <bslstl_referencewrapper.h>
#endif

#ifndef INCLUDED_BSLSTL_SHAREDPTRALLOCATEINPLACEREP
#include <bslstl_sharedptrallocateinplacerep.h>
#endif

#ifndef INCLUDED_BSLSTL_SHAREDPTRALLOCATEOUTOFPLACEREP
#include <bslstl_sharedptrallocateoutofplacerep.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATOR
#include <bslma_allocator.h>
#endif

#ifndef INCLUDED_BSLMA_ALLOCATORTRAITS
#include <bslma_allocatortraits.h>
#endif

#ifndef INCLUDED_BSLMA_DEFAULT
#include <bslma_default.h>
#endif

#ifndef INCLUDED_BSLMA_MANAGEDPTR
#include <bslma_managedptr.h>
#endif

#ifndef INCLUDED_BSLMA_SHAREDPTRINPLACEREP
#include <bslma_sharedptrinplacerep.h>
#endif

#ifndef INCLUDED_BSLMA_SHAREDPTROUTOFPLACEREP
#include <bslma_sharedptroutofplacerep.h>
#endif

#ifndef INCLUDED_BSLMA_SHAREDPTRREP
#include <bslma_sharedptrrep.h>
#endif

#ifndef INCLUDED_BSLMA_STDALLOCATOR
#include <bslma_stdallocator.h>
#endif

#ifndef INCLUDED_BSLMF_ADDLVALUEREFERENCE
#include <bslmf_addlvaluereference.h>
#endif

#ifndef INCLUDED_BSLMF_ADDPOINTER
#include <bslmf_addpointer.h>
#endif

#ifndef INCLUDED_BSLMF_CONDITIONAL
#include <bslmf_conditional.h>
#endif

#ifndef INCLUDED_BSLMF_ENABLEIF
#include <bslmf_enableif.h>
#endif

#ifndef INCLUDED_BSLMF_HASPOINTERSEMANTICS
#include <bslmf_haspointersemantics.h>
#endif

#ifndef INCLUDED_BSLMF_INTEGRALCONSTANT
#include <bslmf_integralconstant.h>
#endif

#ifndef INCLUDED_BSLMF_ISBITWISEMOVEABLE
#include <bslmf_isbitwisemoveable.h>
#endif

#ifndef INCLUDED_BSLMF_ISCONVERTIBLE
#include <bslmf_isconvertible.h>
#endif

#ifndef INCLUDED_BSLMF_ISFUNCTION
#include <bslmf_isfunction.h>
#endif

#ifndef INCLUDED_BSLMF_ISPOINTER
#include <bslmf_ispointer.h>
#endif

#ifndef INCLUDED_BSLMF_MOVABLEREF
#include <bslmf_movableref.h>
#endif

#ifndef INCLUDED_BSLMF_UTIL
#include <bslmf_util.h>
#endif

#ifndef INCLUDED_BSLS_ASSERT
#include <bsls_assert.h>
#endif

#ifndef INCLUDED_BSLS_COMPILERFEATURES
#include <bsls_compilerfeatures.h>
#endif

#ifndef INCLUDED_BSLS_CPP11
#include <bsls_cpp11.h>
#endif

#ifndef INCLUDED_BSLS_LIBRARYFEATURES
#include <bsls_libraryfeatures.h>
#endif

#ifndef INCLUDED_BSLS_NATIVESTD
#include <bsls_nativestd.h>
#endif

#ifndef INCLUDED_BSLS_NULLPTR
#include <bsls_nullptr.h>
#endif

#ifndef INCLUDED_BSLS_PLATFORM
#include <bsls_platform.h>
#endif

#ifndef INCLUDED_BSLS_UNSPECIFIEDBOOL
#include <bsls_unspecifiedbool.h>
#endif

#ifndef INCLUDED_BSLS_UTIL
#include <bsls_util.h>
#endif

#ifndef INCLUDED_FUNCTIONAL
#include <functional>           // use 'std::less' to order pointers
#define INCLUDED_FUNCTIONAL
#endif

#ifndef INCLUDED_MEMORY
#include <memory>               // 'std::auto_ptr', 'std::unique_ptr'
#define INCLUDED_MEMORY
#endif

#ifndef INCLUDED_OSTREAM
#include <ostream>              // 'std::basic_ostream'
#define INCLUDED_OSTREAM
#endif

#ifndef INCLUDED_STDDEF_H
#include <stddef.h>             // 'size_t', 'ptrdiff_t'
#define INCLUDED_STDDEF_H
#endif

#if defined(BSLS_PLATFORM_CMP_GNU) || defined(BSLS_PLATFORM_CMP_CLANG)
        // Here and throughout the file wherever 'auto_ptr' is used, suspend
        // GCC reporting of deprecated declarations since the use of 'auto_ptr'
        // in this standard interface is required.
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(BSLS_COMPILERFEATURES_SUPPORT_DEFAULT_TEMPLATE_ARGS) \
 && (!defined(BSLS_PLATFORM_CMP_MSVC) || BSLS_PLATFORM_CMP_VERSION >= 1900)
# define BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS 1
// If the macro 'BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS' is defined, then a
// conforming C++11 compiler will define the constructors in this component in
// such a way that they will not be selected during overload resolution unless
// they would instantiate correctly.  This means that code depending on the
// result of 'is_constructible' and similar traits will have the expected
// behavior.  There is no attempt to support this feature in C++03.
//
// Support for SFINAE-queries on the constructability of a 'shared_ptr' depend
// on a variety of C++11 language features, including "expression-SFINAE".
// However, the main language feature that enables SFINAE elimination of a
// constructor is the ability to use default template arguments in a function
// template.  It is significantly preferred to use the template parameter list,
// rather than add additional default arguments to the constructor signatures,
// as there are so many constructor overloads in this component that there is a
// real risk of introducing ambiguities that would need to be worked around.
// Therefore, the 'BSLS_COMPILERFEATURES_SUPPORT_DEFAULT_TEMPLATE_ARGS' macro
// serves as our proxy for whether SFINAE-constructors are enabled in this
// component.  Note that the MSVC 2015 compiler almost supported
// "expression-SFINAE", to the extent that it works for this component, unlike
// earlier versions of that compiler.  We therefore make a special version-test
// on Microsoft in addition to the feature testing.
#endif

# if defined(BSLS_PLATFORM_CMP_GNU) || defined(BSLS_PLATFORM_CMP_IBM)
# define BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER 1
// If the macro 'BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER' is
// defined, we recognize that some compilers need an extra hint to disambiguate
// overload resolution when passed a 'bslma::Allocator *' pointer, that might
// also deduce (incorrectly) as a C++11-style allocator.  Compilers known to
// have this problem are gcc and IBM xlC, which were tested as recently as gcc
// 4.9.2, and xlC 12.2.  Both of these compilers have a problem partially
// ordering function templates that differ only by the first argument deducing
// as any object type ('T'), or deducting to the a pointer to something ('T*'),
// The rules for partial ordering should make the second overload a stronger
// match when passed a pointer; however, both of these compilers complain about
// ambiguities when additional parameters are invovled.
#endif

#if defined(BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS)
// Note the intentional comma in the first line of the definition of each
// macro, which allows these macros to be applied incrementally, even when the
// alternate definition is empty.  This avoids the problem of introducing new
// template parameters along with the macros in the non-SFINAE-supporting case
// below.
# define BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE ,                             \
    typename enable_if<                                                       \
        bsl::is_convertible<COMPATIBLE_TYPE *,                                \
                            ELEMENT_TYPE    *>::value>::type * = nullptr

# define BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE ,                              \
    typename enable_if<                                                       \
        bsl::is_convertible<COMPATIBLE_TYPE *,                                \
                            ELEMENT_TYPE    *>::value>::type *


# define BSLSTL_SHAREDPTR_DECLARE_IF_DELETER(FUNCTOR, ARGUMENT) ,             \
    typename enable_if<                                                       \
      BloombergLP::bslstl::SharedPtr_IsCallable  <FUNCTOR,                    \
                                                  ARGUMENT *>::k_VALUE ||     \
      BloombergLP::bslstl::SharedPtr_IsFactoryFor<FUNCTOR,                    \
                                                  ARGUMENT>::k_VALUE>::type * \
                                                                      = nullptr

# define BSLSTL_SHAREDPTR_DEFINE_IF_DELETER(FUNCTOR, ARGUMENT) ,              \
    typename enable_if<                                                       \
      BloombergLP::bslstl::SharedPtr_IsCallable  <FUNCTOR,                    \
                                                  ARGUMENT *>::k_VALUE ||     \
      BloombergLP::bslstl::SharedPtr_IsFactoryFor<FUNCTOR,                    \
                                                  ARGUMENT  >::k_VALUE>::type *


# define BSLSTL_SHAREDPTR_DECLARE_IF_NULLPTR_DELETER(FUNCTOR) ,               \
    typename enable_if<                                                       \
      BloombergLP::bslstl::SharedPtr_IsCallable  <FUNCTOR,                    \
                                                  nullptr_t>::k_VALUE ||      \
      BloombergLP::bslstl::SharedPtr_IsNullableFactory<                       \
                                           FUNCTOR>::k_VALUE>::type * = nullptr

# define BSLSTL_SHAREDPTR_DEFINE_IF_NULLPTR_DELETER(FUNCTOR) ,                \
    typename enable_if<                                                       \
      BloombergLP::bslstl::SharedPtr_IsCallable  <FUNCTOR,                    \
                                                  nullptr_t>::k_VALUE ||      \
      BloombergLP::bslstl::SharedPtr_IsNullableFactory<                       \
                                                     FUNCTOR>::k_VALUE>::type *
#else
// Do not attempt to support SFINAE in constructors in a C++03 compiler
# define BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE
# define BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE

# define BSLSTL_SHAREDPTR_DECLARE_IF_DELETER(FUNCTOR, ARGUMENT)
# define BSLSTL_SHAREDPTR_DEFINE_IF_DELETER(FUNCTOR, ARGUMENT)

# define BSLSTL_SHAREDPTR_DECLARE_IF_NULLPTR_DELETER(FUNCTOR)
# define BSLSTL_SHAREDPTR_DEFINE_IF_NULLPTR_DELETER(FUNCTOR)
#endif  // BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS


namespace BloombergLP {
namespace bslstl {

struct SharedPtr_RepFromExistingSharedPtr {
    // This 'struct' is for internal use only, providing a tag for 'shared_ptr'
    // constructors to recognize that a passed 'SharedPtrRep' was obtained from
    // an existing 'shared_ptr' object.
};

struct SharedPtr_ImpUtil;
    // Forward declaration of 'SharedPtr_ImpUtil'. This is needed because this
    // struct is a friend of 'enable_shared_from_this' in the 'bsl' namespace.

#if defined(BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS)
template <class FUNCTOR, class ARG>
struct SharedPtr_IsCallable;
    // Forward declaration of component-private type trait to indicate whether
    // an object of (template parameter) type 'FUNCTOR' can be called as a
    // function with an argument of (temnplate parameter) type 'ARG'

template <class FACTORY, class ARG>
struct SharedPtr_IsFactoryFor;
    // Forward declaration of component-private type trait to indicate whether
    // a pointer to a 'FACTORY' has a 'deleteObject' member that can be called
    // as 'factory->deleteObject((ARG *)p)'.

template <class FACTORY>
struct SharedPtr_IsNullableFactory;
    // Forward declaration of component-private type trait to indicate whether
    // a pointer to a 'FACTORY' has a 'deleteObject' member that can be called
    // as 'factory->deleteObject((ARG *)p)'.
#endif

}  // close package namespace
}  // close enterprise namespace

namespace bsl {

template<class ELEMENT_TYPE>
class enable_shared_from_this;

template <class ELEMENT_TYPE>
class shared_ptr;

template <class ELEMENT_TYPE>
class weak_ptr;

                        // ================
                        // class shared_ptr
                        // ================

template <class ELEMENT_TYPE>
class shared_ptr {
    // This class provides a thread-safe reference-counted "smart pointer" to
    // support "shared ownership" of objects: a shared pointer ensures that the
    // shared object is destroyed, using the appropriate deletion method, only
    // when there are no shared references to it.  The object (of template
    // parameter type 'ELEMENT_TYPE') referred to by a shared pointer may be
    // accessed directly using the '->' operator, or the dereference operator
    // (operator '*') can be used to obtain a reference to that object.
    //
    // Note that the object referred to by a shared pointer representation is
    // usually the same as the object referred to by that shared pointer (of
    // the same 'ELEMENT_TYPE'), but this need not always be true in the
    // presence of conversions or "aliasing": the object referred to (of
    // template parameter type 'ELEMENT_TYPE') by the shared pointer may differ
    // from the object of type 'COMPATIBLE_TYPE' (see the "Aliasing" section in
    // the component-level documentation) referred to by the shared pointer
    // representation.
    //
    // More generally, this class supports a complete set of *in*-*core*
    // pointer semantic operations.

    // DATA
    ELEMENT_TYPE                     *d_ptr_p; // pointer to the shared object

    BloombergLP::bslma::SharedPtrRep *d_rep_p; // pointer to the representation
                                               // object that manages the
                                               // shared object

    // PRIVATE TYPES
    typedef shared_ptr<ELEMENT_TYPE> SelfType;
        // 'SelfType' is an alias to this 'class', for compilers that do not
        // recognize plain 'shared_ptr'.

    typedef typename BloombergLP::bsls::UnspecifiedBool<shared_ptr>::BoolType
                                                                      BoolType;

    // FRIENDS
    template <class COMPATIBLE_TYPE>
    friend class shared_ptr;

    friend struct BloombergLP::bslstl::SharedPtr_ImpUtil;

  private:
    // PRIVATE CLASS METHODS
    template <class INPLACE_REP>
    static BloombergLP::bslma::SharedPtrRep *makeInternalRep(
                                        ELEMENT_TYPE                     *,
                                        INPLACE_REP                      *,
                                        BloombergLP::bslma::SharedPtrRep *rep);
        // Return the specified 'rep'.

    template <class COMPATIBLE_TYPE, class ALLOCATOR>
    static BloombergLP::bslma::SharedPtrRep *makeInternalRep(
                                     COMPATIBLE_TYPE               *ptr,
                                     ALLOCATOR                     *,
                                     BloombergLP::bslma::Allocator *allocator);
        // Return the address of a new out-of-place representation for a shared
        // pointer that manages the specified 'ptr' and uses the specified
        // 'allocator' to destroy the object pointed to by 'ptr'.  Use
        // 'allocator' to supply memory.

    template <class COMPATIBLE_TYPE, class DELETER>
    static BloombergLP::bslma::SharedPtrRep *makeInternalRep(
                                                      COMPATIBLE_TYPE *ptr,
                                                      DELETER         *deleter,
                                                      ...);
        // Return the address of a new out-of-place representation for a shared
        // pointer that manages the specified 'ptr' and uses the specified
        // 'deleter' to destroy the object pointed to by 'ptr'.  Use the
        // currently installed default allocator to supply memory.

  public:
    // TYPES
    typedef ELEMENT_TYPE element_type;
        // 'element_type' is an alias to the 'ELEMENT_TYPE' template parameter.
        // Note that 'element_type' refers to the same type as 'ElementType'.

    typedef ELEMENT_TYPE ElementType;
        // 'ElementType' is an alias to the 'ELEMENT_TYPE' template parameter.

    typedef weak_ptr<ELEMENT_TYPE> weak_type;
        // 'weak_type' is an alias to a weak pointer with the same element type
        // as this 'shared_ptr'.

    // CREATORS
    BSLS_CPP11_CONSTEXPR
    shared_ptr() BSLS_CPP11_NOEXCEPT;
        // Create an empty shared pointer, i.e., a shared pointer with no
        // representation that does not refer to any object and has no
        // deleter.

    BSLS_CPP11_CONSTEXPR
    shared_ptr(bsl::nullptr_t) BSLS_CPP11_NOEXCEPT;                 // IMPLICIT
        // Create an empty shared pointer, i.e., a shared pointer with no
        // representation that does not refer to any object and has no
        // deleter.


    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    explicit shared_ptr(COMPATIBLE_TYPE *ptr);
        // Create a shared pointer that manages a modifiable object of
        // (template parameter) type 'COMPATIBLE_TYPE' and refers to the
        // specified '(ELEMENT_TYPE *)ptr'.  The currently installed default
        // allocator is used to allocate and deallocate the internal
        // representation of the shared pointer.  When all references have been
        // released, the object pointed to by the managed pointer will be
        // destroyed by a call to 'delete ptr'.  If 'COMPATIBLE_TYPE *' is not
        // implicitly convertible to 'ELEMENT_TYPE *', then a compiler
        // diagnostic will be emitted indicating the error.  If 'ptr' is 0,
        // then this shared pointer will still allocate an internal
        // representation to share ownership of that empty state, which will be
        // reclaimed when the last reference is destroyed.  If an exception is
        // thrown allocating storage for the representation, then 'delete ptr'
        // will be called.  Note that if 'ptr' is a null-pointer constant, the
        // compiler will actually select the 'shared_ptr(bsl::nullptr_t)'
        // constructor, resulting in an empty shared pointer.

    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    shared_ptr(COMPATIBLE_TYPE               *ptr,
               BloombergLP::bslma::Allocator *basicAllocator);
        // Create a shared pointer that manages a modifiable object of
        // (template parameter) type 'COMPATIBLE_TYPE' and refers to the
        // specified 'ptr' cast to a pointer to the (template parameter) type
        // 'ELEMENT_TYPE'.  If the specified 'basicAllocator' is not 0, then
        // 'basicAllocator' is used to allocate and deallocate the internal
        // representation of the shared pointer and to destroy the shared
        // object when all references have been released; otherwise, the
        // currently installed default allocator is used.  If
        // 'COMPATIBLE_TYPE *' is not implicitly convertible to
        // 'ELEMENT_TYPE *', then a compiler diagnostic will be emitted
        // indicating the error.  If 'ptr' is 0, then this shared pointer will
        // still allocate an internal representation to share ownership of that
        // empty state, which will be reclaimed when the last reference is
        // destroyed.  Note that if 'ptr' is a null-pointer constant, the
        // compiler will actually select the
        // 'shared_ptr(bsl::nullptr_t, BloombergLP::bslma::Allocator *)'
        // constructor, resulting in an empty shared pointer.  Note that if
        // 'basicAllocator' is a pointer to a class derived from
        // 'bslma::Allocator', the compiler will actually select the following
        // (more general) constructor that has the same behavior:
        //..
        //  template <class COMPATIBLE_TYPE, class DELETER>
        //  shared_ptr(COMPATIBLE_TYPE *ptr, DELETER * deleter);
        //..

    shared_ptr(ELEMENT_TYPE *ptr, BloombergLP::bslma::SharedPtrRep *rep);
        // Create a shared pointer that takes ownership of the specified 'rep'
        // and refers to the modifiable object at the specified 'ptr' address.
        // The number of references to 'rep' is *NOT* incremented.  Note that
        // if 'rep' is a pointer to a class derived from
        // 'BloombergLP::bslma::SharedPtrRep', the compiler will actually
        // select the following (more general) constructor that has the same
        // behavior:
        //..
        //  template <class COMPATIBLE_TYPE, class DELETER>
        //  shared_ptr(COMPATIBLE_TYPE *ptr, DELETER * deleter);
        //..

    shared_ptr(ELEMENT_TYPE                     *ptr,
               BloombergLP::bslma::SharedPtrRep *rep,
               BloombergLP::bslstl::SharedPtr_RepFromExistingSharedPtr);
        // Create a shared pointer that takes ownership of the specified 'rep'
        // and refers to the modifiable object at the specified 'ptr' address.
        // The number of references to 'rep' is *NOT* incremented.  The
        // behavior is undefined unless 'rep' was previously obtained from an
        // existing 'shared_ptr', 'rep->disposeObject' has not been called, and
        // 'rep->numReferences() > 0'.  Note that this constructor is intended
        // for use by 'weak_ptr::lock', and it would be surprising to find
        // another client.  This solves an obscure problem that arises from
        // unusual use of classes derived from 'enable_shared_from_this'.
        // Further note that the caller is responsible for incrementing the
        // 'numReferences' count prior to calling this constructor, in order to
        // maintain a consistent reference count when this 'shared_ptr' object
        // releases the shared object from its management.

    template <class COMPATIBLE_TYPE, class DELETER
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE
              BSLSTL_SHAREDPTR_DECLARE_IF_DELETER(DELETER *, COMPATIBLE_TYPE)>
    shared_ptr(COMPATIBLE_TYPE *ptr, DELETER *deleter);
        // Create a shared pointer that manages a modifiable object of
        // (template parameter) type 'COMPATIBLE_TYPE', refers to the specified
        // 'ptr' cast to a pointer to the (template parameter) type
        // 'ELEMENT_TYPE', and uses the specified 'deleter' to delete the
        // shared object when all references have been released.  Use the
        // currently installed default allocator to allocate and deallocate the
        // internal representation of the shared pointer, unless 'DELETER' is a
        // class derived from either 'bslma::Allocator' or
        // 'bslma::SharedPtrRep'; if 'DELETER' is a class derived from
        // 'bslma::allocator', create a shared pointer as if calling the
        // constructor:
        //..
        //  template <class COMPATIBLE_TYPE>
        //  shared_ptr(COMPATIBLE_TYPE               *ptr,
        //             BloombergLP::bslma::Allocator *basicAllocator);
        //..
        // If 'DELETER' is a class derived from 'bslma::SharedPtrRep', create a
        // shared pointer as if calling the constructor:
        //..
        //  shared_ptr(ELEMENT_TYPE                     *ptr,
        //             BloombergLP::bslma::SharedPtrRep *rep);
        //..
        // If 'DELETER' does not derive from either 'bslma::Allocator' or
        // 'BloombergLP::bslma::SharedPtrRep', then 'deleter' shall be a
        // pointer to a factory object that exposes a member function that can
        // be invoked as 'deleteObject(ptr)' that will be called to destroy the
        // object at the 'ptr' address (i.e., 'deleter->deleteObject(ptr)' will
        // be called to delete the shared object).  (See the "Deleters" section
        // in the component-level documentation.)  If 'COMPATIBLE_TYPE *' is
        // not implicitly convertible to 'ELEMENT_TYPE *', then a compiler
        // diagnostic will be emitted indicating the error.  If 'ptr' is 0,
        // then the null pointer will be reference counted, and the deleter
        // will be called when the last reference is destroyed.  If an
        // exception is thrown when allocating storage for the internal
        // representation, then 'deleter(ptr)' will be called.  Note that this
        // method is a BDE extension and not part of the C++ standard
        // interface.

    template <class COMPATIBLE_TYPE, class DELETER
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE
              BSLSTL_SHAREDPTR_DECLARE_IF_DELETER(DELETER, COMPATIBLE_TYPE)>
    shared_ptr(COMPATIBLE_TYPE               *ptr,
               DELETER                        deleter,
               BloombergLP::bslma::Allocator *basicAllocator = 0);
        // Create a shared pointer that manages a modifiable object of
        // (template parameter) type 'COMPATIBLE_TYPE', refers to the specified
        // '(ELEMENT_TYPE *)ptr', and uses the specified 'deleter' to delete
        // the shared object when all references have been released.
        // Optionally specify a 'basicAllocator' to allocate and deallocate the
        // internal representation of the shared pointer (including a copy of
        // 'deleter').  If 'basicAllocator' is 0, the currently installed
        // default allocator is used.  'DELETER' shall be either a function
        // pointer or a "factory" deleter that may be invoked to destroy the
        // object referred to by a single argument of type 'COMPATIBLE_TYPE *'
        // (i.e., 'deleter(ptr)' or 'deleter->deleteObject(ptr)' will be called
        // to destroy the shared object).  (See the "Deleters" section in the
        // component-level documentation.)  If 'COMPATIBLE_TYPE *' is not
        // implicitly convertible to 'ELEMENT_TYPE *', then this constructor
        // will not be selected by overload resolution.  If 'ptr' is 0, then
        // the null pointer will be reference counted, and 'deleter(ptr)' will
        // be called when the last reference is destroyed.  If an exception is
        // thrown when allocating storage for the internal representation, then
        // 'deleter(ptr)' will be called.  The behavior is undefined unless the
        // constructor making a copy of 'deleter' does not throw an exception.

    template <class COMPATIBLE_TYPE, class DELETER, class ALLOCATOR
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE
              BSLSTL_SHAREDPTR_DECLARE_IF_DELETER(DELETER, COMPATIBLE_TYPE)>
    shared_ptr(COMPATIBLE_TYPE                *ptr,
               DELETER                         deleter,
               ALLOCATOR                       basicAllocator,
               typename ALLOCATOR::value_type * = 0);
        // Create a shared pointer that manages a modifiable object of
        // (template parameter) type 'COMPATIBLE_TYPE', refers to the specified
        // 'ptr' cast to a pointer to the (template parameter) type
        // 'ELEMENT_TYPE', and uses the specified 'deleter' to delete the
        // shared object when all references have been released.  Use the
        // specified 'basicAllocator' to allocate and deallocate the internal
        // representation of the shared pointer (including a copy of the
        // 'deleter').  The (template parameter) type 'DELETER' shall be either
        // a function pointer or a function-like deleter that may be invoked to
        // destroy the object referred to by a single argument of type
        // 'COMPATIBLE_TYPE *' (i.e., 'deleter(ptr)' will be called to destroy
        // the shared object).  (See the "Deleters" section in the component-
        // level documentation.)  The (template parameter) type 'ALLOCATOR'
        // shall satisfy the Allocator requirements of the C++ standard (C++11
        // 17.6.3.5, [allocator.requirements]).  If 'COMPATIBLE_TYPE *' is not
        // implicitly convertible to 'ELEMENT_TYPE *', then a compiler
        // diagnostic will be emitted indicating the error.  If 'ptr' is 0,
        // then the null pointer will be reference counted, and 'deleter(ptr)'
        // will be called when the last reference is destroyed.  If an
        // exception is thrown when allocating storage for the internal
        // representation, then 'deleter(ptr)' will be called.  The behavior is
        // undefined unless the constructor making a copy of 'deleter' does not
        // throw an exception.  Note that the final dummy parameter is a simple
        // SFINAE check that the (template parameter) 'ALLOCATOR' type probably
        // satisfies the standard allocator requirements; in particular, it
        // will not match pointer types, so any pointers to 'bslma::Allocator'
        // derived classes will dispatch to the constructor above this, and not
        // be greedily matched to a generic type parameter.

    shared_ptr(nullptr_t                      nullPointerLiteral,
               BloombergLP::bslma::Allocator *basicAllocator);
        // Create an empty shared pointer.  The specified 'nullPointerLiteral'
        // and 'basicAllocator' are not used.  Note that use of this
        // constructor is equivalent to calling the default constructor.

    template <class DELETER
              BSLSTL_SHAREDPTR_DECLARE_IF_NULLPTR_DELETER(DELETER)>
    shared_ptr(nullptr_t                      nullPointerLiteral,
               DELETER                        deleter,
               BloombergLP::bslma::Allocator *basicAllocator = 0);
        // Create a shared pointer that reference-counts the null pointer, and
        // calls the specified 'deleter' with a null pointer (i.e., invokes
        // 'deleter((ELEMENT_TYPE *)0)') when the last shared reference is
        // destroyed.  The specified 'nullPointerLiteral' is not used.
        // Optionally specify a 'basicAllocator' to allocate and deallocate the
        // internal representation of the shared pointer (including a copy of
        // 'deleter').  If 'basicAllocator' is 0, the currently installed
        // default allocator is used.  If an exception is thrown when
        // allocating storage for the internal representation, then
        // 'deleter((ELEMENT_TYPE *)0)' will be called.  The behavior is
        // undefined unless 'deleter' can be called with a null pointer, and
        // unless the constructor making a copy of 'deleter' does not throw an
        // exception.

    template <class DELETER, class ALLOCATOR
              BSLSTL_SHAREDPTR_DECLARE_IF_NULLPTR_DELETER(DELETER)>
    shared_ptr(nullptr_t                       nullPointerLiteral,
               DELETER                         deleter,
               ALLOCATOR                       basicAllocator,
               typename ALLOCATOR::value_type * = 0);
        // Create a shared pointer that reference-counts the null pointer,
        // calls the specified 'deleter' with a null pointer (i.e., invokes
        // 'deleter((ELEMENT_TYPE *)0)') when the last shared reference is
        // destroyed, and uses the specified 'basicAllocator' to allocate and
        // deallocate the internal representation of the shared pointer
        // (including a copy of the 'deleter').  The (template parameter) type
        // 'DELETER' shall be either a function pointer or a function-like
        // deleter (See the "Deleters" section in the component- level
        // documentation).  The (template parameter) type 'ALLOCATOR' shall
        // satisfy the Allocator requirements of the C++ standard (C++11
        // 17.6.3.5, [allocator.requirements]).  The specified
        // 'nullPointerLiteral' is not used.  If an exception is thrown when
        // allocating storage for the internal representation, then
        // 'deleter((ELEMENT_TYPE *)0)' will be called.  The behavior is
        // undefined unless 'deleter' can be called with a null pointer, and
        // unless the constructor making a copy of 'deleter' does not throw an
        // exception.  Note that the final dummy parameter is a simple SFINAE
        // check that the 'ALLOCATOR' type probably satisfies the standard
        // allocator requirements; in particular, it will not match pointer
        // types, so any pointers to 'bslma::Allocator' derived classes will
        // dispatch to the constructor above this, and not be greedily matched
        // to a generic type parameter.

    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    shared_ptr(
          BloombergLP::bslma::ManagedPtr<COMPATIBLE_TYPE>  managedPtr,
          BloombergLP::bslma::Allocator                   *basicAllocator = 0);
                                                                    // IMPLICIT
        // Create a shared pointer that takes over the management of the
        // modifiable object (if any) previously managed by the specified
        // 'managedPtr' to the (template parameter) type 'COMPATIBLE_TYPE', and
        // that refers to '(ELEMENT_TYPE *)managedPtr.ptr()'.  The deleter used
        // in the 'managedPtr' will be used to destroy the shared object when
        // all references have been released.  Optionally specify a
        // 'basicAllocator' used to allocate and deallocate the internal
        // representation of the shared pointer.  If 'basicAllocator' is 0, the
        // currently installed default allocator is used.  If
        // 'COMPATIBLE_TYPE *' is not implicitly convertible to
        // 'ELEMENT_TYPE *', then a compiler diagnostic will be emitted
        // indicating the error.  Note that if 'managedPtr' is empty, then an
        // empty shared pointer is created and 'basicAllocator' is ignored.
        // Also note that if 'managedPtr' owns a reference to another shared
        // object (due to a previous call to 'shared_ptr<T>::managedPtr') then
        // no memory will be allocated, and this 'shared_ptr' will adopt the
        // 'ManagedPtr's ownership of that shared object.

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP98_AUTO_PTR)
    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    explicit shared_ptr(
                   native_std::auto_ptr<COMPATIBLE_TYPE>&  autoPtr,
                   BloombergLP::bslma::Allocator          *basicAllocator = 0);
        // Create a shared pointer that takes over the management of the
        // modifiable object previously managed by the specified 'autoPtr' to
        // the (template parameter) type 'COMPATIBLE_TYPE', and that refers to
        // '(ELEMENT_TYPE *)autoPtr.get()'.  'delete(autoPtr.release())' will
        // be called to destroy the shared object when all references have been
        // released.  Optionally specify a 'basicAllocator' used to allocate
        // and deallocate the internal representation of the shared pointer.
        // If 'basicAllocator' is 0, the currently installed default allocator
        // is used.  If 'COMPATIBLE_TYPE *' is not implicitly convertible to
        // 'ELEMENT_TYPE *', then a compiler diagnostic will be emitted
        // indicating the error.

    explicit shared_ptr(
                   native_std::auto_ptr_ref<ELEMENT_TYPE>  autoRef,
                   BloombergLP::bslma::Allocator          *basicAllocator = 0);
        // Create a shared pointer that takes over the management of the
        // modifiable object of (template parameter) type 'COMPATIBLE_TYPE'
        // previously managed by the auto pointer object that the specified
        // 'autoRef' refers to; this shared pointer refers to the same object
        // that it manages, and 'delete(get())' will be called to destroy the
        // shared object when all references have been released.  Optionally
        // specify a 'basicAllocator' used to allocate and deallocate the
        // internal representation of the shared pointer.  If 'basicAllocator'
        // is 0, the currently installed default allocator is used.  This
        // function does not exist unless 'COMPATIBLE_TYPE *' is convertible to
        // 'ELEMENT_TYPE *'.
#endif

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP11_UNIQUE_PTR)
# if defined(BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS)
    template <class COMPATIBLE_TYPE,
              class UNIQUE_DELETER,
              typename enable_if<is_convertible<
                      typename native_std::unique_ptr<COMPATIBLE_TYPE,
                                                      UNIQUE_DELETER>::pointer,
                      ELEMENT_TYPE *>::value>::type * = nullptr>
    explicit shared_ptr(
                 native_std::unique_ptr<COMPATIBLE_TYPE,
                                        UNIQUE_DELETER>&&  adoptee,
                 BloombergLP::bslma::Allocator            *basicAllocator = 0);
        // Create a shared pointer that takes over the management of the
        // modifiable object previously managed by the specified 'adoptee' to
        // the (template parameter) type 'COMPATIBLE_TYPE', and that refers to
        // '(ELEMENT_TYPE *)autoPtr.get()'.  'delete(autoPtr.release())' will
        // be called to destroy the shared object when all references have been
        // released.  Optionally specify a 'basicAllocator' used to allocate
        // and deallocate the internal representation of the shared pointer.
        // If 'basicAllocator' is 0, the currently installed default allocator
        // is used.  This function does not exist unless
        // 'unqiue_ptr<COMPATIBLE_TYPE, DELETER>::pointer' is convertible to
        // 'ELEMENT_TYPE *'.  Note that this function creates a 'shared_ptr'
        // with an unspecified deleter type that has satisfies this contract,
        // which might not be the deleter of 'rhs', which is specified by the
        // C++ standard.
# else
    template <class COMPATIBLE_TYPE, class UNIQUE_DELETER>
    explicit shared_ptr(
                 native_std::unique_ptr<COMPATIBLE_TYPE,
                                        UNIQUE_DELETER>&&  adoptee,
                 BloombergLP::bslma::Allocator            *basicAllocator = 0,
                 typename enable_if<is_convertible<
                      typename
                          native_std::unique_ptr<COMPATIBLE_TYPE,
                                                      UNIQUE_DELETER>::pointer,
                          ELEMENT_TYPE *>::value,
                      BloombergLP::bslstl::SharedPtr_ImpUtil>::type =
                                      BloombergLP::bslstl::SharedPtr_ImpUtil())
        // Create a shared pointer that takes over the management of the
        // modifiable object previously managed by the specified 'adoptee' to
        // the (template parameter) type 'COMPATIBLE_TYPE', and that refers to
        // '(ELEMENT_TYPE *)autoPtr.get()'.  'delete(autoPtr.release())' will
        // be called to destroy the shared object when all references have been
        // released.  Optionally specify a 'basicAllocator' used to allocate
        // and deallocate the internal representation of the shared pointer.
        // If 'basicAllocator' is 0, the currently installed default allocator
        // is used.  This function does not exist unless
        // 'unqiue_ptr<COMPATIBLE_TYPE, DELETER>::pointer' is convertible to
        // 'ELEMENT_TYPE *'.  Note that this function creates a 'shared_ptr'
        // with an unspecified deleter type that has satisfies this contract,
        // which might not be the deleter of 'rhs', which is specified by the
        // C++ standard.
    : d_ptr_p(adoptee.get())
    , d_rep_p(0)
    {
        // This constructor template must be defined inline inside the class
        // defintion, as Microsoft Visual C++ does not recognise the definition
        // as matching this signature when placed out-of-line.

        typedef BloombergLP::bslma::SharedPtrInplaceRep<
                 native_std::unique_ptr<COMPATIBLE_TYPE, UNIQUE_DELETER> > Rep;

        if (d_ptr_p) {
            basicAllocator =
                        BloombergLP::bslma::Default::allocator(basicAllocator);
            Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                            BloombergLP::bslmf::MovableRefUtil::move(adoptee));
            d_rep_p = rep;
            BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(
                                                                       d_ptr_p,
                                                                       this);
        }
    }
# endif // BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS
#endif // BSLS_LIBRARYFEATURES_HAS_CPP11_UNIQUE_PTR

    template <class ANY_TYPE>
    shared_ptr(const shared_ptr<ANY_TYPE>&  source,
               ELEMENT_TYPE                *object) BSLS_CPP11_NOEXCEPT;
        // Create a shared pointer that manages the same modifiable object (if
        // any) as the specified 'source' shared pointer to the (template
        // parameter) type 'ANY_TYPE', and that refers to the modifiable object
        // at the specified 'object' address.  The resulting shared pointer is
        // known as an "alias" of 'source'.  Note that typically the objects
        // referred to by 'source' and 'object' have identical lifetimes (e.g.,
        // one might be a part of the other), so that the deleter for 'source'
        // will destroy them both, but they do not necessarily have the same
        // type.  Also note that if 'source' is empty, then an empty shared
        // pointer is created, even if 'object' is not null (in which case this
        // empty shared pointer will refer to the same object as 'object').
        // Also note that if 'object' is null and 'source' is not empty, then a
        // reference-counted null pointer alias will be created.

    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    shared_ptr(const shared_ptr<COMPATIBLE_TYPE>& other) BSLS_CPP11_NOEXCEPT;
        // Create a shared pointer that manages the same modifiable object (if
        // any) as the specified 'other' shared pointer to the (template
        // parameter) type 'COMPATIBLE_TYPE', uses the same deleter as 'other'
        // to destroy the shared object, and refers to
        // '(ELEMENT_TYPE*)other.get()'.  If 'COMPATIBLE_TYPE *' is not
        // implicitly convertible to 'ELEMENT_TYPE *', then a compiler
        // diagnostic will be emitted indicating the error.  Note that if
        // 'other' is empty, then an empty shared pointer is created, which may
        // still point to an un-managed object if 'other' were constructed
        // through an aliasing constructor.

    shared_ptr(const shared_ptr& original) BSLS_CPP11_NOEXCEPT;
        // Create a shared pointer that refers to and manages the same object
        // (if any) as the specified 'original' shared pointer, and uses the
        // same deleter as 'original' to destroy the shared object.  Note that
        // if 'original' is empty, then an empty shared pointer is created,
        // which may still point to an un-managed object if 'original' were
        // constructed through an aliasing constructor.

    shared_ptr(BloombergLP::bslmf::MovableRef<shared_ptr> original)
                                                           BSLS_CPP11_NOEXCEPT;
        // Create a shared pointer that refers to and assumes management of the
        // same object (if any) as the specified 'original' shared pointer,
        // using the same deleter as 'original' to destroy the shared object,
        // and reset 'original' to an empty state, not pointing to any object.
        // Note that if 'original' is empty, then an empty shared pointer is
        // created, which may still point to an un-managed object if 'original'
        // were constructed through an aliasing constructor.

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    shared_ptr(shared_ptr<COMPATIBLE_TYPE>&& other) BSLS_CPP11_NOEXCEPT;
        // Create a shared pointer that refers to and assumes management of the
        // same object (if any) as the specified 'other' shared pointer to the
        // (template parameter) type 'COMPATIBLE_TYPE', using the same deleter
        // as 'other' to destroy the shared object, and refers to
        // '(ELEMENT_TYPE*)other.get()'.  If 'COMPATIBLE_TYPE *' is not
        // implicitly convertible to 'ELEMENT_TYPE *', then a compiler
        // diagnostic will be emitted indicating the error.  Note that if
        // 'other' is empty, then an empty shared pointer is created, which may
        // still point to an un-managed object if 'other' were constructed
        // through an aliasing constructor.
#else
    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    shared_ptr(
            BloombergLP::bslmf::MovableRef<shared_ptr<COMPATIBLE_TYPE> > other)
                                                           BSLS_CPP11_NOEXCEPT;
        // Create a shared pointer that refers to and assumes management of the
        // same object (if any) as the specified 'other' shared pointer to the
        // (template parameter) type 'COMPATIBLE_TYPE', using the same deleter
        // as 'other' to destroy the shared object, and refers to
        // '(ELEMENT_TYPE*)other.get()'.  If 'COMPATIBLE_TYPE *' is not
        // implicitly convertible to 'ELEMENT_TYPE *', then a compiler
        // diagnostic will be emitted indicating the error.  Note that if
        // 'other' is empty, then an empty shared pointer is created, which may
        // still point to an un-managed object if 'other' were constructed
        // through an aliasing constructor.
#endif

    template<class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    explicit shared_ptr(const weak_ptr<COMPATIBLE_TYPE>& ptr);
        // Create a shared pointer that refers to and manages the same object
        // as the specified 'ptr' if 'ptr.expired()' is 'false'; otherwise,
        // create a shared pointer in the empty state.  Note that the
        // referenced and managed objects may be different if 'ptr' was created
        // from a 'shared_ptr' in an aliasing state.

#if !defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
    template<class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    explicit shared_ptr(
               BloombergLP::bslmf::MovableRef<weak_ptr<COMPATIBLE_TYPE> > ptr);
        // Create a shared pointer that refers to and manages the same object
        // as the specified 'ptr' if  'ptr.expired()' is 'false'; otherwise,
        // create a shared pointer in the empty state.  Note that the
        // referenced and managed objects may be different if 'ptr' was created
        // from a 'shared_ptr' in an aliasing state.  Also note that this
        // overloaded constructor is necessary only for C++03 compilers that
        // rely on the BDE move-emulation type, 'bslmf::MovableRef'; a C++11
        // compiler will pass rvalues directly to the constructor taking a
        // 'const weak_ptr&', rendering this constructor redundant.
#endif

    ~shared_ptr();
        // Destroy this shared pointer.  If this shared pointer refers to a
        // (possibly shared) object, then release the reference to that object,
        // and destroy the shared object using its associated deleter if this
        // shared pointer is the last reference to that object.

    // MANIPULATORS
    shared_ptr& operator=(const shared_ptr& rhs) BSLS_CPP11_NOEXCEPT;
        // Make this shared pointer manage the same modifiable object as the
        // specified 'rhs' shared pointer to the (template parameter) type
        // 'COMPATIBLE_TYPE', use the same deleter as 'rhs', and refer to
        // '(ELEMENT_TYPE *)rhs.get()'; return a reference providing modifiable
        // access to this shared pointer.  Note that if 'rhs' is empty, then
        // this shared pointer will also be empty after the assignment.  Also
        // note that if '*this' is the same object as 'rhs', then this method
        // has no effect.

    shared_ptr& operator=(BloombergLP::bslmf::MovableRef<shared_ptr> rhs)
                                                           BSLS_CPP11_NOEXCEPT;
        // Make this shared pointer manage the same modifiable object as the
        // specified 'rhs' shared pointer to the (template parameter) type
        // 'COMPATIBLE_TYPE', use the same deleter as 'rhs', and refer to
        // 'rhs.get()'; return a reference providing modifiable access to this
        // shared pointer.  Reset 'rhs' to an empty state, not pointing to any
        // object, unless '*this' is the same object as 'rhs'.  Note that if
        // 'rhs' is empty, then this shared pointer will also be empty after
        // the assignment.

    template <class COMPATIBLE_TYPE>
    typename enable_if<
        is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
        shared_ptr&>::type
    operator=(const shared_ptr<COMPATIBLE_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
        // Make this shared pointer refer to and manage the same modifiable
        // object as the specified 'rhs' shared pointer to the (template
        // parameter) type 'COMPATIBLE_TYPE', using the same deleter as 'rhs'
        // and referring to '(ELEMENT_TYPE *)rhs.get()', and return a reference
        // to this modifiable shared pointer.  If this shared pointer is
        // already managing a (possibly shared) object, then release the shared
        // reference to that object, and destroy it using its associated
        // deleter if this shared pointer held the last shared reference to
        // that object.  Note that if 'rhs' is empty, then this shared pointer
        // will also be empty after the assignment.

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
    template <class COMPATIBLE_TYPE>
    typename enable_if<
        is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
        shared_ptr&>::type
    operator=(shared_ptr<COMPATIBLE_TYPE>&& rhs) BSLS_CPP11_NOEXCEPT;
        // Make this shared pointer refer to and manage the same modifiable
        // object as the specified 'rhs' shared pointer to the (template
        // parameter) type 'COMPATIBLE_TYPE', using the same deleter as 'rhs'
        // and referring to '(ELEMENT_TYPE *)rhs.get()', and return a reference
        // to this modifiable shared pointer.  If this shared pointer is
        // already managing a (possibly shared) object, then release the shared
        // reference to that object, and destroy it using its associated
        // deleter if this shared pointer held the last shared reference to
        // that object.  Reset 'rhs' to an empty state, not pointing to any
        // object, unless '*this' is the same object as 'rhs'.  This function
        // does not exist unless a pointer to (template parameter)
        // 'COMPATIBLE_TYPE' is convertible to a pointer to the (template
        // paramter) 'ELEMENT_TYPE' of this 'shared_ptr'.  Note that if 'rhs'
        // is empty, then this shared pointer will also be empty after the
        // assignment.
#else
    template <class COMPATIBLE_TYPE>
    typename enable_if<
        is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
        shared_ptr&>::type
    operator=(BloombergLP::bslmf::MovableRef<shared_ptr<COMPATIBLE_TYPE> > rhs)
                                                           BSLS_CPP11_NOEXCEPT;
        // Make this shared pointer refer to and manage the same modifiable
        // object as the specified 'rhs' shared pointer to the (template
        // parameter) type 'COMPATIBLE_TYPE', using the same deleter as 'rhs'
        // and referring to '(ELEMENT_TYPE *)rhs.get()', and return a reference
        // to this modifiable shared pointer.  If this shared pointer is
        // already managing a (possibly shared) object, then release the shared
        // reference to that object, and destroy it using its associated
        // deleter if this shared pointer held the last shared reference to
        // that object.  Reset 'rhs' to an empty state, not pointing to any
        // object, unless '*this' is the same object as 'rhs'.  This function
        // does not exist unless a pointer to (template parameter)
        // 'COMPATIBLE_TYPE' is convertible to a pointer to the (template
        // paramter) 'ELEMENT_TYPE' of this 'shared_ptr'.  Note that if 'rhs'
        // is empty, then this shared pointer will also be empty after the
        // assignment.
#endif

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP98_AUTO_PTR)
    template <class COMPATIBLE_TYPE>
    typename enable_if<
        is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
        shared_ptr&>::type
    operator=(native_std::auto_ptr<COMPATIBLE_TYPE> rhs);
        // Transfer, to this shared pointer, ownership of the modifiable object
        // managed by the specified 'rhs' auto pointer to the (template
        // parameter) type 'COMPATIBLE_TYPE', and make this shared pointer
        // refer to '(ELEMENT_TYPE *)rhs.get()'.  'delete(autoPtr.release())'
        // will be called to destroy the shared object when all references have
        // been released.  If this shared pointer is already managing a
        // (possibly shared) object, then release the reference to that shared
        // object, and destroy it using its associated deleter if this shared
        // pointer held the last shared reference to that object.  Note that if
        // 'rhs' is empty, then this shared pointer will be empty after the
        // assignment.
#endif

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP11_UNIQUE_PTR)
    template <class COMPATIBLE_TYPE, class UNIQUE_DELETER>
    typename enable_if<
        is_convertible<
            typename
              native_std::unique_ptr<COMPATIBLE_TYPE, UNIQUE_DELETER>::pointer,
              ELEMENT_TYPE *>::value,
            shared_ptr&>::type
    operator=(native_std::unique_ptr<COMPATIBLE_TYPE, UNIQUE_DELETER>&& rhs);
        // Transfer, to this shared pointer, ownership of the object managed by
        // the specified 'rhs' unique pointer to the (template parameter) type
        // 'COMPATIBLE_TYPE', and make this shared pointer refer to
        // '(ELEMENT_TYPE *)rhs.get()'.  The deleter of 'rhs' will be called to
        // destroy the shared object when all references have been released.
        // If this shared pointer is already managing a (possibly shared)
        // object, then release the reference to that shared object, and
        // destroy it using its associated deleter if this shared pointer held
        // the last shared reference to that object.  This function does not
        // exist unless 'unqiue_ptr<COMPATIBLE_TYPE, DELETER>::pointer' is
        // convertible to 'ELEMENT_TYPE *'.  Note that if 'rhs' is empty, then
        // this shared pointer will be empty after the assignment.  Also note
        // that this function creates a 'shared_ptr' with an unspecified
        // deleter type that satisfies this contract; the C++11 standard
        // specifies the exact deleter that should be in use after assignment,
        // so this implementation may be non-conforming.
#endif

    void reset() BSLS_CPP11_NOEXCEPT;
        // Reset this shared pointer to the empty state.  If this shared
        // pointer is managing a (possibly shared) object, then release the
        // reference to the shared object, calling the associated deleter to
        // destroy the shared object if this shared pointer is the last shared
        // reference.

    template <class COMPATIBLE_TYPE>
    typename
      enable_if<is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value>::type
    reset(COMPATIBLE_TYPE *ptr);
        // Modify this shared pointer to manage the modifiable object of the
        // (template parameter) type 'COMPATIBLE_TYPE' at the specified 'ptr'
        // address and to refer to '(ELEMENT_TYPE *)ptr'.  If this shared
        // pointer is already managing a (possibly shared) object, then, unless
        // an exception is thrown allocating memory to manage 'ptr', release
        // the reference to the shared object, calling the associated deleter
        // to destroy the shared object if this shared pointer is the last
        // reference.  The currently installed default allocator is used to
        // allocate the internal representation of this shared pointer, and the
        // shared object will be destroyed by a call to 'delete ptr' when all
        // references have been released.  If an exception is thrown allocating
        // the internal representation, then 'delete ptr' is called and this
        // shared pointer retains ownership of its original object.  If
        // 'COMPATIBLE_TYPE*' is not implicitly convertible to 'ELEMENT_TYPE*',
        // then a compiler diagnostic will be emitted indicating the error.
        // Note that if 'ptr' is 0, then this shared pointer will still
        // allocate an internal representation to share ownership of that empty
        // state, which will be reclaimed when the last reference is destroyed.

    template <class COMPATIBLE_TYPE, class DELETER>
    typename
      enable_if<is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value>::type
    reset(COMPATIBLE_TYPE *ptr, DELETER deleter);
        // Modify this shared pointer to manage the modifiable object of the
        // (template parameter) type 'COMPATIBLE_TYPE' at the specified 'ptr'
        // address, refer to '(ELEMENT_TYPE *)ptr', and use the specified
        // 'deleter' to delete the shared object when all references have been
        // released.  If this shared pointer is already managing a (possibly
        // shared) object, then unless an exception is thrown allocating memory
        // to manage 'ptr', release the reference to the shared object, calling
        // the associated deleter to destroy the shared object if this shared
        // pointer is the last reference.  If 'DELETER' is an object type, then
        // 'deleter' is assumed to be a function-like deleter that may be
        // invoked to destroy the object referred to by a single argument of
        // type 'COMPATIBLE_TYPE *' (i.e., 'deleter(ptr)' will be called to
        // destroy the shared object).  If 'DELETER' is a pointer type that is
        // not a function pointer, then 'deleter' shall be a pointer to a
        // factory object that exposes a member function that can be invoked as
        // 'deleteObject(ptr)' that will be called to destroy the object at the
        // 'ptr' address (i.e., 'deleter->deleteObject(ptr)' will be called to
        // delete the shared object).  (See the "Deleters" section in the
        // component-level documentation.)  If 'DELETER' is also a pointer to
        // 'bslma::Allocator' or to a class derived from 'bslma::Allocator',
        // then that allocator will also be used to allocate and destroy the
        // internal representation of this shared pointer when all references
        // have been released; otherwise, the currently installed default
        // allocator is used to allocate and destroy the internal
        // representation of this shared pointer when all references have been
        // released.  If an exception is thrown allocating the internal
        // representation, then 'deleter(ptr)' is called (or
        // 'deleter->deleteObject(ptr)' for factory-type deleters) and this
        // shared pointer retains ownership of its original object.  If
        // 'COMPATIBLE_TYPE*' is not implicitly convertible to 'ELEMENT_TYPE*',
        // then a compiler diagnostic will be emitted indicating the error.
        // Note that, for factory deleters, 'deleter' must remain valid until
        // all references to 'ptr' have been released.  If 'ptr' is 0, then an
        // internal representation will still be allocated, and this shared
        // pointer will share ownership of a copy of 'deleter'.  Further note
        // that this function is logically equivalent to:
        //..
        //  *this = shared_ptr<ELEMENT_TYPE>(ptr, deleter);
        //..


    template <class COMPATIBLE_TYPE, class DELETER, class ALLOCATOR>
    typename
      enable_if<is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value>::type
    reset(COMPATIBLE_TYPE *ptr,
          DELETER          deleter,
          ALLOCATOR        basicAllocator);
        // Modify this shared pointer to manage the modifiable object of the
        // (template parameter) type 'COMPATIBLE_TYPE' at the specified 'ptr'
        // address, refer to '(ELEMENT_TYPE *)ptr' and use the specified
        // 'deleter' to delete the shared object when all references have been
        // released.  Use the specified 'basicAllocator' to allocate and
        // deallocate the internal representation of the shared pointer.  If
        // this shared pointer is already managing a (possibly shared) object,
        // then, unless an exception is thrown allocating memory to manage
        // 'ptr', release the shared reference to that shared object, and
        // destroy it using its associated deleter if this shared pointer held
        // the last shared reference to that object.  If 'DELETER' is a
        // reference type, then 'deleter' is assumed to be a function-like
        // deleter that may be invoked to destroy the object referred to by a
        // single argument of type 'COMPATIBLE_TYPE *' (i.e., 'deleter(ptr)'
        // will be called to destroy the shared object).  If 'DELETER' is a
        // pointer type, then 'deleter' is assumed to be a pointer to a factory
        // object that exposes a member function that can be invoked as
        // 'deleteObject(ptr)' that will be called to destroy the object at the
        // 'ptr' address (i.e., 'deleter->deleteObject(ptr)' will be called to
        // delete the shared object).  (See the "Deleters" section in the
        // component-level documentation.)  If an exception is thrown
        // allocating the internal representation, then 'deleter(ptr)' is
        // called (or 'deleter->deleteObject(ptr)' for factory-type deleters)
        // and this shared pointer retains ownership of its original object.
        // The behavior is undefined unless 'deleter(ptr)' is a well-defined
        // expression (or 'deleter->deleteObject(ptr)' for factory-type
        // deleters), and unless the copy constructor for 'deleter' does not
        // throw an exception.  If 'COMPATIBLE_TYPE *' is not implicitly
        // convertible to 'ELEMENT_TYPE *', then a compiler diagnostic will be
        // emitted indicating the error.  Note that, for factory deleters, the
        // 'deleter' must remain valid until all references to 'ptr' have been
        // released.  Also note that if 'ptr' is 0, then an internal
        // representation will still be allocated, and this shared pointer will
        // share ownership of a copy of 'deleter'.  Further note that this
        // function is logically equivalent to:
        //..
        //  *this = shared_ptr<ELEMENT_TYPE>(ptr, deleter, basicAllocator);
        //..

    template <class ANY_TYPE>
    void reset(const shared_ptr<ANY_TYPE>& source, ELEMENT_TYPE *ptr);
        // Modify this shared pointer to manage the same modifiable object (if
        // any) as the specified 'source' shared pointer to the (template
        // parameter) type 'ANY_TYPE', and refer to the modifiable object at
        // the specified 'ptr' address (i.e., make this shared pointer an
        // "alias" of 'source').  If this shared pointer is already managing a
        // (possibly shared) object, then release the reference to the shared
        // object, calling the associated deleter to destroy the shared object
        // if this shared pointer is the last reference.  Note that typically
        // the objects referred to by 'source' and 'ptr' have identical
        // lifetimes (e.g., one might be a part of the other), so that the
        // deleter for 'source' will destroy them both, but do not necessarily
        // have the same type.  Also note that if 'source' is empty, then this
        // shared pointer will be reset to an empty state, even if 'ptr' is not
        // null (in which case this empty shared pointer will refer to the same
        // object as 'ptr').  Also note that if 'ptr' is null and 'source' is
        // not empty, then this shared pointer will be reset to a
        // (reference-counted) null pointer alias.  Further note that the
        // behavior of this method is the same as 'loadAlias(source, ptr)'.
        // Finally note that this is a non-standard BDE extension to the C++
        // Standard 'shared_ptr' interface, which does not provide an alias
        // overload for the 'reset' function.

    void swap(shared_ptr& other) BSLS_CPP11_NOEXCEPT;
        // Efficiently exchange the states of this shared pointer and the
        // specified 'other' shared pointer such that each will refer to the
        // object formerly referred to by the other and each will manage the
        // object formerly managed by the other.

    // ADDITIONAL BSL MANIPULATORS
    void createInplace();
        // Create "in-place" in a large enough contiguous memory region both an
        // internal representation for this shared pointer and a
        // default-constructed object of 'ELEMENT_TYPE', and make this shared
        // pointer refer to the newly-created 'ELEMENT_TYPE' object.  The
        // currently installed default allocator is used to supply memory.  If
        // an exception is thrown during allocation or construction of the
        // 'ELEMENT_TYPE' object, this shared pointer will be unchanged.
        // Otherwise, if this shared pointer is already managing a (possibly
        // shared) object, then release the shared reference to that shared
        // object, and destroy it using its associated deleter if this shared
        // pointer held the last shared reference to that object.

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES // $var-args=14
    template <class... ARGS>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       ARGS&&...                      args);
        // Create "in-place" in a large enough contiguous memory region, using
        // the specified 'basicAllocator' to supply memory, both an internal
        // representation for this shared pointer and an object of
        // 'ELEMENT_TYPE' using the 'ELEMENT_TYPE' constructor that takes the
        // specified 'args...' arguments, and make this shared pointer refer to
        // the newly-created 'ELEMENT_TYPE' object.  If an exception is thrown
        // during the construction of the 'ELEMENT_TYPE' object, this shared
        // pointer will be unchanged.  Otherwise, if this shared pointer is
        // already managing a (possibly shared) object, then release the shared
        // reference to that shared object, and destroy it using its associated
        // deleter if this shared pointer held the last shared reference to
        // that object.  Note that the allocator argument is *not* implicitly
        // passed to the constructor for 'ELEMENT_TYPE'; to construct an object
        // of 'ELEMENT_TYPE' with an allocator, pass the allocator as one of
        // the arguments (typically the last argument), or assign with a
        // 'shared_ptr' created using the standard 'allocate_shared' function.
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl --var-args=14 bslstl_sharedptr.h
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator);

    template <class ARGS_01>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01);

    template <class ARGS_01,
              class ARGS_02>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07,
              class ARGS_08>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07,
              class ARGS_08,
              class ARGS_09>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07,
              class ARGS_08,
              class ARGS_09,
              class ARGS_10>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07,
              class ARGS_08,
              class ARGS_09,
              class ARGS_10,
              class ARGS_11>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07,
              class ARGS_08,
              class ARGS_09,
              class ARGS_10,
              class ARGS_11,
              class ARGS_12>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07,
              class ARGS_08,
              class ARGS_09,
              class ARGS_10,
              class ARGS_11,
              class ARGS_12,
              class ARGS_13>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13);

    template <class ARGS_01,
              class ARGS_02,
              class ARGS_03,
              class ARGS_04,
              class ARGS_05,
              class ARGS_06,
              class ARGS_07,
              class ARGS_08,
              class ARGS_09,
              class ARGS_10,
              class ARGS_11,
              class ARGS_12,
              class ARGS_13,
              class ARGS_14>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14);

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.
    template <class... ARGS>
    void createInplace(BloombergLP::bslma::Allocator *basicAllocator,
                       BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args);
// }}} END GENERATED CODE
#endif

    template <class ANY_TYPE>
    void loadAlias(const shared_ptr<ANY_TYPE>&  source,
                   ELEMENT_TYPE                *object);
        // [!DEPRECATED!] Use 'reset' instead.
        //
        // Modify this shared pointer to manage the same modifiable object (if
        // any) as the specified 'source' shared pointer to the (template
        // parameter) type 'ANY_TYPE', and refer to the modifiable object at
        // the specified 'object' address (i.e., make this shared pointer an
        // "alias" of 'source').  If this shared pointer is already managing a
        // (possibly shared) object, then release the shared reference to that
        // shared object, and destroy it using its associated deleter if this
        // shared pointer held the last shared reference to that object.  Note
        // that typically the objects referred to by 'source' and 'object' have
        // identical lifetimes (e.g., one might be a part of the other), so
        // that the deleter for 'source' will destroy them both, but they do
        // not necessarily have the same type.  Also note that if 'source' is
        // empty, then this shared pointer will be reset to an empty state,
        // even if 'object' is not null (in which case this empty shared
        // pointer will refer to the same object as 'object').  Also note that
        // if 'object' is null and 'source' is not empty, then this shared
        // pointer will be reset to a (reference-counted) null pointer alias.
        // Also note that this function is logically equivalent to:
        //..
        //  *this = shared_ptr<ELEMENT_TYPE>(source, object);
        //..
        // Further note that the behavior of this method is the same as
        // 'reset(source, object)'.

    pair<ELEMENT_TYPE *, BloombergLP::bslma::SharedPtrRep *> release()
                                                           BSLS_CPP11_NOEXCEPT;
        // Return the pair consisting of the addresses of the modifiable
        // 'ELEMENT_TYPE' object referred to, and the representation shared by,
        // this shared pointer, and reset this shared pointer to the empty
        // state, referring to no object, with no effect on the representation.
        // The reference counter is not modified nor is the shared object
        // deleted; if the reference count of the representation is greater
        // than one, then it is not safe to release the representation (thereby
        // destroying the shared object), but it is always safe to create
        // another shared pointer with the representation using the constructor
        // with the signature
        // 'shared_ptr(ELEMENT_TYPE                     *ptr,
        //             BloombergLP::bslma::SharedPtrRep *rep)'.
        // Note that this function returns a pair of null pointers if this
        // shared pointer is empty.


    // ACCESSORS
    operator BoolType() const BSLS_CPP11_NOEXCEPT;
        // Return a value of an "unspecified bool" type that evaluates to
        // 'false' if this shared pointer does not refer to an object, and
        // 'true' otherwise.  Note that this conversion operator allows a
        // shared pointer to be used within a conditional context (e.g., within
        // an 'if' or 'while' statement), but does *not* allow shared pointers
        // to unrelated types to be compared (e.g., via '<' or '>').

    typename add_lvalue_reference<ELEMENT_TYPE>::type
    operator*() const BSLS_CPP11_NOEXCEPT;
        // Return a reference providing modifiable access to the object
        // referred to by this shared pointer.  The behavior is undefined
        // unless this shared pointer refers to an object, and 'ELEMENT_TYPE'
        // is not (potentially 'const' or 'volatile' qualified) 'void'.

    ELEMENT_TYPE *operator->() const BSLS_CPP11_NOEXCEPT;
        // Return the address providing modifiable access to the object
        // referred to by this shared pointer, or 0 if this shared pointer does
        // not refer to an object.  Note that applying this operator
        // conventionally (e.g., to invoke a method) to an shared pointer that
        // does not refer to an object will result in undefined behavior.

    ELEMENT_TYPE *get() const BSLS_CPP11_NOEXCEPT;
        // Return the address providing modifiable access to the object
        // referred to by this shared pointer, or 0 if this shared pointer does
        // not refer to an object.

    template<class ANY_TYPE>
    bool owner_before(const shared_ptr<ANY_TYPE>& other) const;
    template<class ANY_TYPE>
    bool owner_before(const weak_ptr<ANY_TYPE>& other) const;
        // Return 'true' if the address of the
        // 'BloombergLP::bslma::SharedPtrRep' object used by this shared
        // pointer is ordered before the address of the
        // 'BloombergLP::bslma::SharedPtrRep' object used by the specified
        // 'other' shared pointer under the total ordering defined by
        // 'std::less<BloombergLP::bslma::SharedPtrRep *>', and 'false'
        // otherwise.

    bool unique() const BSLS_CPP11_NOEXCEPT;
        // Return 'true' if this shared pointer is not empty and does not share
        // ownership of the object it managed with any other shared pointer,
        // and 'false' otherwise.  Note that a shared pointer with a custom
        // deleter can refer to a null pointer without being empty, and so may
        // be 'unique'.  Also note that the result of this function may not be
        // reliable in a multithreaded program, where a weak pointer may be
        // locked on another thread.  This function is deprecated in C++17.

    long use_count() const BSLS_CPP11_NOEXCEPT;
        // Return a "snapshot" of the number of shared pointers (including this
        // one) that share ownership of the object managed by this shared
        // pointer.  Note that 0 is returned if this shared pointer is empty.
        // Also note that any result other than 0 may be unreliable in a
        // multithreaded program, where another pointer sharing ownership in a
        // different thread may be copied or destroyed, or a weak pointer may
        // be locked in the case that 1 is returned (that would otherwise
        // indicate unique ownership).

    // ADDITIONAL BSL ACCESSORS
    typename add_lvalue_reference<ELEMENT_TYPE>::type
    operator[](ptrdiff_t index) const;
        // Return a reference providing modifiable access to the object at the
        // specified 'index' offset in the object referred to by this shared
        // pointer.  The behavior is undefined unless this shared pointer is
        // not empty, 'ELEMENT_TYPE' is not 'void' (a compiler error will be
        // generated if this operator is instantiated within the
        // 'shared_ptr<void>' class), and this shared pointer refers to an
        // array of 'ELEMENT_TYPE' objects.  Note that this method is logically
        // equivalent to '*(get() + index)'.

    BloombergLP::bslma::ManagedPtr<ELEMENT_TYPE> managedPtr() const;
        // Return a managed pointer that refers to the same object as this
        // shared pointer.  If this shared pointer is not empty, and is not
        // null, then increment the shared count on the shared object, and give
        // the managed pointer a deleter that decrements the reference count
        // for the shared object.  Note that if this 'shared_ptr' is reference-
        // counting a null pointer, the empty 'bslma::ManagedPtr' returned will
        // not participate in that shared ownership.

    BloombergLP::bslma::SharedPtrRep *rep() const BSLS_CPP11_NOEXCEPT;
        // Return the address providing modifiable access to the
        // 'BloombergLP::bslma::SharedPtrRep' object used by this shared
        // pointer, or 0 if this shared pointer is empty.

};

// FREE OPERATORS
template <class LHS_TYPE, class RHS_TYPE>
bool operator==(const shared_ptr<LHS_TYPE>& lhs,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer refers to the same
    // object (if any) as that referred to by the specified 'rhs' shared
    // pointer (if any), and 'false' otherwise; a compiler diagnostic will be
    // emitted indicating the error unless a (raw) pointer to 'LHS_TYPE' can
    // be compared to a (raw) pointer to 'RHS_TYPE'.  Note that two shared
    // pointers that compare equal do not necessarily manage the same object
    // due to aliasing.

template <class LHS_TYPE, class RHS_TYPE>
bool operator!=(const shared_ptr<LHS_TYPE>& lhs,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer does not refer to
    // the same object (if any) as that referred to by the specified 'rhs'
    // shared pointer (if any), and 'false' otherwise; a compiler diagnostic
    // will be emitted indicating the error unless a (raw) pointer to
    // 'LHS_TYPE' can be compared to a (raw) pointer to 'RHS_TYPE'.  Note that
    // two shared pointers that do not compare equal may manage the same object
    // due to aliasing.

template<class LHS_TYPE, class RHS_TYPE>
bool operator<(const shared_ptr<LHS_TYPE>& lhs,
               const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the address of the object that the specified 'lhs'
    // shared pointer refers to is ordered before the address of the object
    // that the specified 'rhs' shared pointer refers to under the total
    // ordering supplied by 'std::less<T *>', where 'T *' is the composite
    // pointer type of 'LHS_TYPE *' and 'RHS_TYPE *', and 'false' otherwise.

template<class LHS_TYPE, class RHS_TYPE>
bool operator>(const shared_ptr<LHS_TYPE>& lhs,
               const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the address of the object that the specified 'lhs'
    // shared pointer refers to is ordered after the address of the object
    // that the specified 'rhs' shared pointer refers to under the total
    // ordering supplied by 'std::less<T *>', where 'T *' is the composite
    // pointer type of 'LHS_TYPE *' and 'RHS_TYPE *', and 'false' otherwise.

template<class LHS_TYPE, class RHS_TYPE>
bool operator<=(const shared_ptr<LHS_TYPE>& lhs,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer refers to the same
    // object as the specified 'rhs' shared pointer, or if the address of the
    // object referred to by 'lhs' (if any) is ordered before the address of
    // the object referred to by 'rhs' (if any) under the total ordering
    // supplied by 'std::less<T *>', where 'T *' is the composite pointer type
    // of 'LHS_TYPE *' and 'RHS_TYPE *', and 'false' otherwise.

template<class LHS_TYPE, class RHS_TYPE>
bool operator>=(const shared_ptr<LHS_TYPE>& lhs,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer refers to the same
    // object as the specified 'rhs' shared pointer, or if the address of the
    // object referred to by 'lhs' (if any) is ordered after the address of the
    // object referred to by 'rhs' (if any) under the total ordering supplied
    // by 'std::less<T *>', where 'T *' is the composite pointer type of
    // 'LHS_TYPE *' and 'RHS_TYPE *', and 'false' otherwise.

template <class LHS_TYPE>
bool operator==(const shared_ptr<LHS_TYPE>& lhs,
                nullptr_t) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer does not refer to an
    // object, and 'false' otherwise.

template <class RHS_TYPE>
bool operator==(nullptr_t,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'rhs' shared pointer does not refer to an
    // object, and 'false' otherwise.

template <class LHS_TYPE>
bool operator!=(const shared_ptr<LHS_TYPE>& lhs,
                nullptr_t) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer refers to an object,
    // and 'false' otherwise.

template <class RHS_TYPE>
bool operator!=(nullptr_t,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'rhs' shared pointer refers to an object,
    // and 'false' otherwise.

template <class LHS_TYPE>
bool operator<(const shared_ptr<LHS_TYPE>& lhs, nullptr_t) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the address of the object referred to by the specified
    // 'lhs' shared pointer is ordered before the null-pointer value under the
    // total ordering supplied by 'std::less<LHS_TYPE *>', and 'false'
    // otherwise.

template <class RHS_TYPE>
bool operator<(nullptr_t, const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the address of the object referred to by the specified
    // 'rhs' shared pointer is ordered after the null-pointer value under the
    // total ordering supplied by 'std::less<RHS_TYPE *>', and 'false'
    // otherwise.

template <class LHS_TYPE>
bool operator<=(const shared_ptr<LHS_TYPE>& lhs,
                nullptr_t) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer does not refer to an
    // object, or if the address of the object referred to by 'lhs' is ordered
    // before the null-pointer value under the total ordering supplied by
    // 'std::less<LHS_TYPE *>', and 'false' otherwise.

template <class RHS_TYPE>
bool operator<=(nullptr_t,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'rhs' shared pointer does not refer to an
    // object, or if the address of the object referred to by 'rhs' is ordered
    // after the null-pointer value under the total ordering supplied by
    // 'std::less<RHS_TYPE *>', and 'false' otherwise.

template <class LHS_TYPE>
bool operator>(const shared_ptr<LHS_TYPE>& lhs, nullptr_t) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the address of the object referred to by the specified
    // 'lhs' shared pointer is ordered after the null-pointer value under the
    // total ordering supplied by 'std::less<LHS_TYPE *>', and 'false'
    // otherwise.

template <class RHS_TYPE>
bool operator>(nullptr_t, const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the address of the object referred to by the specified
    // 'rhs' shared pointer is ordered before the null-pointer value under the
    // total ordering supplied by 'std::less<RHS_TYPE *>', and 'false'
    // otherwise.

template <class LHS_TYPE>
bool operator>=(const shared_ptr<LHS_TYPE>& lhs,
                nullptr_t) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'lhs' shared pointer does not refer to an
    // object, or if the address of the object referred to by 'lhs' is ordered
    // after the null-pointer value under the total ordering supplied by
    // 'std::less<LHS_TYPE *>', and 'false' otherwise.

template <class RHS_TYPE>
bool operator>=(nullptr_t,
                const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    // Return 'true' if the specified 'rhs' shared pointer does not refer to an
    // object, or if the address of the object referred to by 'rhs' is ordered
    // before the null-pointer value under the total ordering supplied by
    // 'std::less<RHS_TYPE *>', and 'false' otherwise.

template<class CHAR_TYPE, class CHAR_TRAITS, class ELEMENT_TYPE>
native_std::basic_ostream<CHAR_TYPE, CHAR_TRAITS>&
operator<<(native_std::basic_ostream<CHAR_TYPE, CHAR_TRAITS>& stream,
           const shared_ptr<ELEMENT_TYPE>&                    rhs);
    // Print to the specified 'stream' the address of the shared object
    // referred to by the specified 'rhs' shared pointer and return a reference
    // to the modifiable 'stream'.

// ASPECTS
template <class HASHALG, class ELEMENT_TYPE>
void hashAppend(HASHALG& hashAlg, const shared_ptr<ELEMENT_TYPE>& input);
    // Pass the address of the object referred to by the specified 'input'
    // shared pointer to the specified 'hashAlg' hashing algorithm of (template
    // parameter) type 'HASHALG'.

template <class ELEMENT_TYPE>
void swap(shared_ptr<ELEMENT_TYPE>& a, shared_ptr<ELEMENT_TYPE>& b)
                                                           BSLS_CPP11_NOEXCEPT;
    // Efficiently exchange the states of the specified 'a' and 'b' shared
    // pointers such that each will refer to the object formerly referred to by
    // the other, and each will manage the object formerly managed by the
    // other.

// STANDARD FREE FUNCTIONS
template<class DELETER, class ELEMENT_TYPE>
DELETER *get_deleter(const shared_ptr<ELEMENT_TYPE>& p) BSLS_CPP11_NOEXCEPT;
    // Return the address of deleter used by the specified 'p' shared pointer
    // if the (template parameter) type 'DELETER' is the type of the deleter
    // installed in 'p', and a null pointer value otherwise.

// STANDARD CAST FUNCTIONS
template<class TO_TYPE, class FROM_TYPE>
shared_ptr<TO_TYPE> const_pointer_cast(const shared_ptr<FROM_TYPE>& source)
                                                           BSLS_CPP11_NOEXCEPT;
    // Return a 'shared_ptr<TO_TYPE>' object sharing ownership of the same
    // object as the specified 'source' shared pointer to the (template
    // parameter) 'FROM_TYPE', and referring to
    // 'const_cast<TO_TYPE *>(source.get())'.  Note that if 'source' cannot be
    // 'const'-cast to 'TO_TYPE *', then a compiler diagnostic will be emitted
    // indicating the error.

template<class TO_TYPE, class FROM_TYPE>
shared_ptr<TO_TYPE> dynamic_pointer_cast(const shared_ptr<FROM_TYPE>& source)
                                                           BSLS_CPP11_NOEXCEPT;
    // Return a 'shared_ptr<TO_TYPE>' object sharing ownership of the same
    // object as the specified 'source' shared pointer to the (template
    // parameter) 'FROM_TYPE', and referring to
    // 'dynamic_cast<TO_TYPE*>(source.get())'.  If 'source' cannot be
    // dynamically cast to 'TO_TYPE *', then an empty 'shared_ptr<TO_TYPE>'
    // object is returned.

template<class TO_TYPE, class FROM_TYPE>
shared_ptr<TO_TYPE> static_pointer_cast(const shared_ptr<FROM_TYPE>& source)
                                                           BSLS_CPP11_NOEXCEPT;
    // Return a 'shared_ptr<TO_TYPE>' object sharing ownership of the same
    // object as the specified 'source' shared pointer to the (template
    // parameter) 'FROM_TYPE', and referring to
    // 'static_cast<TO_TYPE *>(source.get())'.  Note that if 'source' cannot be
    // statically cast to 'TO_TYPE *', then a compiler diagnostic will be
    // emitted indicating the error.

// STANDARD FACTORY FUNCTIONS
#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES // $var-args=14
template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
    // work-around for gcc variadic template bug - confirm not fixed by 4.9?
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator, ARGS&&... args);
    // Return a 'shared_ptr' object referring to and managing a new
    // 'ELEMENT_TYPE' object.  The specified 'basicAllocator' will be used to
    // supply a single contiguous region of memory holding the returned shared
    // pointer's internal representation and the new 'ELEMENT_TYPE' object,
    // which is initialized by calling 'allocator_traits<ALLOC>::construct'
    // passing 'basicAllocator', an 'ELEMENT_TYPE *' pointer to space for the
    // new shared object, and the specified arguments
    // 'std::forward<ARGS>(args)...'.

template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator, ARGS&&... args);
    // Return a 'shared_ptr' object referring to and managing a new
    // 'ELEMENT_TYPE' object.  The specified 'basicAllocator' will be used to
    // supply a single contiguous region of memory holding the returned shared
    // pointer's internal representation and the new 'ELEMENT_TYPE' object,
    // which is initialized using the 'ELEMENT_TYPE' constructor that takes the
    // specified arguments 'std::forward<ARGS>(args)...'.  If 'ELEMENT_TYPE'
    // uses 'bslma' allocators, then 'basicAllocator' is passed as an extra
    // argument in the final position.  If 'basicAllocator' is 0, then the
    // default allocator will be used instead, and passed as the allocator,
    // when appropriate, to the 'ELEMENT_TYPE' constructor.

template<class ELEMENT_TYPE, class... ARGS>
shared_ptr<ELEMENT_TYPE> make_shared(ARGS&&... args);
    // Return a 'shared_ptr' object referring to and managing a new
    // 'ELEMENT_TYPE' object.  The default allocator will be used to supply a
    // single contiguous region of memory holding the returned shared pointer's
    // internal representation and the new 'ELEMENT_TYPE' object, which is
    // initialized using the 'ELEMENT_TYPE' constructor that takes the
    // specified arguments 'std::forward<ARGS>(args)...'.  If 'ELEMENT_TYPE'
    // uses 'bslma' allocators, then the default allocator is passed as an
    // extra argument in the final position.
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl --var-args=14 bslstl_sharedptr.h
template<class ELEMENT_TYPE, class ALLOC>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13,
                                          class ARGS_14>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14);


template<class ELEMENT_TYPE, class ALLOC>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13);

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13,
                                          class ARGS_14>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14);


template<class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE> make_shared(
                              );

template<class ELEMENT_TYPE, class ARGS_01>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11,
                             class ARGS_12>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11,
                             class ARGS_12,
                             class ARGS_13>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13);

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11,
                             class ARGS_12,
                             class ARGS_13,
                             class ARGS_14>
shared_ptr<ELEMENT_TYPE> make_shared(
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                           BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14);

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.
template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename enable_if<!is_pointer<ALLOC>::value,
                   shared_ptr<ELEMENT_TYPE> >::type
# else
shared_ptr<ELEMENT_TYPE>
# endif
allocate_shared(ALLOC basicAllocator,
                              BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args);

template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
shared_ptr<ELEMENT_TYPE>
allocate_shared(ALLOC *basicAllocator,
                              BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args);

template<class ELEMENT_TYPE, class... ARGS>
shared_ptr<ELEMENT_TYPE> make_shared(
                              BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args);
// }}} END GENERATED CODE
#endif

                        // ==============
                        // class weak_ptr
                        // ==============

template <class ELEMENT_TYPE>
class weak_ptr {
    // This 'class' provides a mechanism to create weak references to
    // reference-counted shared ('shared_ptr') objects.  A weak reference
    // provides conditional access to a shared object managed by a
    // 'shared_ptr', but, unlike a shared (or "strong") reference, does not
    // affect the shared object's lifetime.

    // DATA
    ELEMENT_TYPE                     *d_ptr_p; // pointer to the referenced
                                               // object

    BloombergLP::bslma::SharedPtrRep *d_rep_p; // pointer to the representation
                                               // object that manages the
                                               // shared object (held, not
                                               // owned)

    // PRIVATE MANIPULATORS
    void privateAssign(BloombergLP::bslma::SharedPtrRep *rep,
                       ELEMENT_TYPE                     *target);
        // Release weak ownership of the currently managed shared pointer rep
        // and assign to this weak pointer weak ownership of the specified
        // shared pointer 'rep', aliasing the specified 'target' pointer.

    // FRIENDS
    template <class COMPATIBLE_TYPE>
    friend class weak_ptr;
        // This 'friend' declaration provides access to the internal data
        // members while constructing a weak pointer from a weak pointer of a
        // different type.

    friend struct BloombergLP::bslstl::SharedPtr_ImpUtil;

  public:
    // TYPES
    typedef ELEMENT_TYPE element_type;
        // 'element_type' is an alias for the 'ELEMENT_TYPE' parameter of this
        // class template.


    // CREATORS
    BSLS_CPP11_CONSTEXPR
    weak_ptr() BSLS_CPP11_NOEXCEPT;
        // Create a weak pointer in the empty state and referring to no object,
        // i.e., a weak pointer having no representation.

    weak_ptr(BloombergLP::bslmf::MovableRef<weak_ptr> original)
                                                           BSLS_CPP11_NOEXCEPT;
        // Create a weak pointer that refers to the same object (if any) as the
        // specified 'original' weak pointer, and reset 'original' to an empty
        // state.

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    weak_ptr(weak_ptr<COMPATIBLE_TYPE>&& other) BSLS_CPP11_NOEXCEPT;
#else
    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    weak_ptr(BloombergLP::bslmf::MovableRef<weak_ptr<COMPATIBLE_TYPE> > other)
                                                           BSLS_CPP11_NOEXCEPT;
#endif
        // Create a weak pointer that refers to the same object (if any) as the
        // specified 'other' weak pointer, and reset 'original' to an empty
        // state.  Note that this operation does not involve any change to
        // reference counts.

    weak_ptr(const weak_ptr& original) BSLS_CPP11_NOEXCEPT;
        // Create a weak pointer that refers to the same object (if any) as the
        // specified 'original' weak pointer, and increment the number of weak
        // references to the object managed by 'original' (if any).  Note that
        // if 'original' is in the empty state, this weak pointer will be
        // initialized to the empty state.

    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    weak_ptr(const shared_ptr<COMPATIBLE_TYPE>& other) BSLS_CPP11_NOEXCEPT;
                                                                    // IMPLICIT
    template <class COMPATIBLE_TYPE
              BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE>
    weak_ptr(const weak_ptr<COMPATIBLE_TYPE>& other) BSLS_CPP11_NOEXCEPT;
                                                                    // IMPLICIT
        // Create a weak pointer that refers to the same object (if any) as the
        // specified 'other' (shared or weak) pointer of the (template
        // parameter) 'COMPATIBLE_TYPE', and increment the number of weak
        // references to the object managed by 'other' (if any).  If
        // 'COMPATIBLE_TYPE *' is not implicitly convertible to
        // 'ELEMENT_TYPE *', then a compiler diagnostic will be emitted.  Note
        // that if 'other' is in the empty state, this weak pointer will be
        // initialized to the empty state.

    ~weak_ptr();
        // Destroy this weak pointer object.  If this weak pointer manages a
        // (possibly shared) object, release the weak reference to that object.

    // MANIPULATORS
    weak_ptr& operator=(BloombergLP::bslmf::MovableRef<weak_ptr> rhs)
                                                           BSLS_CPP11_NOEXCEPT;
        // Make this weak pointer refer to the same object (if any) as the
        // specified 'rhs' weak pointer.  If 'rhs' is not a reference to this
        // weak pointer, decrement the number of weak references to the object
        // this weak pointer managed (if any), and reset 'rhs' to an empty
        // state.  Return a reference providing modifiable access to this weak
        // pointer.  Note that if 'rhs' is in an empty state, this weak pointer
        // will be set to an empty state.

    weak_ptr& operator=(const weak_ptr& rhs) BSLS_CPP11_NOEXCEPT;
        // Make this weak pointer refer to the same object (if any) as the
        // specified 'rhs' weak pointer.  Decrement the number of weak
        // references to the object this weak pointer manages (if any), and
        // increment the number of weak references to the object managed by
        // 'rhs' (if any).  Return a reference providing modifiable access to
        // this weak pointer.  Note that if 'rhs' is in an empty state, this
        // weak pointer will be set to an empty state.

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
    template <class COMPATIBLE_TYPE>
    typename enable_if<
     is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value, weak_ptr&>::type
    operator=(weak_ptr<COMPATIBLE_TYPE>&& rhs) BSLS_CPP11_NOEXCEPT;
#else
    template <class COMPATIBLE_TYPE>
    typename enable_if<
     is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value, weak_ptr&>::type
    operator=(BloombergLP::bslmf::MovableRef<weak_ptr<COMPATIBLE_TYPE> > rhs)
                                                           BSLS_CPP11_NOEXCEPT;
#endif
        // Make this weak pointer refer to the same object (if any) as the
        // specified 'rhs' weak pointer.  Decrement the number of weak
        // references to the object this weak pointer managed (if any), and
        // reset 'rhs' to an empty state.  Return a reference providing
        // modifiable access to this weak pointer.  This function does not
        // exist unless a pointer to (the template parameter) 'COMPATIBLE_TYPE'
        // is convertible to a pointer to (the template parameter)
        // 'ELEMENT_TYPE'.  Note that if 'rhs' is in an empty state, this weak
        // pointer will be set to an empty state.

    template <class COMPATIBLE_TYPE>
    typename enable_if<
     is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value, weak_ptr&>::type
    operator=(const shared_ptr<COMPATIBLE_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
    template <class COMPATIBLE_TYPE>
    typename enable_if<
     is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value, weak_ptr&>::type
    operator=(const weak_ptr<COMPATIBLE_TYPE>& rhs) BSLS_CPP11_NOEXCEPT;
        // Make this weak pointer refer to the same object (if any) as the
        // specified 'rhs' (shared or weak) pointer to the (template parameter)
        // 'COMPATIBLE_TYPE'.  Decrement the number of weak references to the
        // object to which this weak pointer currently manages (if any), and
        // increment the number of weak references to the object managed by
        // 'rhs' (if any).  Return a reference providing modifiable access to
        // this weak pointer.  If 'COMPATIBLE_TYPE *' is not implicitly
        // convertible to 'TYPE *', then a compiler diagnostic will be emitted.
        // Note that if 'rhs' is in the empty state, this weak pointer will be
        // set to the empty state.

    void reset() BSLS_CPP11_NOEXCEPT;
        // Reset this weak pointer to the empty state.  If this weak pointer
        // manages a (possibly shared) object, then decrement the number of
        // weak references to that object.

    void swap(weak_ptr& other) BSLS_CPP11_NOEXCEPT;
        // Efficiently exchange the states of this weak pointer and the
        // specified 'other' weak pointer such that each will refer to the
        // object (if any) and representation (if any) formerly referred to and
        // managed by the other.

    // ACCESSORS
    bool expired() const BSLS_CPP11_NOEXCEPT;
        // Return 'true' if this weak pointer is in the empty state or the
        // object that it originally referenced has been destroyed, and 'false'
        // otherwise.

    shared_ptr<ELEMENT_TYPE> lock() const BSLS_CPP11_NOEXCEPT;
        // Return a shared pointer to the object referred to by this weak
        // pointer if 'false == expired()', and a shared pointer in the empty
        // state otherwise.

    template <class ANY_TYPE>
    bool owner_before(const shared_ptr<ANY_TYPE>& other) const;
    template <class ANY_TYPE>
    bool owner_before(const weak_ptr<ANY_TYPE>& other) const;
        // Return 'true' if the address of the
        // 'BloombergLP::bslma::SharedPtrRep' object used by this weak pointer
        // is ordered before the address of the
        // 'BloombergLP::bslma::SharedPtrRep' object used by the specified
        // 'other' shared pointer under the total ordering defined by
        // 'std::less<BloombergLP::bslma::SharedPtrRep *>', and 'false'
        // otherwise.

    BloombergLP::bslma::SharedPtrRep *rep() const BSLS_CPP11_NOEXCEPT;
        // Return the address providing modifiable access to the
        // 'BloombergLP::bslma::SharedPtrRep' object held by this weak pointer,
        // or 0 if this weak pointer is in the empty state.

    long use_count() const BSLS_CPP11_NOEXCEPT;
        // Return a "snapshot" of the current number of shared pointers that
        // share ownership of the object referred to by this weak pointer, or 0
        // if this weak pointer is in the empty state.  Note that any result
        // other than 0 may be unreliable in a multithreaded program, where
        // another pointer sharing ownership in a different thread may be
        // copied or destroyed, or another weak pointer may be locked in the
        // case that 1 is returned (that would otherwise indicate unique
        // ownership).

};

                    //==============================
                    // class enable_shared_from_this
                    //==============================

template<class ELEMENT_TYPE>
class enable_shared_from_this {
    // This class allows an object that is currently managed by a 'shared_ptr'
    // to safely generate a copy of the managing 'shared_ptr' object.
    // Inheriting from 'enable_shared_from_this<ELEMENT_TYPE>' provides the
    // (template parameter) 'ELEMENT_TYPE' type with a member function
    // 'shared_from_this'.  If an object of type 'ELEMENT_TYPE' is managed by a
    // 'shared_ptr' then calling 'shared_from_this' will return a
    // 'shared_ptr<ELEMENT_TYPE>' that shares ownership of that object.  It is
    // undefined behavior to call 'shared_from_this' on an object unless that
    // object is managed by a 'shared_ptr'.
    //
    // The intended use of 'enable_shared_from_this' is that the (template
    // parameter) type 'ELEMENT_TYPE' inherits directly from the
    // 'enable_shared_from_this' class template.  In the case of multiple
    // inheritance, only one of the base classes should inherit from the
    // 'enable_shared_from_this' class template.  If multiple base classes
    // inherit from 'enable_shared_from_this', then there will be ambiguous
    // calls to the 'shared_from_this' function.

    // FRIENDS
    friend struct BloombergLP::bslstl::SharedPtr_ImpUtil;
        // Allows 'shared_ptr' to initialize 'd_weakThis' when it detects an
        // 'enable_shared_from_this' base class.

  private:
    // DATA
    mutable bsl::weak_ptr<ELEMENT_TYPE> d_weakThis;

  protected:
    // PROTECTED CREATORS
    enable_shared_from_this() BSLS_CPP11_NOEXCEPT;
        // Create an 'enable_shared_from_this' object that is not owned by any
        // 'shared_ptr' object.

    enable_shared_from_this(const enable_shared_from_this& unused)
                                                           BSLS_CPP11_NOEXCEPT;
        // Create an 'enable_shared_from_this' object that is not owned by any
        // 'shared_ptr' object.  Note that the specified 'unused' argument is
        // not used by this constructor.

    ~enable_shared_from_this();
        // Destroy this 'enable_shared_form_this'.

    // PROTECTED MANIPULATORS
    enable_shared_from_this& operator=(const enable_shared_from_this& rhs)
                                                           BSLS_CPP11_NOEXCEPT;
        // Return '*this'. This object is unchanged.  Note that the specified
        // 'rhs' is not used.

  public:
    // MANIPULATORS
    bsl::shared_ptr<ELEMENT_TYPE> shared_from_this();
        // Return a 'shared_ptr<ELEMENT_TYPE>' that shares ownership with an
        // existing 'shared_ptr' object that managed this object, and throw a
        // 'std::bad_weak_ptr' exception if there is no 'shared_ptr' currently
        // managing this object.  If multiple groups of 'shared_ptr's are
        // managing this object, the returned 'shared_ptr' will share ownership
        // with the group that first managed this object.

    bsl::weak_ptr<ELEMENT_TYPE> weak_from_this() BSLS_CPP11_NOEXCEPT;
        // Return a 'weak_ptr' holding a weak reference to this managed object
        // if this object is currently managed by 'shared_ptr', and return an
        // expired 'weak_ptr' otherwise.  If multiple groups of 'shared_ptr's
        // are managing this object, the returned 'weak_ptr' will hold a weak
        // reference to the group that first managed this object.

    // ACCESSORS
    bsl::shared_ptr<const ELEMENT_TYPE> shared_from_this() const;
        // Return a 'shared_ptr<const ELEMENT_TYPE>' that shares ownership with
        // an existing 'shared_ptr' object that managed this object, and throw
        // a 'std::bad_weak_ptr' exception if there is no 'shared_ptr'
        // currently managing this object.  If multiple groups of 'shared_ptr's
        // are managing this object, the returned 'shared_ptr' will share
        // ownership with the group that first managed this object.


    bsl::weak_ptr<const ELEMENT_TYPE> weak_from_this() const
                                                           BSLS_CPP11_NOEXCEPT;
        // Return a 'weak_ptr' holding a weak reference (with only 'const'
        // access) to this managed object if this object is currently managed
        // by 'shared_ptr', and return an expired 'weak_ptr' otherwise.  If
        // multiple groups of 'shared_ptr's are managing this object, the
        // returned 'weak_ptr' will hold a weak reference to the group that
        // first managed this object.
};

// ASPECTS
template <class ELEMENT_TYPE>
void swap(weak_ptr<ELEMENT_TYPE>& a, weak_ptr<ELEMENT_TYPE>& b)
                                                           BSLS_CPP11_NOEXCEPT;
    // Efficiently exchange the states of the specified 'a' and 'b' weak
    // pointers such that each will refer to the object (if any) and
    // representation formerly referred to by the other.

                        // =========================
                        // class hash specialization
                        // =========================

// A partial specialization of 'bsl::hash' is no longer necessary, as the
// primary template has the correct behavior once 'hashAppend' is defined.

}  // close namespace bsl

namespace BloombergLP {
namespace bslstl {

                            // ====================
                            // struct SharedPtrUtil
                            // ====================

struct SharedPtrUtil {
    // This 'struct' provides a namespace for operations on shared pointers.

    // CLASS METHODS
    static
    bsl::shared_ptr<char>
    createInplaceUninitializedBuffer(size_t            bufferSize,
                                     bslma::Allocator *basicAllocator = 0);
        // Return a shared pointer with an in-place representation holding a
        // newly-created uninitialized buffer of the specified 'bufferSize' (in
        // bytes).  Optionally specify a 'basicAllocator' used to supply
        // memory.  If 'basicAllocator' is 0, the currently installed default
        // allocator is used.  The behavior is undefined unless
        // '0 < bufferSize'.

    // CASTING FUNCTIONS
    template <class TARGET, class SOURCE>
    static
    void constCast(bsl::shared_ptr<TARGET>        *target,
                   const bsl::shared_ptr<SOURCE>&  source);
        // Load into the specified 'target' an aliased shared pointer sharing
        // ownership of the object managed by the specified 'source' shared
        // pointer and referring to 'const_cast<TARGET *>(source.get())'.  If
        // '*target' is already managing a (possibly shared) object, then
        // release the shared reference to that object, and destroy it using
        // its associated deleter if that shared pointer held the last shared
        // reference to that object.  Note that a compiler diagnostic will be
        // emitted indicating an error unless
        // 'const_cast<TARGET *>(source.get())' is a valid expression.

    template <class TARGET, class SOURCE>
    static
    bsl::shared_ptr<TARGET> constCast(const bsl::shared_ptr<SOURCE>& source)
                                                           BSLS_CPP11_NOEXCEPT;
        // Return a 'bsl::shared_ptr<TARGET>' object sharing ownership of the
        // same object as the specified 'source' shared pointer to the
        // (template parameter) 'SOURCE' type, and referring to
        // 'const_cast<TARGET *>(source.get())'.  Note that a compiler
        // diagnostic will be emitted indicating an error unless
        // 'const_cast<TARGET *>(source.get())' is a valid expression.

    template <class TARGET, class SOURCE>
    static
    void dynamicCast(bsl::shared_ptr<TARGET>        *target,
                     const bsl::shared_ptr<SOURCE>&  source);
        // Load into the specified 'target' an aliased shared pointer sharing
        // ownership of the object managed by the specified 'source' shared
        // pointer and referring to 'dynamic_cast<TARGET *>(source.get())'.  If
        // '*target' is already managing a (possibly shared) object, then
        // release the shared reference to that object, and destroy it using
        // its associated deleter if that shared pointer held the last shared
        // reference to that object.  If
        // '0 == dynamic_cast<TARGET*>(source.get())', then '*target' shall be
        // reset to an empty state that does not refer to an object.  Note that
        // a compiler diagnostic will be emitted indicating an error unless
        // 'dynamic_cast<TARGET *>(source.get())' is a valid expression.

    template <class TARGET, class SOURCE>
    static
    bsl::shared_ptr<TARGET> dynamicCast(const bsl::shared_ptr<SOURCE>& source)
                                                           BSLS_CPP11_NOEXCEPT;
        // Return a 'bsl::shared_ptr<TARGET>' object sharing ownership of the
        // same object as the specified 'source' shared pointer to the
        // (template parameter) 'SOURCE' type, and referring to
        // 'dynamic_cast<TARGET *>(source.get())'.  If that would return a
        // shared pointer referring to nothing ('0 == get()'), then instead
        // return an (empty) default constructed shared pointer.  Note that a
        // compiler diagnostic will be emitted indicating an error unless
        // 'dynamic_cast<TARGET *>(source.get())' is a valid expression..

    template <class TARGET, class SOURCE>
    static
    void staticCast(bsl::shared_ptr<TARGET>        *target,
                    const bsl::shared_ptr<SOURCE>&  source);
        // Load into the specified 'target' an aliased shared pointer sharing
        // ownership of the object managed by the specified 'source' shared
        // pointer and referring to 'static_cast<TARGET *>(source.get())'.  If
        // '*target' is already managing a (possibly shared) object, then
        // release the shared reference to that object, and destroy it using
        // its associated deleter if that shared pointer held the last shared
        // reference to that object.  Note that a compiler diagnostic will be
        // emitted indicating an error unless
        // 'static_cast<TARGET *>(source.get())' is a valid expression.

    template <class TARGET, class SOURCE>
    static
    bsl::shared_ptr<TARGET> staticCast(const bsl::shared_ptr<SOURCE>& source)
                                                           BSLS_CPP11_NOEXCEPT;
        // Return a 'bsl::shared_ptr<TARGET>' object sharing ownership of the
        // same object as the specified 'source' shared pointer to the
        // (template parameter) 'SOURCE' type, and referring to
        // 'static_cast<TARGET *>(source.get())'.  Note that a compiler
        // diagnostic will be emitted indicating an error unless
        // 'static_cast<TARGET *>(source.get())' is a valid expression.
};

                        // ==========================
                        // struct SharedPtrNilDeleter
                        // ==========================

struct SharedPtrNilDeleter {
    // This 'struct' provides a function-like shared pointer deleter that does
    // nothing when invoked.

    // ACCESSORS
    void operator()(const volatile void *) const BSLS_CPP11_NOEXCEPT;
        // No-Op.
};

                        // ===============================
                        // struct SharedPtr_DefaultDeleter
                        // ===============================

struct SharedPtr_DefaultDeleter {
    // This 'struct' provides a function-like shared pointer deleter that
    // invokes 'delete' with the passed pointer.

    // ACCESSORS
    template <class ANY_TYPE>
    void operator()(ANY_TYPE *ptr) const BSLS_CPP11_NOEXCEPT;
        // Call 'delete' with the specified 'ptr'.
};

                        //=========================
                        // struct SharedPtr_ImpUtil
                        //=========================

struct SharedPtr_ImpUtil {
    // This 'struct' should be used by only 'shared_ptr' constructors. Its
    // purpose is to enable 'shared_ptr' constructors to determine if the
    // (template parameter) types 'COMPATIBLE_TYPE' or 'ELEMENT_TYPE' have a
    // specialization of 'enable_shared_from_this' as an unambiguous, publicly
    // accessible, base class.

    // CLASS METHODS
    template<class SHARED_TYPE, class ENABLE_TYPE>
    static void loadEnableSharedFromThis(
                   const bsl::enable_shared_from_this<ENABLE_TYPE> *result,
                   bsl::shared_ptr<SHARED_TYPE>                    *sharedPtr);
        // If the data member 'd_weakThis' of the specified 'result' has
        // expired, then set it to refer to the object managed by the specified
        // 'sharedPtr'; otherwise (i.e., 'd_weakThis' has not expired) this
        // method has no effect.  This function shall be called only by
        // 'shared_ptr' constructors creating shared pointers for classes that
        // derive publicly and unambiguously from a specialization of
        // 'enabled_shared_from_this'.  Note that overload resolution will
        // select the overload below if a supplied type does not derive from a
        // specialization of 'enable_shared_from_this'.

    static void loadEnableSharedFromThis(const volatile void *, const void *)
                                                           BSLS_CPP11_NOEXCEPT;
        // Do nothing.  This overload is selected, rather than the immediately
        // preceding template, when the 'SHARED_TYPE' template type parameter
        // of 'shared_ptr<SHARED_TYPE>' does not derive from a specialization
        // of 'enable_shared_from_this'.

    static void throwBadWeakPtr();
        // Throw a 'bsl::bad_weak_ptr' exception.

    template <class TYPE>
    static void *voidify(TYPE *address) BSLS_CPP11_NOEXCEPT;
        // Return the specified 'address' cast as a pointer to 'void', even if
        // (the template parameter) 'TYPE' is cv-qualified.

    template <class TYPE>
    static TYPE *unqualify(const volatile TYPE *address) BSLS_CPP11_NOEXCEPT;
        // Return the specified 'address' of a potentially 'cv'-qualified
        // object of the given (template parameter) 'TYPE', cast as a pointer
        // to a modifiable non-volatile object of the given 'TYPE'.
};

                        // ==========================
                        // class SharedPtr_RepProctor
                        // ==========================

class SharedPtr_RepProctor {
    // This 'class' implements a proctor that, unless its 'release' method has
    // previously been invoked, automatically releases a reference held by the
    // 'bslma::SharedPtrRep' object that is supplied at construction.

  private:
    // DATA
    bslma::SharedPtrRep *d_rep_p;    // Address of representation being managed

  private:
    // NOT IMPLEMENTED
    SharedPtr_RepProctor(const SharedPtr_RepProctor&);
    SharedPtr_RepProctor& operator=(const SharedPtr_RepProctor&);

  public:
    // CREATORS
    explicit SharedPtr_RepProctor(bslma::SharedPtrRep *rep)
                                                           BSLS_CPP11_NOEXCEPT;
        // Create a 'SharedPtr_RepProctor' that conditionally manages the
        // specified 'rep' (if non-zero).

    ~SharedPtr_RepProctor();
        // Destroy this 'SharedPtr_RepProctor', and dispose of (deallocate) the
        // 'bslma::SharedPtrRep' it manages (if any).  If no such object is
        // currently being managed, this method has no effect.  Note that the
        // destructor of the 'bslma::SharedPtrRep' will not be called as the
        // reference count will not be decremented.

    // MANIPULATORS
    void release() BSLS_CPP11_NOEXCEPT;
        // Release from management the object currently managed by this
        // proctor.  If no object is currently being managed, this method has
        // no effect.
};

}  // close package namespace
}  // close enterprise namespace

// ============================================================================
//                           INLINE DEFINITIONS
// ============================================================================

#if defined(BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS)
namespace BloombergLP {
namespace bslstl {

template <class FUNCTOR>
struct SharedPtr_TestIsCallable {
  private:
    // PRIVATE TYPES
    typedef BloombergLP::bslmf::Util Util;

    struct TrueType  { char d_padding; };
    struct FalseType { char d_padding[17]; };
        // The two structs 'TrueType' and 'FalseType' are guaranteed to have
        // distinct sizes, so that a 'sizeof(expression)' query, where
        // 'expression' returns one of these two types, will give different
        // answers depending on which type is returned.

  public:
    // CLASS METHODS
    template <class ARG>
    static FalseType test(...);
    template <class ARG>
    static TrueType test(typename bsl::enable_if<static_cast<bool>(sizeof(
                      ((void)Util::declval<FUNCTOR>()(Util::declval<ARG>())), 0
                                                                  ))>::type *);
        // This function is never defined.  It provides a property-checker that
        // an entity of (template parameter) type 'FACTORY' can be called like
        // a function with a single argument, which is a pointer to an object
        // of (template parameter) type 'ARG'.  The 'sizeof' expression
        // provides an unevaluated context to check the validity of the
        // enclosed expression, and the ', 0' ensures that the 'sizeof' check
        // remains valid, even if the expression returns 'void'.  Similarly,
        // the cast to 'void' ensures that there are no surprises with types
        // that overload the comma operator.

};

#if defined(BSLS_PLATFORM_CMP_MSVC)
// Microsoft needs a workaround to correctly handle calling through function
// pointers with incompatible types.

template <class RESULT, class PARAM>
struct SharedPtr_TestIsCallable<RESULT(PARAM)> {
  private:
    // PRIVATE TYPES
    typedef BloombergLP::bslmf::Util Util;

    struct TrueType  { char d_padding; };
    struct FalseType { char d_padding[17]; };

    // PRIVATE CLASS METHODS
    static RESULT callMe(PARAM);

  public:
    // CLASS METHODS
    template <class ARG>
    static FalseType test(...);
    template <class ARG>
    static TrueType test(typename bsl::enable_if<sizeof(
                                        ((void)callMe(Util::declval<ARG>())), 0
                                                       )>::type *);
        // This function is never defined.  It provides a property-checker that
        // an entity of (template parameter) type 'FACTORY' can be called like
        // a function with a single argument, which is a pointer to an object
        // of (template parameter) type 'ARG'.  The 'sizeof' expression
        // provides an unevaluated context to check the validity of the
        // enclosed expression, and the ', 0' ensures that the 'sizeof' check
        // remains valid, even if the expression returns 'void'.  Similarly,
        // the cast to 'void' ensures that there are no surprises with types
        // that overload the comma operator.
};

template <class RESULT, class PARAM>
struct SharedPtr_TestIsCallable<RESULT(*)(PARAM)>
    :  SharedPtr_TestIsCallable<RESULT(PARAM)> {
};

template <class RESULT, class PARAM>
struct SharedPtr_TestIsCallable<RESULT(&)(PARAM)>
    :  SharedPtr_TestIsCallable<RESULT(PARAM)> {
};

#if BSLS_PLATFORM_CMP_VERSION >= 1910
// MSVC 2017 expression-SFINAE has a regression that is failing in two
// additional cases:
//  1) for pointers to object types
//  2) where '0' is used for a null pointer literal, deducing as 'int'.
// We resolve those issues with a couple more specializations below.

template <class TYPE>
struct SharedPtr_TestIsCallable<TYPE *> {
    struct TrueType  { char d_padding; };
    struct FalseType { char d_padding[17]; };

    template <class ARG>
    static FalseType test(...);
};

template <>
struct SharedPtr_TestIsCallable<int> {
    struct TrueType  { char d_padding; };
    struct FalseType { char d_padding[17]; };

    template <class ARG>
    static FalseType test(...);
};
#endif  // MSVC 2017

#endif  // BSLS_PLATFORM_CMP_MSVC

template <class FUNCTOR, class ARG>
struct SharedPtr_IsCallable {
    enum { k_VALUE =
          sizeof(SharedPtr_TestIsCallable<FUNCTOR>::template test<ARG>(0)) == 1
    };
};


struct SharedPtr_IsFactoryFor_Impl {
  private:
    // PRIVATE TYPES
    struct TrueType  { char d_padding; };
    struct FalseType { char d_padding[17]; };

  public:
    // CLASS METHODS
    template <class FACTORY, class ARG>
    static FalseType test(...);
    template <class FACTORY, class ARG>
    static TrueType test(typename bsl::enable_if<static_cast<bool>(
                     sizeof(((void)(*(FACTORY *)0)->deleteObject((ARG *)0)), 0)
                                                                   )>::type *);
        // This function is never defined.  It provides a property-checker that
        // an object of (template parameter) type 'FACTORY' has a
        // member-function called 'deleteObject' that can be called with a
        // single argument, which is a pointer to an object of (template
        // parameter) type 'ARG'.  The 'sizeof' expression provides an
        // unevaluated context to check the validity of the enclosed
        // expression, and the ', 0' ensures that the 'sizeof' check remains
        // valid, even if the expression returns 'void'.  Similarly, the cast
        // to 'void' ensures that there are no surprises with types that
        // overload the comma operator.
};

template <class FACTORY, class ARG>
struct SharedPtr_IsFactoryFor {
    enum { k_VALUE =
                sizeof(SharedPtr_IsFactoryFor_Impl::test<FACTORY, ARG>(0)) == 1
    };
};


struct SharedPtr_IsNullableFactory_Impl {
  private:
    // PRIVATE TYPES
    struct TrueType  { char d_padding; };
    struct FalseType { char d_padding[17]; };

  public:
    // CLASS METHODS
    template <class FACTORY>
    static FalseType test(...);
    template <class FACTORY>
    static TrueType test(typename bsl::enable_if<static_cast<bool>(
                      sizeof(((void)(*(FACTORY *)0)->deleteObject(nullptr)), 0)
                                                                   )>::type *);
        // This function is never defined.  It provides a property-checker that
        // an object of (template parameter) type 'FACTORY' has a
        // member-function called 'deleteObject' that can be called with a
        // single argument, which is a pointer to an object of (template
        // parameter) type 'ARG'.  The 'sizeof' expression provides an
        // unevaluated context to check the validity of the enclosed
        // expression, and the ', 0' ensures that the 'sizeof' check remains
        // valid, even if the expression returns 'void'.  Similarly, the cast
        // to 'void' ensures that there are no surprises with types that
        // overload the comma operator.
};

template <class FACTORY>
struct SharedPtr_IsNullableFactory {
    enum { k_VALUE =
                sizeof(SharedPtr_IsNullableFactory_Impl::test<FACTORY>(0)) == 1
    };
};

}  // close package namespace
}  // close enterprise namespace
#endif  // BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS


namespace bsl {
                    //------------------------------
                    // class enable_shared_from_this
                    //------------------------------
// CREATORS
template<class ELEMENT_TYPE>
inline // constexpr
enable_shared_from_this<ELEMENT_TYPE>::enable_shared_from_this()
                                                            BSLS_CPP11_NOEXCEPT
: d_weakThis()
{
}

template<class ELEMENT_TYPE>
inline
enable_shared_from_this<ELEMENT_TYPE>::enable_shared_from_this(
                                                const enable_shared_from_this&)
                                                            BSLS_CPP11_NOEXCEPT
: d_weakThis()
{
}

template<class ELEMENT_TYPE>
inline
enable_shared_from_this<ELEMENT_TYPE>::~enable_shared_from_this()
{
}

// MANIPULATORS
template<class ELEMENT_TYPE>
inline
enable_shared_from_this<ELEMENT_TYPE>&
enable_shared_from_this<ELEMENT_TYPE>::operator=(
                                                const enable_shared_from_this&)
                                                            BSLS_CPP11_NOEXCEPT
{
    return *this;
}

template<class ELEMENT_TYPE>
inline
shared_ptr<ELEMENT_TYPE>
enable_shared_from_this<ELEMENT_TYPE>::shared_from_this()
{
    return shared_ptr<ELEMENT_TYPE>(d_weakThis);
}

template<class ELEMENT_TYPE>
inline
shared_ptr<const ELEMENT_TYPE>
enable_shared_from_this<ELEMENT_TYPE>::shared_from_this() const
{
    return shared_ptr<const ELEMENT_TYPE>(d_weakThis);
}

template<class ELEMENT_TYPE>
inline
weak_ptr<ELEMENT_TYPE>
enable_shared_from_this<ELEMENT_TYPE>::weak_from_this() BSLS_CPP11_NOEXCEPT
{
    return d_weakThis;
}

template<class ELEMENT_TYPE>
inline
weak_ptr<const ELEMENT_TYPE>
enable_shared_from_this<ELEMENT_TYPE>::weak_from_this() const
                                                            BSLS_CPP11_NOEXCEPT
{
    return d_weakThis;
}

                            // ----------------
                            // class shared_ptr
                            // ----------------

// PRIVATE CLASS METHODS
template <class ELEMENT_TYPE>
template <class INPLACE_REP>
inline
BloombergLP::bslma::SharedPtrRep *
shared_ptr<ELEMENT_TYPE>::makeInternalRep(
                                         ELEMENT_TYPE                     *,
                                         INPLACE_REP                      *,
                                         BloombergLP::bslma::SharedPtrRep *rep)
{
    return rep;
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class ALLOCATOR>
inline
BloombergLP::bslma::SharedPtrRep *
shared_ptr<ELEMENT_TYPE>::makeInternalRep(
                                      COMPATIBLE_TYPE               *ptr,
                                      ALLOCATOR                     *,
                                      BloombergLP::bslma::Allocator *allocator)
{
    typedef BloombergLP::bslma::SharedPtrOutofplaceRep<
                                               COMPATIBLE_TYPE,
                                               BloombergLP::bslma::Allocator *>
                                                                      RepMaker;

    return RepMaker::makeOutofplaceRep(ptr, allocator, allocator);
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class DELETER>
inline
BloombergLP::bslma::SharedPtrRep *
shared_ptr<ELEMENT_TYPE>::makeInternalRep(COMPATIBLE_TYPE *ptr,
                                          DELETER         *deleter,
                                          ...)
{
    typedef BloombergLP::bslma::SharedPtrOutofplaceRep<COMPATIBLE_TYPE,
                                                       DELETER *>     RepMaker;

    return RepMaker::makeOutofplaceRep(ptr, deleter, 0);
}

// CREATORS
template <class ELEMENT_TYPE>
inline
BSLS_CPP11_CONSTEXPR
shared_ptr<ELEMENT_TYPE>::shared_ptr() BSLS_CPP11_NOEXCEPT
: d_ptr_p(0)
, d_rep_p(0)
{
}

template <class ELEMENT_TYPE>
inline
BSLS_CPP11_CONSTEXPR
shared_ptr<ELEMENT_TYPE>::shared_ptr(bsl::nullptr_t) BSLS_CPP11_NOEXCEPT
: d_ptr_p(0)
, d_rep_p(0)
{
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(COMPATIBLE_TYPE *ptr)
: d_ptr_p(ptr)
{
    typedef BloombergLP::bslstl::SharedPtr_DefaultDeleter Deleter;
    typedef BloombergLP::bslma::SharedPtrOutofplaceRep<COMPATIBLE_TYPE,
                                                       Deleter>       RepMaker;

    d_rep_p = RepMaker::makeOutofplaceRep(ptr, Deleter(), 0);
    BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(ptr,
                                                                     this);
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                                 COMPATIBLE_TYPE               *ptr,
                                 BloombergLP::bslma::Allocator *basicAllocator)
: d_ptr_p(ptr)
{
    typedef BloombergLP::bslma::SharedPtrOutofplaceRep<
                                               COMPATIBLE_TYPE,
                                               BloombergLP::bslma::Allocator *>
                                                                      RepMaker;

    d_rep_p = RepMaker::makeOutofplaceRep(ptr, basicAllocator, basicAllocator);
    BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(ptr,
                                                                     this);
}

template <class ELEMENT_TYPE>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(ELEMENT_TYPE                     *ptr,
                                     BloombergLP::bslma::SharedPtrRep *rep)
: d_ptr_p(ptr)
, d_rep_p(rep)
{
    BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(ptr,
                                                                     this);
}

template <class ELEMENT_TYPE>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                       ELEMENT_TYPE                     *ptr,
                       BloombergLP::bslma::SharedPtrRep *rep,
                       BloombergLP::bslstl::SharedPtr_RepFromExistingSharedPtr)
: d_ptr_p(ptr)
, d_rep_p(rep)
{
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class DISPATCH
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE
          BSLSTL_SHAREDPTR_DEFINE_IF_DELETER(DISPATCH *, COMPATIBLE_TYPE)>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(COMPATIBLE_TYPE *ptr,
                                     DISPATCH        *dispatch)
: d_ptr_p(ptr)
, d_rep_p(makeInternalRep(ptr, dispatch, dispatch))
{
    BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(ptr,
                                                                     this);
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class DELETER
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE
          BSLSTL_SHAREDPTR_DEFINE_IF_DELETER(DELETER, COMPATIBLE_TYPE)>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                                 COMPATIBLE_TYPE               *ptr,
                                 DELETER                        deleter,
                                 BloombergLP::bslma::Allocator *basicAllocator)
: d_ptr_p(ptr)
{
    typedef BloombergLP::bslma::SharedPtrOutofplaceRep<COMPATIBLE_TYPE,
                                                       DELETER> RepMaker;

    d_rep_p = RepMaker::makeOutofplaceRep(ptr, deleter, basicAllocator);
    BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(ptr,
                                                                     this);
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class DELETER, class ALLOCATOR
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE
          BSLSTL_SHAREDPTR_DEFINE_IF_DELETER(DELETER, COMPATIBLE_TYPE)>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(COMPATIBLE_TYPE *ptr,
                                     DELETER          deleter,
                                     ALLOCATOR        basicAllocator,
                                     typename ALLOCATOR::value_type *)
: d_ptr_p(ptr)
{
#ifdef BSLS_PLATFORM_CMP_MSVC
    // This is not quite C++11 'decay' as we do not need to worry about array
    // types, and do not want to remove reference or cv-qualification from
    // DELETER otherwise.  This works around a Microsoft bug turning function
    // pointers into function references.

    typedef typename bsl::conditional<bsl::is_function<DELETER>::value,
                                      typename bsl::add_pointer<DELETER>::type,
                                      DELETER>::type DeleterType;
#else
    typedef DELETER DeleterType;
#endif

    typedef
    BloombergLP::bslstl::SharedPtrAllocateOutofplaceRep<COMPATIBLE_TYPE,
                                                        DeleterType,
                                                        ALLOCATOR> RepMaker;

    d_rep_p = RepMaker::makeOutofplaceRep(ptr, deleter, basicAllocator);
    BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(ptr,
                                                                     this);
}

template <class ELEMENT_TYPE>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(nullptr_t,
                                     BloombergLP::bslma::Allocator *)
: d_ptr_p(0)
, d_rep_p(0)
{
}

template <class ELEMENT_TYPE>
template <class DELETER
          BSLSTL_SHAREDPTR_DEFINE_IF_NULLPTR_DELETER(DELETER)>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                                 nullptr_t,
                                 DELETER                        deleter,
                                 BloombergLP::bslma::Allocator *basicAllocator)
: d_ptr_p(0)
{
    typedef BloombergLP::bslma::SharedPtrOutofplaceRep<ELEMENT_TYPE,
                                                       DELETER> RepMaker;

    if (bsl::is_convertible<DELETER, BloombergLP::bslma::Allocator *>::value &&
        bsl::is_pointer<DELETER>::value) {
        d_rep_p = 0;
    }
    else {
        d_rep_p = RepMaker::makeOutofplaceRep((ELEMENT_TYPE *)0,
                                              deleter,
                                              basicAllocator);
    }
}

template <class ELEMENT_TYPE>
template <class DELETER, class ALLOCATOR
          BSLSTL_SHAREDPTR_DEFINE_IF_NULLPTR_DELETER(DELETER)>
inline
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                                nullptr_t,
                                DELETER                         deleter,
                                ALLOCATOR                       basicAllocator,
                                typename ALLOCATOR::value_type *)
: d_ptr_p(0)
{
#ifdef BSLS_PLATFORM_CMP_MSVC
    // This is not quite C++11 'decay' as we do not need to worry about array
    // types, and do not want to remove reference or cv-qualification from
    // DELETER otherwise.  This works around a Microsoft bug turning function
    // pointers into function references.

    typedef typename bsl::conditional<bsl::is_function<DELETER>::value,
                                      typename bsl::add_pointer<DELETER>::type,
                                      DELETER>::type DeleterType;
#else
    typedef DELETER DeleterType;
#endif

    typedef
    BloombergLP::bslstl::SharedPtrAllocateOutofplaceRep<ELEMENT_TYPE,
                                                        DeleterType,
                                                        ALLOCATOR> RepMaker;

    d_rep_p = RepMaker::makeOutofplaceRep((ELEMENT_TYPE *)0,
                                          deleter,
                                          basicAllocator);
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(
               BloombergLP::bslma::ManagedPtr<COMPATIBLE_TYPE>  managedPtr,
               BloombergLP::bslma::Allocator                   *basicAllocator)
: d_ptr_p(managedPtr.ptr())
, d_rep_p(0)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<
                            BloombergLP::bslma::ManagedPtr<ELEMENT_TYPE> > Rep;

    if (d_ptr_p) {
        ELEMENT_TYPE *pPotentiallyShared = static_cast<ELEMENT_TYPE *>(
                                                managedPtr.deleter().object());

        if (&BloombergLP::bslma::SharedPtrRep::managedPtrDeleter ==
                                              managedPtr.deleter().deleter()) {
            d_rep_p = static_cast<BloombergLP::bslma::SharedPtrRep *>
                                       (managedPtr.release().second.factory());
        }
        else if (&BloombergLP::bslma::SharedPtrRep::managedPtrEmptyDeleter ==
                                              managedPtr.deleter().deleter()) {
            d_rep_p = 0;
            managedPtr.release();
        }
        else {
            basicAllocator =
                        BloombergLP::bslma::Default::allocator(basicAllocator);
            Rep *rep = new (*basicAllocator) Rep(basicAllocator);
            (*rep->ptr()) = managedPtr;
            d_rep_p = rep;
        }

        BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(
                                                            pPotentiallyShared,
                                                            this);
    }
}

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP98_AUTO_PTR)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                        native_std::auto_ptr<COMPATIBLE_TYPE>&  autoPtr,
                        BloombergLP::bslma::Allocator          *basicAllocator)
: d_ptr_p(autoPtr.get())
, d_rep_p(0)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<
                                   native_std::auto_ptr<COMPATIBLE_TYPE> > Rep;

    if (d_ptr_p) {
        basicAllocator =
                        BloombergLP::bslma::Default::allocator(basicAllocator);
        Rep *rep = new (*basicAllocator) Rep(basicAllocator);
        (*rep->ptr()) = autoPtr;
        d_rep_p = rep;
        BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(
                                                                       d_ptr_p,
                                                                       this);
    }
}

template <class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                        native_std::auto_ptr_ref<ELEMENT_TYPE>  autoRef,
                        BloombergLP::bslma::Allocator          *basicAllocator)
: d_ptr_p(0)
, d_rep_p(0)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<
                                      native_std::auto_ptr<ELEMENT_TYPE> > Rep;

    native_std::auto_ptr<ELEMENT_TYPE> autoPtr(autoRef);
    if (autoPtr.get()) {
        basicAllocator =
                        BloombergLP::bslma::Default::allocator(basicAllocator);
        Rep *rep = new (*basicAllocator) Rep(basicAllocator);
        d_ptr_p = autoPtr.get();
        (*rep->ptr()) = autoPtr;
        d_rep_p = rep;
    }
}
#endif

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP11_UNIQUE_PTR)
# if defined(BSLSTL_SHAREDPTR_SUPPORTS_SFINAE_CHECKS)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE,
          class UNIQUE_DELETER,
          typename enable_if<is_convertible<
                      typename native_std::unique_ptr<COMPATIBLE_TYPE,
                                                      UNIQUE_DELETER>::pointer,
                      ELEMENT_TYPE *>::value>::type *>
shared_ptr<ELEMENT_TYPE>::shared_ptr(
     native_std::unique_ptr<COMPATIBLE_TYPE, UNIQUE_DELETER>&&  adoptee,
     BloombergLP::bslma::Allocator                             *basicAllocator)
: d_ptr_p(adoptee.get())
, d_rep_p(0)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<
                 native_std::unique_ptr<COMPATIBLE_TYPE, UNIQUE_DELETER> > Rep;

    if (d_ptr_p) {
        basicAllocator =
                        BloombergLP::bslma::Default::allocator(basicAllocator);
        Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                            BloombergLP::bslmf::MovableRefUtil::move(adoptee));
        d_rep_p = rep;
        BloombergLP::bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(
                                                                       d_ptr_p,
                                                                       this);
    }
}
# endif
#endif

template <class ELEMENT_TYPE>
template <class ANY_TYPE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(const shared_ptr<ANY_TYPE>&  source,
                                     ELEMENT_TYPE                *object)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(object)
, d_rep_p(source.d_rep_p)
{
    if (d_rep_p) {
        d_rep_p->acquireRef();
    }
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
shared_ptr<ELEMENT_TYPE>::
shared_ptr(const shared_ptr<COMPATIBLE_TYPE>& other) BSLS_CPP11_NOEXCEPT
: d_ptr_p(other.d_ptr_p)
, d_rep_p(other.d_rep_p)
{
    if (d_rep_p) {
        d_rep_p->acquireRef();
    }
}

template <class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(const shared_ptr& original)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(original.d_ptr_p)
, d_rep_p(original.d_rep_p)
{
    if (d_rep_p) {
        d_rep_p->acquireRef();
    }
}

template <class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE>::shared_ptr
                          (BloombergLP::bslmf::MovableRef<shared_ptr> original)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(BloombergLP::bslmf::MovableRefUtil::access(original).d_ptr_p)
, d_rep_p(BloombergLP::bslmf::MovableRefUtil::access(original).d_rep_p)
{
    BloombergLP::bslmf::MovableRefUtil::access(original).d_ptr_p = 0;
    BloombergLP::bslmf::MovableRefUtil::access(original).d_rep_p = 0;
}

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(shared_ptr<COMPATIBLE_TYPE>&& other)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(other.d_ptr_p)
, d_rep_p(other.d_rep_p)
{
    other.d_ptr_p = 0;
    other.d_rep_p = 0;
}
#else
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
shared_ptr<ELEMENT_TYPE>::
shared_ptr(BloombergLP::bslmf::MovableRef<shared_ptr<COMPATIBLE_TYPE> > other)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(BloombergLP::bslmf::MovableRefUtil::access(other).d_ptr_p)
, d_rep_p(BloombergLP::bslmf::MovableRefUtil::access(other).d_rep_p)
{
    BloombergLP::bslmf::MovableRefUtil::access(other).d_ptr_p = 0;
    BloombergLP::bslmf::MovableRefUtil::access(other).d_rep_p = 0;
}
#endif

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(const weak_ptr<COMPATIBLE_TYPE>& other)
: d_ptr_p(0)
, d_rep_p(0)
{
    // This implementation handles two awkward cases:
    //
    // i) a ref-counted null pointer, means we cannot simply test 'if (!value)'
    // ii) a null pointer aliasing a non-null pointer is still expired, and so
    //     should throw.

    SelfType value = other.lock();
    if (other.expired()) {
        // Test after lock to avoid a race between testing 'expired' and
        // claiming the lock.

        BloombergLP::bslstl::SharedPtr_ImpUtil::throwBadWeakPtr();
    }

    swap(value);
}

#if !defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
shared_ptr<ELEMENT_TYPE>::shared_ptr(
                BloombergLP::bslmf::MovableRef<weak_ptr<COMPATIBLE_TYPE> > ptr)
: d_ptr_p(0)
, d_rep_p(0)
{
    // This implementation handles two awkward cases:
    //
    // i) a ref-counted null pointer, means we cannot simply test 'if (!value)'
    // ii) a null pointer aliasing a non-null pointer is still expired, and so
    //     should throw.

    weak_ptr<COMPATIBLE_TYPE>& other = ptr;

    SelfType value = other.lock();
    if (other.expired()) {
        // Test after lock to avoid a race between testing 'expired' and
        // claiming the lock.

        BloombergLP::bslstl::SharedPtr_ImpUtil::throwBadWeakPtr();
    }

    swap(value);
}
#endif

template <class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE>::~shared_ptr()
{
    if (d_rep_p) {
        d_rep_p->releaseRef();
    }
}

// MANIPULATORS
template <class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE>&
shared_ptr<ELEMENT_TYPE>::operator=(const shared_ptr& rhs) BSLS_CPP11_NOEXCEPT
{
    // Instead of testing '&rhs == this', which happens infrequently, optimize
    // for when reps are the same.

    if (rhs.d_rep_p == d_rep_p) {
        d_ptr_p = rhs.d_ptr_p;
    }
    else {
        SelfType(rhs).swap(*this);
    }

    return *this;
}

template <class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE>&
shared_ptr<ELEMENT_TYPE>::operator=(
                                BloombergLP::bslmf::MovableRef<shared_ptr> rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    // No self-assignment to optimize, postcondition demands 'rhs' is left
    // empty, unless it is the exact same object, not just the same 'rep'.

    shared_ptr(BloombergLP::bslmf::MovableRefUtil::move(rhs)).swap(*this);

    return *this;
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    shared_ptr<ELEMENT_TYPE>&>::type
shared_ptr<ELEMENT_TYPE>::operator=(const shared_ptr<COMPATIBLE_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    // Instead of testing '&rhs == this', which happens infrequently, optimize
    // for when reps are the same.

    if (rhs.d_rep_p == d_rep_p) {
        d_ptr_p = rhs.d_ptr_p;
    }
    else {
        SelfType(rhs).swap(*this);
    }

    return *this;
}

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    shared_ptr<ELEMENT_TYPE>&>::type
shared_ptr<ELEMENT_TYPE>::operator=(shared_ptr<COMPATIBLE_TYPE>&& rhs)
                                                            BSLS_CPP11_NOEXCEPT
#else
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    shared_ptr<ELEMENT_TYPE>&>::type
shared_ptr<ELEMENT_TYPE>::operator=(
              BloombergLP::bslmf::MovableRef<shared_ptr<COMPATIBLE_TYPE> > rhs)
                                                            BSLS_CPP11_NOEXCEPT
#endif
{
    // No self-assignment to optimize, postcondition demands 'rhs' is left
    // empty, unless it is the exact same object, not just the same 'rep'.

    shared_ptr(BloombergLP::bslmf::MovableRefUtil::move(rhs)).swap(*this);

    return *this;
}

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP98_AUTO_PTR)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
inline
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    shared_ptr<ELEMENT_TYPE>&>::type
shared_ptr<ELEMENT_TYPE>::operator=(native_std::auto_ptr<COMPATIBLE_TYPE> rhs)
{
    SelfType(rhs).swap(*this);
    return *this;
}
#endif

#if defined(BSLS_LIBRARYFEATURES_HAS_CPP11_UNIQUE_PTR)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class UNIQUE_DELETER>
inline
typename enable_if<
    is_convertible<
        typename
            native_std::unique_ptr<COMPATIBLE_TYPE, UNIQUE_DELETER>::pointer,
        ELEMENT_TYPE *>::value,
    shared_ptr<ELEMENT_TYPE>&>::type
shared_ptr<ELEMENT_TYPE>::operator=(
                 native_std::unique_ptr<COMPATIBLE_TYPE, UNIQUE_DELETER>&& rhs)
{
    SelfType(rhs).swap(*this);
    return *this;
}
#endif

template <class ELEMENT_TYPE>
inline
void shared_ptr<ELEMENT_TYPE>::reset() BSLS_CPP11_NOEXCEPT
{
    BloombergLP::bslma::SharedPtrRep *rep = d_rep_p;

    // Clear 'd_rep_p' first so that a self-referencing shared pointer's
    // destructor does not try to call 'releaseRef' again.

    d_rep_p = 0;
    d_ptr_p = 0;

    if (rep) {
        rep->releaseRef();
    }
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
inline
typename
      enable_if<is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value>::type
shared_ptr<ELEMENT_TYPE>::reset(COMPATIBLE_TYPE *ptr)
{
    SelfType(ptr).swap(*this);
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class DELETER>
inline
typename
      enable_if<is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value>::type
shared_ptr<ELEMENT_TYPE>::reset(COMPATIBLE_TYPE *ptr,
                                DELETER          deleter)
{
    SelfType(ptr, deleter).swap(*this);
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE, class DELETER, class ALLOCATOR>
inline
typename
      enable_if<is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value>::type
shared_ptr<ELEMENT_TYPE>::reset(COMPATIBLE_TYPE *ptr,
                                DELETER          deleter,
                                ALLOCATOR        basicAllocator)
{
    SelfType(ptr, deleter, basicAllocator).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ANY_TYPE>
inline
void shared_ptr<ELEMENT_TYPE>::reset(const shared_ptr<ANY_TYPE>&  source,
                                     ELEMENT_TYPE                *ptr)
{
    // Optimize for the (expected) common case where aliases are managing the
    // same data structure.

    if (source.d_rep_p == d_rep_p && ptr) {
        d_ptr_p = ptr;
    }
    else {
        SelfType(source, ptr).swap(*this);
    }
}

template <class ELEMENT_TYPE>
inline
void shared_ptr<ELEMENT_TYPE>::swap(shared_ptr& other) BSLS_CPP11_NOEXCEPT
{
    // We directly implement swapping of two pointers, rather than simply
    // calling 'bsl::swap' or using 'bslalg::SwapUtil', to avoid (indirectly)
    // including the platform <algorithm> header, which may transitively
    // include other standard headers.  This reduces the risk of
    // platform-specific cycles, which have been observed to cause problems.

    // Also, as 'shared_ptr' is bitwise-moveable, we could simplify this to
    // 'memcpy'-ing through an (aligned?) array of sufficient 'char'.

    ELEMENT_TYPE *tempPtr_p = d_ptr_p;
    d_ptr_p       = other.d_ptr_p;
    other.d_ptr_p = tempPtr_p;

    BloombergLP::bslma::SharedPtrRep *tempRep_p = d_rep_p;
    d_rep_p       = other.d_rep_p;
    other.d_rep_p = tempRep_p;
}

// ADDITIONAL BSL MANIPULATORS
template<class ELEMENT_TYPE>
void
shared_ptr<ELEMENT_TYPE>::createInplace()
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    BloombergLP::bslma::Allocator *basicAllocator =
                                      BloombergLP::bslma::Default::allocator();

    Rep *rep = new (*basicAllocator) Rep(basicAllocator);
    SelfType(rep->ptr(), rep).swap(*this);
}

#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES
template <class ELEMENT_TYPE>
template <class... ARGS>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                                 ARGS&&...                      args)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                                  BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    SelfType(rep->ptr(), rep).swap(*this);
}
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl --var-args=14 bslstl_sharedptr.h
template <class ELEMENT_TYPE>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator);
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07,
          class ARGS_08>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07,
          class ARGS_08,
          class ARGS_09>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07,
          class ARGS_08,
          class ARGS_09,
          class ARGS_10>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07,
          class ARGS_08,
          class ARGS_09,
          class ARGS_10,
          class ARGS_11>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07,
          class ARGS_08,
          class ARGS_09,
          class ARGS_10,
          class ARGS_11,
          class ARGS_12>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07,
          class ARGS_08,
          class ARGS_09,
          class ARGS_10,
          class ARGS_11,
          class ARGS_12,
          class ARGS_13>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13));
    SelfType(rep->ptr(), rep).swap(*this);
}

template <class ELEMENT_TYPE>
template <class ARGS_01,
          class ARGS_02,
          class ARGS_03,
          class ARGS_04,
          class ARGS_05,
          class ARGS_06,
          class ARGS_07,
          class ARGS_08,
          class ARGS_09,
          class ARGS_10,
          class ARGS_11,
          class ARGS_12,
          class ARGS_13,
          class ARGS_14>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_14,args_14));
    SelfType(rep->ptr(), rep).swap(*this);
}

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.
template <class ELEMENT_TYPE>
template <class... ARGS>
void
shared_ptr<ELEMENT_TYPE>::createInplace(
                                 BloombergLP::bslma::Allocator *basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args)
{
    typedef BloombergLP::bslma::SharedPtrInplaceRep<ELEMENT_TYPE> Rep;

    basicAllocator = BloombergLP::bslma::Default::allocator(basicAllocator);
    Rep *rep = new (*basicAllocator) Rep(basicAllocator,
                                  BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    SelfType(rep->ptr(), rep).swap(*this);
}
// }}} END GENERATED CODE
#endif

template <class ELEMENT_TYPE>
template <class ANY_TYPE>
void
shared_ptr<ELEMENT_TYPE>::loadAlias(const shared_ptr<ANY_TYPE>&  source,
                                    ELEMENT_TYPE                *object)
{
    if (source.d_rep_p == d_rep_p && object) {
        d_ptr_p = object;
    }
    else {
        SelfType(source, object).swap(*this);
    }
}

template <class ELEMENT_TYPE>
pair<ELEMENT_TYPE *, BloombergLP::bslma::SharedPtrRep *>
shared_ptr<ELEMENT_TYPE>::release() BSLS_CPP11_NOEXCEPT
{
    pair<ELEMENT_TYPE *, BloombergLP::bslma::SharedPtrRep *> ret(d_ptr_p,
                                                                 d_rep_p);
    d_ptr_p = 0;
    d_rep_p = 0;
    return ret;
}


// ACCESSORS
template <class ELEMENT_TYPE>
inline
# if defined(BSLS_PLATFORM_CMP_IBM)
shared_ptr<ELEMENT_TYPE>::operator typename shared_ptr::BoolType() const
                                                            BSLS_CPP11_NOEXCEPT
# else
shared_ptr<ELEMENT_TYPE>::operator BoolType() const BSLS_CPP11_NOEXCEPT
# endif
{
    return BloombergLP::bsls::UnspecifiedBool<shared_ptr>::makeValue(d_ptr_p);
}

template <class ELEMENT_TYPE>
inline
typename add_lvalue_reference<ELEMENT_TYPE>::type
shared_ptr<ELEMENT_TYPE>::operator*() const BSLS_CPP11_NOEXCEPT
{
    BSLS_ASSERT_SAFE(d_ptr_p);

    return *d_ptr_p;
}

template <class ELEMENT_TYPE>
inline
ELEMENT_TYPE *shared_ptr<ELEMENT_TYPE>::operator->() const BSLS_CPP11_NOEXCEPT
{
    return d_ptr_p;
}

template <class ELEMENT_TYPE>
inline
ELEMENT_TYPE *shared_ptr<ELEMENT_TYPE>::get() const BSLS_CPP11_NOEXCEPT
{
    return d_ptr_p;
}

template <class ELEMENT_TYPE>
template<class ANY_TYPE>
inline
bool shared_ptr<ELEMENT_TYPE>::owner_before(
                                       const shared_ptr<ANY_TYPE>& other) const
{
    return native_std::less<BloombergLP::bslma::SharedPtrRep *>()(rep(),
                                                                  other.rep());
}

template <class ELEMENT_TYPE>
template<class ANY_TYPE>
inline
bool
shared_ptr<ELEMENT_TYPE>::owner_before(const weak_ptr<ANY_TYPE>& other) const
{
    return native_std::less<BloombergLP::bslma::SharedPtrRep *>()(rep(),
                                                                  other.rep());
}

template <class ELEMENT_TYPE>
inline
bool shared_ptr<ELEMENT_TYPE>::unique() const BSLS_CPP11_NOEXCEPT
{
    return 1 == use_count();
}

template <class ELEMENT_TYPE>
inline
long shared_ptr<ELEMENT_TYPE>::use_count() const BSLS_CPP11_NOEXCEPT
{
    return d_rep_p ? d_rep_p->numReferences() : 0;
}

// ADDITIONAL BSL ACCESSORS
template <class ELEMENT_TYPE>
inline
typename add_lvalue_reference<ELEMENT_TYPE>::type
shared_ptr<ELEMENT_TYPE>::operator[](ptrdiff_t index) const
{
    BSLS_ASSERT_SAFE(d_ptr_p);

    return *(d_ptr_p + index);
}

template <class ELEMENT_TYPE>
BloombergLP::bslma::ManagedPtr<ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE>::managedPtr() const
{
    if (d_rep_p && d_ptr_p) {
        d_rep_p->acquireRef();
        return BloombergLP::bslma::ManagedPtr<ELEMENT_TYPE>(d_ptr_p,
                                                            d_rep_p,
                         &BloombergLP::bslma::SharedPtrRep::managedPtrDeleter);
                                                                      // RETURN
    }

    return BloombergLP::bslma::ManagedPtr<ELEMENT_TYPE>(
                     d_ptr_p,
                    (BloombergLP::bslma::SharedPtrRep *)0,
                    &BloombergLP::bslma::SharedPtrRep::managedPtrEmptyDeleter);
}

template <class ELEMENT_TYPE>
inline
BloombergLP::bslma::SharedPtrRep *shared_ptr<ELEMENT_TYPE>::rep() const
                                                            BSLS_CPP11_NOEXCEPT
{
    return d_rep_p;
}


                        // --------------
                        // class weak_ptr
                        // --------------

// CREATORS
template <class ELEMENT_TYPE>
inline
BSLS_CPP11_CONSTEXPR
weak_ptr<ELEMENT_TYPE>::weak_ptr() BSLS_CPP11_NOEXCEPT
: d_ptr_p(0)
, d_rep_p(0)
{
}

template <class ELEMENT_TYPE>
weak_ptr<ELEMENT_TYPE>::weak_ptr(
         BloombergLP::bslmf::MovableRef<weak_ptr> original) BSLS_CPP11_NOEXCEPT
: d_ptr_p(BloombergLP::bslmf::MovableRefUtil::access(original).d_ptr_p)
, d_rep_p(BloombergLP::bslmf::MovableRefUtil::access(original).d_rep_p)
{
    BloombergLP::bslmf::MovableRefUtil::access(original).d_rep_p = 0;
//    original.d_ptr_p = 0; // this seems overkill for a /weak/ pointer
}

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
weak_ptr<ELEMENT_TYPE>::weak_ptr(weak_ptr<COMPATIBLE_TYPE>&& original)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(original.d_ptr_p)
, d_rep_p(original.d_rep_p)
{
    original.d_rep_p = 0;
//    original.d_ptr_p = 0; // this seems overkill for a /weak/ pointer
}
#else
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
weak_ptr<ELEMENT_TYPE>::weak_ptr(
           BloombergLP::bslmf::MovableRef<weak_ptr<COMPATIBLE_TYPE> > original)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(original.d_ptr_p)
, d_rep_p(original.d_rep_p)
{
    original.d_rep_p = 0;
//    original.d_ptr_p = 0; // this seems overkill for a /weak/ pointer
}
#endif

template <class ELEMENT_TYPE>
weak_ptr<ELEMENT_TYPE>::weak_ptr(const weak_ptr<ELEMENT_TYPE>& original)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(original.d_ptr_p)
, d_rep_p(original.d_rep_p)
{
    if (d_rep_p) {
        d_rep_p->acquireWeakRef();
    }
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
weak_ptr<ELEMENT_TYPE>::weak_ptr(const shared_ptr<COMPATIBLE_TYPE>& other)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(other.get())
, d_rep_p(other.rep())
{
    if (d_rep_p) {
        d_rep_p->acquireWeakRef();
    }
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE
          BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE>
weak_ptr<ELEMENT_TYPE>::weak_ptr(const weak_ptr<COMPATIBLE_TYPE>& other)
                                                            BSLS_CPP11_NOEXCEPT
: d_ptr_p(other.d_ptr_p)
, d_rep_p(other.d_rep_p)
{
    if (d_rep_p) {
        d_rep_p->acquireWeakRef();
    }
}

template <class ELEMENT_TYPE>
inline
weak_ptr<ELEMENT_TYPE>::~weak_ptr()
{
    if (d_rep_p) {
        d_rep_p->releaseWeakRef();
    }
}

// PRIVATE MANIPULATORS
template <class ELEMENT_TYPE>
inline
void weak_ptr<ELEMENT_TYPE>::privateAssign(
                                      BloombergLP::bslma::SharedPtrRep *rep,
                                      ELEMENT_TYPE                     *target)
{
    if (d_rep_p) {
        d_rep_p->releaseWeakRef();
    }

    d_ptr_p = target;
    d_rep_p = rep;
    d_rep_p->acquireWeakRef();
}

// MANIPULATORS
template <class ELEMENT_TYPE>
weak_ptr<ELEMENT_TYPE>& weak_ptr<ELEMENT_TYPE>::operator=(
                                  BloombergLP::bslmf::MovableRef<weak_ptr> rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    weak_ptr tmp(BloombergLP::bslmf::MovableRefUtil::move(rhs));
    tmp.swap(*this);
    return *this;
}

#if defined(BSLMF_MOVABLEREF_USES_RVALUE_REFERENCES)
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    weak_ptr<ELEMENT_TYPE>&>::type
weak_ptr<ELEMENT_TYPE>::operator=(weak_ptr<COMPATIBLE_TYPE>&& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    weak_ptr tmp(BloombergLP::bslmf::MovableRefUtil::move(rhs));
    tmp.swap(*this);
    return *this;
}
#else
template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    weak_ptr<ELEMENT_TYPE>&>::type
weak_ptr<ELEMENT_TYPE>::operator=(
                BloombergLP::bslmf::MovableRef<weak_ptr<COMPATIBLE_TYPE> > rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    weak_ptr tmp(BloombergLP::bslmf::MovableRefUtil::move(rhs));
    tmp.swap(*this);
    return *this;
}
#endif

template <class ELEMENT_TYPE>
weak_ptr<ELEMENT_TYPE>& weak_ptr<ELEMENT_TYPE>::operator=(
                                             const weak_ptr<ELEMENT_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
#if 1
    weak_ptr tmp(rhs);
    tmp.swap(*this);
#else
    // needs friendship, or use the cheating util class.
    privateAssign(rhs.d_rep_p, rhs.d_ptr_p);
#endif
    return *this;
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    weak_ptr<ELEMENT_TYPE>&>::type
weak_ptr<ELEMENT_TYPE>::operator=(const shared_ptr<COMPATIBLE_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
#if 1
    weak_ptr tmp(rhs);
    tmp.swap(*this);
#else
    // needs friendship, or use the cheating util class.
    privateAssign(rhs.d_rep_p, rhs.d_ptr_p);
#endif
    return *this;
}

template <class ELEMENT_TYPE>
template <class COMPATIBLE_TYPE>
typename enable_if<
    is_convertible<COMPATIBLE_TYPE *, ELEMENT_TYPE *>::value,
    weak_ptr<ELEMENT_TYPE>&>::type
weak_ptr<ELEMENT_TYPE>::operator=(const weak_ptr<COMPATIBLE_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
#if 1
    weak_ptr tmp(rhs);
    tmp.swap(*this);
#else
    // needs friendship, or use the cheating util class.
    privateAssign(rhs.d_rep_p, rhs.d_ptr_p);
#endif
    return *this;
}

template <class ELEMENT_TYPE>
inline
void weak_ptr<ELEMENT_TYPE>::reset() BSLS_CPP11_NOEXCEPT
{
    if (d_rep_p) {
        d_rep_p->releaseWeakRef();
    }

    d_ptr_p = 0;
    d_rep_p = 0;
}

template <class ELEMENT_TYPE>
inline
void weak_ptr<ELEMENT_TYPE>::swap(weak_ptr<ELEMENT_TYPE>& other)
                                                            BSLS_CPP11_NOEXCEPT
{
    // We directly implement swapping of two pointers, rather than simply
    // calling 'bsl::swap' or using 'bslalg::SwapUtil', to avoid (indirectly)
    // including the platform <algorithm> header, which may transitively
    // include other standard headers.  This reduces the risk of
    // platform-specific cycles, which have been observed to cause problems.

    ELEMENT_TYPE *tempPtr_p = d_ptr_p;
    d_ptr_p       = other.d_ptr_p;
    other.d_ptr_p = tempPtr_p;

    BloombergLP::bslma::SharedPtrRep *tempRep_p = d_rep_p;
    d_rep_p       = other.d_rep_p;
    other.d_rep_p = tempRep_p;
}

// ACCESSORS
template <class ELEMENT_TYPE>
inline
bool weak_ptr<ELEMENT_TYPE>::expired() const BSLS_CPP11_NOEXCEPT
{
    return !(d_rep_p && d_rep_p->numReferences());
}

template <class ELEMENT_TYPE>
shared_ptr<ELEMENT_TYPE> weak_ptr<ELEMENT_TYPE>::lock() const
                                                            BSLS_CPP11_NOEXCEPT
{
    if (d_rep_p && d_rep_p->tryAcquireRef()) {
        return shared_ptr<ELEMENT_TYPE>(
                    d_ptr_p,
                    d_rep_p,
                    BloombergLP::bslstl::SharedPtr_RepFromExistingSharedPtr());
                                                                      // RETURN
    }
    return shared_ptr<ELEMENT_TYPE>();
}

template <class ELEMENT_TYPE>
template <class ANY_TYPE>
inline
bool
weak_ptr<ELEMENT_TYPE>::owner_before(const shared_ptr<ANY_TYPE>& other) const
{
    return native_std::less<BloombergLP::bslma::SharedPtrRep *>()(d_rep_p,
                                                                  other.rep());
}

template <class ELEMENT_TYPE>
template <class ANY_TYPE>
inline
bool
weak_ptr<ELEMENT_TYPE>::owner_before(const weak_ptr<ANY_TYPE>& other) const
{
    return native_std::less<BloombergLP::bslma::SharedPtrRep *>()(
                                                                d_rep_p,
                                                                other.d_rep_p);
}

template <class ELEMENT_TYPE>
inline
BloombergLP::bslma::SharedPtrRep *weak_ptr<ELEMENT_TYPE>::rep() const
                                                            BSLS_CPP11_NOEXCEPT
{
    return d_rep_p;
}

template <class ELEMENT_TYPE>
inline
long weak_ptr<ELEMENT_TYPE>::use_count() const BSLS_CPP11_NOEXCEPT
{
    return d_rep_p ? d_rep_p->numReferences() : 0;
}


}  // close namespace bsl

namespace BloombergLP {
namespace bslstl {

                            // -----------------
                            // SharedPtr_ImpUtil
                            // -----------------

template <class SHARED_TYPE, class ENABLE_TYPE>
inline
void SharedPtr_ImpUtil::loadEnableSharedFromThis(
                    const bsl::enable_shared_from_this<ENABLE_TYPE> *result,
                    bsl::shared_ptr<SHARED_TYPE>                    *sharedPtr)
{
    BSLS_ASSERT(0 != sharedPtr);
    BSLS_ASSERT(0 != result);

    if (result->d_weakThis.expired()) {
        result->d_weakThis.privateAssign(
                         sharedPtr->d_rep_p,
                         const_cast <ENABLE_TYPE      *>(
                         static_cast<ENABLE_TYPE const*>(sharedPtr->d_ptr_p)));
    }
}

inline
void bslstl::SharedPtr_ImpUtil::loadEnableSharedFromThis(const volatile void *,
                                                         const void *)
                                                            BSLS_CPP11_NOEXCEPT
{
}

template <class TYPE>
inline
TYPE *SharedPtr_ImpUtil::unqualify(const volatile TYPE *address)
                                                            BSLS_CPP11_NOEXCEPT
{
    return const_cast<typename bsl::remove_cv<TYPE>::type *>(address);
}

template <class TYPE>
inline
void *SharedPtr_ImpUtil::voidify(TYPE *address) BSLS_CPP11_NOEXCEPT
{
    return static_cast<void *>(
            const_cast<typename bsl::remove_cv<TYPE>::type *>(address));
}

                            // --------------------
                            // struct SharedPtrUtil
                            // --------------------

// CLASS METHODS
template <class TARGET, class SOURCE>
inline
void SharedPtrUtil::constCast(bsl::shared_ptr<TARGET>        *target,
                              const bsl::shared_ptr<SOURCE>&  source)
{
    BSLS_ASSERT(0 != target);

    target->reset(source, const_cast<TARGET *>(source.get()));
}

template <class TARGET, class SOURCE>
inline
bsl::shared_ptr<TARGET>
SharedPtrUtil::constCast(const bsl::shared_ptr<SOURCE>& source)
                                                            BSLS_CPP11_NOEXCEPT
{
    return bsl::shared_ptr<TARGET>(source,
                                   const_cast<TARGET *>(source.get()));
}

template <class TARGET, class SOURCE>
inline
void SharedPtrUtil::dynamicCast(bsl::shared_ptr<TARGET>        *target,
                                const bsl::shared_ptr<SOURCE>&  source)
{
    BSLS_ASSERT(0 != target);

    if (TARGET *castPtr = dynamic_cast<TARGET *>(source.get())) {
        target->reset(source, castPtr);
    }
    else {
        target->reset();
    }
}

template <class TARGET, class SOURCE>
inline
bsl::shared_ptr<TARGET>
SharedPtrUtil::dynamicCast(const bsl::shared_ptr<SOURCE>& source)
                                                            BSLS_CPP11_NOEXCEPT
{
    if (TARGET *castPtr = dynamic_cast<TARGET *>(source.get())) {
        return bsl::shared_ptr<TARGET>(source, castPtr);              // RETURN
    }

    return bsl::shared_ptr<TARGET>();
}

template <class TARGET, class SOURCE>
inline
void SharedPtrUtil::staticCast(bsl::shared_ptr<TARGET>        *target,
                               const bsl::shared_ptr<SOURCE>&  source)
{
    BSLS_ASSERT(0 != target);

    target->reset(source, static_cast<TARGET *>(source.get()));
}

template <class TARGET, class SOURCE>
inline
bsl::shared_ptr<TARGET>
SharedPtrUtil::staticCast(const bsl::shared_ptr<SOURCE>& source)
                                                            BSLS_CPP11_NOEXCEPT
{
    return bsl::shared_ptr<TARGET>(source,
                                   static_cast<TARGET *>(source.get()));
}

                        // --------------------------
                        // struct SharedPtrNilDeleter
                        // --------------------------

// ACCESSORS
inline
void SharedPtrNilDeleter::operator()(const volatile void *) const
                                                            BSLS_CPP11_NOEXCEPT
{
}

                        // -------------------------------
                        // struct SharedPtr_DefaultDeleter
                        // -------------------------------

// ACCESSORS
template <class ANY_TYPE>
inline
void SharedPtr_DefaultDeleter::operator()(ANY_TYPE *ptr) const
                                                            BSLS_CPP11_NOEXCEPT
{
    delete ptr;
}

                        // --------------------------
                        // class SharedPtr_RepProctor
                        // --------------------------

// CREATORS
inline
SharedPtr_RepProctor::SharedPtr_RepProctor(bslma::SharedPtrRep *rep)
                                                            BSLS_CPP11_NOEXCEPT
: d_rep_p(rep)
{
}

inline
SharedPtr_RepProctor::~SharedPtr_RepProctor()
{
    if (d_rep_p) {
        d_rep_p->disposeRep();
    }
}

// MANIPULATORS
inline
void SharedPtr_RepProctor::release() BSLS_CPP11_NOEXCEPT
{
    d_rep_p = 0;
}

}  // close package namespace
}  // close enterprise namespace

// FREE OPERATORS
template <class LHS_TYPE, class RHS_TYPE>
inline
bool bsl::operator==(const shared_ptr<LHS_TYPE>& lhs,
                     const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT
{
    return lhs.get() == rhs.get();
}

template <class LHS_TYPE, class RHS_TYPE>
inline
bool bsl::operator!=(const shared_ptr<LHS_TYPE>& lhs,
                     const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT
{
    return !(lhs == rhs);
}

template <class LHS_TYPE, class RHS_TYPE>
inline
bool bsl::operator<(const shared_ptr<LHS_TYPE>& lhs,
                    const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT
{
    return native_std::less<const void *>()(lhs.get(), rhs.get());
}

template <class LHS_TYPE, class RHS_TYPE>
inline
bool bsl::operator>(const shared_ptr<LHS_TYPE>& lhs,
                    const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT
{
    return rhs < lhs;
}

template <class LHS_TYPE, class RHS_TYPE>
inline
bool bsl::operator<=(const shared_ptr<LHS_TYPE>& lhs,
                     const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT
{
    return !(rhs < lhs);
}

template <class LHS_TYPE, class RHS_TYPE>
inline
bool bsl::operator>=(const shared_ptr<LHS_TYPE>& lhs,
                     const shared_ptr<RHS_TYPE>& rhs) BSLS_CPP11_NOEXCEPT
{
    return !(lhs < rhs);
}

template <class LHS_TYPE>
inline
bool bsl::operator==(const shared_ptr<LHS_TYPE>& lhs, bsl::nullptr_t)
                                                            BSLS_CPP11_NOEXCEPT
{
    return !lhs;
}

template <class RHS_TYPE>
inline
bool bsl::operator==(bsl::nullptr_t, const shared_ptr<RHS_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    return !rhs;
}

template <class LHS_TYPE>
inline
bool bsl::operator!=(const shared_ptr<LHS_TYPE>& lhs, bsl::nullptr_t)
                                                            BSLS_CPP11_NOEXCEPT
{
    return static_cast<bool>(lhs);
}

template <class RHS_TYPE>
inline
bool bsl::operator!=(bsl::nullptr_t, const shared_ptr<RHS_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    return static_cast<bool>(rhs);
}

template <class LHS_TYPE>
inline
bool bsl::operator<(const shared_ptr<LHS_TYPE>& lhs, bsl::nullptr_t)
                                                            BSLS_CPP11_NOEXCEPT
{
    return native_std::less<LHS_TYPE *>()(lhs.get(), 0);
}

template <class RHS_TYPE>
inline
bool bsl::operator<(bsl::nullptr_t, const shared_ptr<RHS_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    return native_std::less<RHS_TYPE *>()(0, rhs.get());
}

template <class LHS_TYPE>
inline
bool bsl::operator<=(const shared_ptr<LHS_TYPE>& lhs, bsl::nullptr_t)
                                                            BSLS_CPP11_NOEXCEPT
{
    return !native_std::less<LHS_TYPE *>()(0, lhs.get());
}

template <class RHS_TYPE>
inline
bool bsl::operator<=(bsl::nullptr_t, const shared_ptr<RHS_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    return !native_std::less<RHS_TYPE *>()(rhs.get(), 0);
}

template <class LHS_TYPE>
inline
bool bsl::operator>(const shared_ptr<LHS_TYPE>& lhs, bsl::nullptr_t)
                                                            BSLS_CPP11_NOEXCEPT
{
    return native_std::less<LHS_TYPE *>()(0, lhs.get());
}

template <class RHS_TYPE>
inline
bool bsl::operator>(bsl::nullptr_t, const shared_ptr<RHS_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    return native_std::less<RHS_TYPE *>()(rhs.get(), 0);
}

template <class LHS_TYPE>
inline
bool bsl::operator>=(const shared_ptr<LHS_TYPE>& lhs, bsl::nullptr_t)
                                                            BSLS_CPP11_NOEXCEPT
{
    return !native_std::less<LHS_TYPE *>()(lhs.get(), 0);
}

template <class RHS_TYPE>
inline
bool bsl::operator>=(bsl::nullptr_t, const shared_ptr<RHS_TYPE>& rhs)
                                                            BSLS_CPP11_NOEXCEPT
{
    return !native_std::less<RHS_TYPE *>()(0, rhs.get());
}

template <class CHAR_TYPE, class CHAR_TRAITS, class ELEMENT_TYPE>
inline
native_std::basic_ostream<CHAR_TYPE, CHAR_TRAITS>&
bsl::operator<<(native_std::basic_ostream<CHAR_TYPE, CHAR_TRAITS>& stream,
                const shared_ptr<ELEMENT_TYPE>&                    rhs)
{
    return stream << rhs.get();
}

// ASPECTS
template <class HASHALG, class ELEMENT_TYPE>
inline
void bsl::hashAppend(HASHALG& hashAlg, const shared_ptr<ELEMENT_TYPE>& input)
{
    hashAppend(hashAlg, input.get());
}

template <class ELEMENT_TYPE>
inline
void bsl::swap(shared_ptr<ELEMENT_TYPE>& a, shared_ptr<ELEMENT_TYPE>& b)
                                                            BSLS_CPP11_NOEXCEPT
{
    a.swap(b);
}

template <class ELEMENT_TYPE>
inline
void bsl::swap(weak_ptr<ELEMENT_TYPE>& a, weak_ptr<ELEMENT_TYPE>& b)
                                                            BSLS_CPP11_NOEXCEPT
{
    a.swap(b);
}

// STANDARD FREE FUNCTIONS
template<class DELETER, class ELEMENT_TYPE>
inline
DELETER *bsl::get_deleter(const shared_ptr<ELEMENT_TYPE>& p)
                                                            BSLS_CPP11_NOEXCEPT
{
    BloombergLP::bslma::SharedPtrRep *rep = p.rep();
    return rep ? static_cast<DELETER *>(rep->getDeleter(typeid(DELETER))) : 0;
}

// STANDARD CAST FUNCTIONS
template<class TO_TYPE, class FROM_TYPE>
inline
bsl::shared_ptr<TO_TYPE>
bsl::const_pointer_cast(const shared_ptr<FROM_TYPE>& source)
                                                            BSLS_CPP11_NOEXCEPT
{
    return shared_ptr<TO_TYPE>(source, const_cast<TO_TYPE *>(source.get()));
}

template<class TO_TYPE, class FROM_TYPE>
inline
bsl::shared_ptr<TO_TYPE>
bsl::dynamic_pointer_cast(const shared_ptr<FROM_TYPE>& source)
                                                            BSLS_CPP11_NOEXCEPT
{
    if (TO_TYPE *castPtr = dynamic_cast<TO_TYPE *>(source.get())) {
        return shared_ptr<TO_TYPE>(source, castPtr);                  // RETURN
    }

    return shared_ptr<TO_TYPE>();
}

template<class TO_TYPE, class FROM_TYPE>
inline
bsl::shared_ptr<TO_TYPE>
bsl::static_pointer_cast(const shared_ptr<FROM_TYPE>& source)
                                                            BSLS_CPP11_NOEXCEPT
{
    return shared_ptr<TO_TYPE>(source, static_cast<TO_TYPE *>(source.get()));
}

// STANDARD FACTORY FUNCTIONS
#if !BSLS_COMPILERFEATURES_SIMULATE_CPP11_FEATURES // $var-args=14

// C++11, PERFECT FORWARDING THROUGH A VARIADIC TEMPLATE

template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator, ARGS&&... args)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                                  BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                                                   ARGS&&...  args)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType  alloc(basicAllocator);
    Rep           *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

// 'make_shared' using the default allocator

template<class ELEMENT_TYPE, class... ARGS>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(ARGS&&... args)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType  basicAllocator;
    Rep           *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}
#elif BSLS_COMPILERFEATURES_SIMULATE_VARIADIC_TEMPLATES
// {{{ BEGIN GENERATED CODE
// The following section is automatically generated.  **DO NOT EDIT**
// Generator command line: sim_cpp11_features.pl --var-args=14 bslstl_sharedptr.h


template<class ELEMENT_TYPE, class ALLOC>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13,
                                          class ARGS_14>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_14,args_14));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}


template<class ELEMENT_TYPE, class ALLOC>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class ARGS_01,
                                          class ARGS_02,
                                          class ARGS_03,
                                          class ARGS_04,
                                          class ARGS_05,
                                          class ARGS_06,
                                          class ARGS_07,
                                          class ARGS_08,
                                          class ARGS_09,
                                          class ARGS_10,
                                          class ARGS_11,
                                          class ARGS_12,
                                          class ARGS_13,
                                          class ARGS_14>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_14,args_14));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}



template<class ELEMENT_TYPE>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                               )
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11,
                             class ARGS_12>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11,
                             class ARGS_12,
                             class ARGS_13>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ARGS_01,
                             class ARGS_02,
                             class ARGS_03,
                             class ARGS_04,
                             class ARGS_05,
                             class ARGS_06,
                             class ARGS_07,
                             class ARGS_08,
                             class ARGS_09,
                             class ARGS_10,
                             class ARGS_11,
                             class ARGS_12,
                             class ARGS_13,
                             class ARGS_14>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_01) args_01,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_02) args_02,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_03) args_03,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_04) args_04,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_05) args_05,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_06) args_06,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_07) args_07,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_08) args_08,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_09) args_09,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_10) args_10,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_11) args_11,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_12) args_12,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_13) args_13,
                            BSLS_COMPILERFEATURES_FORWARD_REF(ARGS_14) args_14)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_01,args_01),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_02,args_02),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_03,args_03),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_04,args_04),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_05,args_05),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_06,args_06),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_07,args_07),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_08,args_08),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_09,args_09),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_10,args_10),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_11,args_11),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_12,args_12),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_13,args_13),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS_14,args_14));
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

#else
// The generated code below is a workaround for the absence of perfect
// forwarding in some compilers.


template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
# if defined(BSLSTL_SHAREDPTR_NO_PARTIAL_ORDER_ON_ALLOCATOR_POINTER)
typename bsl::enable_if<!bsl::is_pointer<ALLOC>::value,
                         bsl::shared_ptr<ELEMENT_TYPE> >::type
# else
bsl::shared_ptr<ELEMENT_TYPE>
# endif
bsl::allocate_shared(ALLOC basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<ELEMENT_TYPE,
                                                             ALLOC> Rep;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    bsl::allocator_traits<ALLOC>::construct(
                                  basicAllocator,
                                  ImpUtil::unqualify(rep_p->ptr()),
                                  BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}

template<class ELEMENT_TYPE, class ALLOC, class... ARGS>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::allocate_shared(ALLOC     *basicAllocator,
                               BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;
    AllocatorType alloc(basicAllocator);
    Rep *rep_p = Rep::makeRep(alloc);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(alloc,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}


template<class ELEMENT_TYPE, class... ARGS>
inline
bsl::shared_ptr<ELEMENT_TYPE> bsl::make_shared(
                               BSLS_COMPILERFEATURES_FORWARD_REF(ARGS)... args)
{
    typedef BloombergLP::bslstl::SharedPtr_ImpUtil ImpUtil;
    typedef bsl::allocator<char>                   AllocatorType;
    typedef bsl::allocator_traits<AllocatorType>   AllocatorTraits;

    typedef BloombergLP::bslstl::SharedPtrAllocateInplaceRep<
                                                            ELEMENT_TYPE,
                                                            AllocatorType> Rep;

    AllocatorType basicAllocator;
    Rep *rep_p = Rep::makeRep(basicAllocator);

    BloombergLP::bslstl::SharedPtr_RepProctor proctor(rep_p);
    AllocatorTraits::construct(basicAllocator,
                               ImpUtil::unqualify(rep_p->ptr()),
                               BSLS_COMPILERFEATURES_FORWARD(ARGS,args)...);
    proctor.release();
    return shared_ptr<ELEMENT_TYPE>(rep_p->ptr(), rep_p);
}
// }}} END GENERATED CODE
#endif

// ============================================================================
//                              TYPE TRAITS
// ============================================================================

// Type traits for smart pointers:
//: o 'shared_ptr' has pointer semantics, but 'weak_ptr' does not.
//:
//: o Although 'shared_ptr' constructs with an allocator, it does not 'use' an
//:   allocator in the manner of the 'UsesBslmaAllocator' trait, and should be
//:   explicitly specialized as a clear sign to code inspection tools.
//:
//: o Smart pointers are bitwise-movable as long as there is no opportunity for
//:   holding a pointer to internal state in the immediate object itself.  As
//:   'd_ptr_p' is never exposed by reference, it is not possible to create an
//:   internal pointer, so the trait should be 'true'.

namespace BloombergLP {

namespace bslma {

template <class ELEMENT_TYPE>
struct UsesBslmaAllocator< ::bsl::shared_ptr<ELEMENT_TYPE> >
    : bsl::false_type
{};

}  // close namespace bslma

namespace bslmf {

template <class ELEMENT_TYPE>
struct HasPointerSemantics< ::bsl::shared_ptr<ELEMENT_TYPE> >
    : bsl::true_type
{};

template <class ELEMENT_TYPE>
struct IsBitwiseMoveable< ::bsl::shared_ptr<ELEMENT_TYPE> >
    : bsl::true_type
{};

template <class ELEMENT_TYPE>
struct IsBitwiseMoveable< ::bsl::weak_ptr<ELEMENT_TYPE> >
    : bsl::true_type
{};

}  // close namespace bslmf

}  // close enterprise namespace

#if defined(BSLS_PLATFORM_CMP_GNU) || defined(BSLS_PLATFORM_CMP_CLANG)
# pragma GCC diagnostic pop
#endif

#undef BSLSTL_SHAREDPTR_DECLARE_IF_COMPATIBLE
#undef BSLSTL_SHAREDPTR_DEFINE_IF_COMPATIBLE

#undef BSLSTL_SHAREDPTR_DECLARE_IF_DELETER
#undef BSLSTL_SHAREDPTR_DEFINE_IF_DELETER

#undef BSLSTL_SHAREDPTR_DECLARE_IF_NULLPTR_DELETER
#undef BSLSTL_SHAREDPTR_DEFINE_IF_NULLPTR_DELETER

#endif

// ----------------------------------------------------------------------------
// Copyright 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
