Network Server/Client Manager Library
-------------------------------------
Inside of the includes folder, there are 3 folders that seperate the includes into base, client, and server.

When including these files it is suggested that you include both base and the type you are looking for instead of everything.
e.g. For a client include the base folder and client folder. 

Doing so will place all the headers being used when called by #include <> in the ntwk directory.
e.g. #include <ntwk/StaticFunctions.h>

Built using /MD and /MDd.