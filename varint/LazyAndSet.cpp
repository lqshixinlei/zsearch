#include <vector>
#include "Set.h"
#include "LazyAndSet.h"


LazyAndSet::LazyAndSet()
{
	sets_ = vector<shared_ptr<Set> >();
	nonNullSize = 0;
	setSize = 0;
    init = false;
}

LazyAndSet::LazyAndSet(vector<shared_ptr<Set> >& sets)
{
	sets_ = sets;
	nonNullSize = sets.size();
	setSize = 0;
	init = false;
}

inline bool LazyAndSet::find(unsigned int val) const 
{ 
	LazyAndSetIterator finder(this);
	unsigned docid = finder.Advance(val);
	return docid != NO_MORE_DOCS && docid == val;
}

unsigned int LazyAndSet::size() const
{
        if (nonNullSize == 0){
          return 0;
	}
	// Do the size if we haven't done it so far.
	if(!init) {
	  LazyAndSetIterator dcit(this);
	  setSize = 0;
	  while(dcit.nextDoc() != NO_MORE_DOCS)
		 setSize++;
	}
	init = true;
	return setSize;
}

shared_ptr<Set::Iterator> LazyAndSet::iterator() const 
{
	shared_ptr<Set::Iterator> it(new LazyAndSetIterator(this));
	return it;
}	

LazyAndSetIterator::LazyAndSetIterator(const LazyAndSet* parent) : set(*parent){
	if (set.nonNullSize == 0){
		lastReturn = NO_MORE_DOCS;
		return;
	}
	    	
	for (vector<shared_ptr<Set> >::const_iterator it = set.sets_.begin(); it!=set.sets_.end(); it++){
		shared_ptr<Set> set  = *it;
		shared_ptr<Set::Iterator>  dcit = set->iterator();
		iterators.push_back(dcit);
	}
	lastReturn = 0;
}

unsigned int  LazyAndSetIterator::docID() {
    return lastReturn;
}

unsigned int LazyAndSetIterator::nextDoc() {
    // DAAT
    if (lastReturn == NO_MORE_DOCS) 
       return NO_MORE_DOCS;    
    
    shared_ptr<Set::Iterator> dcit = iterators[0];
    unsigned target = dcit->nextDoc();

    // shortcut: if it reaches the end of the shortest list, do not scan other lists
    if(target == NO_MORE_DOCS) { 
        return (lastReturn = target);
    }

    int size = iterators.size();
    int skip = 0;
    int i = 1;
   
    // i is ith iterator
    while (i < size) {
      if (i != skip) {
        dcit = iterators[i];
        unsigned int docId = dcit->Advance(target);
       
        // once we reach the end of one of the blocks, we return NO_MORE_DOCS
        if(docId == NO_MORE_DOCS) {
          return (lastReturn = docId);
        }
    
        if (docId > target) { //  cannot find the target in the next list
          target = docId;
          if(i != 0) {
            skip = i;
            i = 0;
            continue;
          } else { // for the first list, it must succeed as long as the docId is not NO_MORE_DOCS
            skip = 0;
          }
        }

      }
      i++;
    }
   
    return (lastReturn = target);
}

unsigned int LazyAndSetIterator::Advance(unsigned int  target) {
     if (lastReturn == NO_MORE_DOCS) 
        return NO_MORE_DOCS;
    
      shared_ptr<Set::Iterator> dcit = iterators[0];
     target = dcit->Advance(target);
     if(target == NO_MORE_DOCS) { 
       return (lastReturn = target);
     }
     
     int size = iterators.size();
     int skip = 0;
     int i = 1;
     while (i < size) {
       if (i != skip) {
         dcit = iterators[i];
         unsigned int docId = dcit->Advance(target);
         if(docId == NO_MORE_DOCS) {
           return (lastReturn = docId);
         }
         if (docId > target) {
           target = docId;
           if(i != 0) {
             skip = i;
             i = 0;
             continue;
           } else {
             skip = 0;
           }
         }
       }
       i++;
     }
     return (lastReturn = target);
}
