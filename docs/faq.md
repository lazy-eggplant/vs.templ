## Is there a SAX implementation?

No, there is not. 
Some features don't strictly require a DOM, but they don't really translate well in terms of a pull parser.  
The memory peak usage would not be much better either for many reasonable templates, so there is no real incentive to implement this preprocessor based on a different backend for now.  
Still, if you want to do so you are very welcome!