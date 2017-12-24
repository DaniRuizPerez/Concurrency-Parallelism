SPMD Master-Slave matrix vector multiplication and sorting
============

Matrix vector product using Single Program, Multiple Data (SPMD) with Send/Receive and collective operations. Then it sorts the resulting vector using a master-slave architecture. It prints computation time and different statistics. I developed this project for the Concurrency and Parallelism course in the sophomore year of my undergrad in computer science at UDC (Spain). 


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

The software in this repository is available under the GNU General Public License, version 3. See the [LICENSE](https://github.com/DaniRuizPerez/CharacterRecognizerLeagueOfLegends/blob/master/LICENSE) file for more information.
