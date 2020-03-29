# induced-subtree-enumerator

This program, and its various branches, aim to help in answering the question of: "What is the largest Applied Energistics 2 controller that can be built?". Thus far, the main focus is to improve the known lower bounds emperically by finding better and better controllers.
A promising set of controllers, when represented as a graph, form trees, and most of the algorithms currently here aim to find the best trees possible.

These programs were programmed and tested on Ubuntu, though I do support MacOS as well.

## Key Algorithms

### Exhaustive Enumeration

This algorithm (in master), based on the paper at https://pdfs.semanticscholar.org/9631/ef7c303b64c90797eabc26cbfcd11dcc4507.pdf, is used to exhaustively check every single induced subtree of a cubic lattice. This is the most naive approach to the search for the largest induced subtree, and is very computationally expensive (though it does support multithreading). The largest cubic lattice that has finished running to date is a 3x4x4.

### Nested Monte-Carlo Tree Search

This algorithm (in monto-carlo-search), based on the paper at https://www.ijcai.org/Proceedings/09/Papers/083.pdf, is used to search for 'good' tree-based structures by using nested monte-carlo tree search combined with the base enumeration algorithm in master. To date, it has given us the largest known induced subtrees of any graph, though the search is not exhaustive.

## Running the Programs

For both programs, if not already on the master branch, run  
```git checkout master```  

### Exhaustive Enumeration

Run using either  
```make run size=A```  
for a cube of side length A or  
```make run sizeX=A sizeY=B sizeZ=C```  
for a rectangular prism of side lengths A, B, and C.

### Nested Monte-Carlo Tree Search

Run using either  
```make mcs level=K size=A```  
for nested Monte-Carlo at level K (higher levels take longer, but tend to produce better results) on a cube of size length A or  
```make mcs level=K sizeX=A sizeY=B sizeZ=C```  
for nested Monte-Carlo at level K on a rectangular prism of side lengths A,B, and C.
