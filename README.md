# Mother

Linux web server that can host static web sites, traverse through the file system and execute Pearl scripts.
Also able to create multiple connections at the same time through multi threading.

### Installing

To clone and compile this application you need [Git](https://git-scm.com/), make and [GCC](https://gcc.gnu.org/) on your system.

To install the programm, clone this repository to an arbitary folder: 

```bash
$ git clone https://github.com/jgdevroman/mother.git
```

And compile the program:

```bash
$ make all
```

## How to use

The web server can be launched by the command:

```bash
$ ./mother --wwwpath=<dir> [--port=<p>] [--threads=<t>]
```

* --wwwpath - Full path of an arbitary folder where the root folder of the web site is stored. 
* --port - Optional: Specify an arbitary port. Default: 2016
* --threads - Optional: Specify arbitary number of threads. Default: 8

After that, you can access your server by inserting the following URL in your browser:

```
http://localhost:<port> (if --port is not set: 2016)
```

### Test

You can test the server by using the test site "wwwdir" 

```bash
$ ./mother --wwwpath=</path/to/the/installed/folder/wwwdir> [--port=<p>] [--threads=<t>]
```

## Authors

* **Roman Shirasaki** - connection-mt.c, jbuffer.c, request-httpx.c, sem.c - [jdevroman](https://github.com/jgdevroman)

* **University of Erlangen/Nuremberg Department of Distributed Systems and Operating Systems** - Makefile, libmother.a 