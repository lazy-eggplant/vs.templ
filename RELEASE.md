## v0.4.5

This release has not visible changes in its functionality (aside for some subtle bug fixes).  
The main objectives were:

- Reducing complexity and verbosity for most of the code
- Removing obsolete or unused features from earlier revisions
- Better documentation of the public interface.

## Breaking changes

The interface to build the preprocessor is now a configuration object.  
This is done to avoid verbosity, and to allow for the definition of more fields if so needed.  
In most cases, using the new interface is just as simple as placing braces between the original args.
