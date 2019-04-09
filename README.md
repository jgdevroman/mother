# Mother

UNIX web server that can host static web sites, traverse through the file system and execute Pearl scripts.
Also able to create multiple connections at the same time through multi threading.

### Installing

To clone and compile this application you need [Git](https://git-scm.com/), make and [GCC](https://gcc.gnu.org/) on your system.

To install the programm clone this repository in an arbitary folder with 

```bash
$ git clone https://github.com/jgdevroman/mother.git
```

And compile the program with

```bash
$ make all
```

## How to use

The web server can be launched by the command

```bash
$ ./mother --wwwpath=<dir> [--port=<p>] [--threads=<t>]
```

* --wwwpath - Full path of an arbitary folder where the data of the system to be shown is stored. 
* --port - Optional: Specify an arbitary port. Default: 2016
* --threads - Optional: Specify arbitary number of threads. Default: 8

After that, you can access your server by inserting the following URL in your browser:

```
localhost:<port>
```

### Test

You can test the server by using the test site "wwwdir" by  

```bash
$ ./mother --wwwpath=</path/to/the/installed/folder/wwwdir> [--port=<p>] [--threads=<t>]
```

## Built With

* [Dropwizard](http://www.dropwizard.io/1.0.2/docs/) - The web framework used
* [Maven](https://maven.apache.org/) - Dependency Management
* [ROME](https://rometools.github.io/rome/) - Used to generate RSS Feeds


## Authors

* **Roman Shirasaki** - connection-mt.c, jbuffer.c, request-httpx.c, sem.c - [jdevroman](https://github.com/jgdevroman)

* **University of Erlangen/Nuremberg Department of Distributed Systems and Operating Systems** - Makefile, libmother.a 