Barbers and Clients using Mutex
============

This is a well known problem that I developed for the Concurrency and Parallelism course in the sophomore year of my undergrad in computer science at UDC (Spain). There are multiple barbers that run in their own thread, and the clients come to the Barber that has to sequentially attend a certain number of  clients. The clients are generated randomly also as threads. If a client walks in into the barber shop, he can wait in line. If the room is full or he waits more than max waiting time, he leaves. Some clients have a preference for the barber that should cut their hair, other don't care. If barbers have no clients, they go to sleep and one is awakened when a client arrives at his shop. There is a history of every action taken by both barbers or clients.


## Execution

```
Usage:  barber [OPTION] [DIR]
  -b n, --barbers=<n>: number of barbers
  -c n, --chairs=<n>: number of chairs
  -n n, --clients=<n>: number of customers
  -t n, --max_waiting_time=<n>: maximum time a customer will wait
  -p n, --choosy-percent=<n>: percentage of customers that want a specific barber
  -h, --help: shows this help
```


## Contact

Contact [Daniel Ruiz Perez](mailto:druiz072@fiu.edu) for requests, bug reports and good jokes.


## License

The software in this repository is available under the GNU General Public License, version 3. See the [LICENSE](https://github.com/DaniRuizPerez/Concurrency-Parallelism/blob/master/LICENSE) file for more information.
