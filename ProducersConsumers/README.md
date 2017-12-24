Producer Consumer Problem using Mutex
============

This is a well-known problem that I developed for the Concurrency and Parallelism course in the sophomore year of my undergrad in computer science at UDC (Spain). Producers and consumers run in a thread each and they synchronize using mutexes. Producers push elements in a "queue" and consumers pop them. It creates producers or consumers dynamically to balance the load.


## Execution

```
Usage:  producerconsumer [OPTION] [DIR]
  -p n, --producers=<n>: number of producers
  -c n, --consumers=<n>: number of consumers
  -b n, --buffer_size=<n>: number of elements in buffer
  -i n, --iterations=<n>: total number of itereations
  -e n, --espera=<n>: max time to wait of a thread
  -h, --help: shows this help
```


## Contact

Contact [Daniel Ruiz Perez](mailto:druiz072@fiu.edu) for requests, bug reports and good jokes.


## License

The software in this repository is available under the GNU General Public License, version 3. See the [LICENSE](https://github.com/DaniRuizPerez/Concurrency-Parallelism/blob/master/LICENSE) file for more information.
