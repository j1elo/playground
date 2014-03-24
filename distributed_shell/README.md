Distributed Shell
=================

*DShell* is a very simple Distributed Shell in which the real location of several resources is totally irrelevant. This project aims to achieve full location transparency to all files and commands, making it possible to call a remote executable to operate over remote files, where **neither the executable nor the files reside in the same computer**.

The architecture of the distributed system consists of several server process (*dserver*) which may get instantiated either in the same or different machines. These servers are able to accept remote requests, and they will communicate between each others to process those requests that cannot be resolved locally.

In order to collaborate, all servers must know of each other; in order to do so, they will register themselves into a coordinated *cluster* of servers. Client applications can communicate and send requests to any arbitrary server, as long as it was correctly registered to be part of some cluster. These are the specific commands supported by the current protocol version:

- ***r_exec*** `<executable name> <arguments>` runs a remote process.
- ***d_find*** `<file name>` locates a file in the server cluster.
- ***d_register*** `<IP:port>` registers a new server in the cluster.
- ***d_unregister*** `<IP:port>` unregisters a server before it shuts down.
- ***d_ls*** `<name>` locates a path in the cluster, and lists its directory contents.

Some sample client applications are already implemented: *dcat*, *dtac*, *dls*. They work in the same way as the original Unix tools *cat*, *tac* and *ls*. Finally, *dsh* is a simple implementation of a distributed shell, which uses the aforementioned tools in order to allow remote access to files and processes. It is able to open files and redirect standard input and standard output through a network socket, making it possible to manage remote files. All clients make use of a common library (*libdclient*) which implements remote communication procedures.