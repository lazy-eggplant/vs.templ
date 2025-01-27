## v0.4.5

This release has not visible changes in its functionality (aside for some subtle bug fixes).  
The main objectives were:

- Reducing the complexity and verbosity of some code
- Removing obsolete or unused features
- Better document the public interface of the library.

## Breaking changes

The interface to build the preprocessor is now a configuration object.  
This is done to avoid verbosity, and allow further fields to be defined more easily if so needed.  
In most cases, using the new interface is just as simple as placing braces between the original args.
