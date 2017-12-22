// DANIEL RUIZ PÃ‰REZ 2.1.2

#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int num_producers = 2;
int num_consumers = 2;
int buffer_size = 1;
int iterations = 100;

int consumir; 
int lleno = 0;
int vacio = 0;
int espera = 100000;
int morir = 0;
int consumidores = 0;


//la variable consumir es propia de los consumidores y se inicializa a iterations, y cada vez que se consume se decrementa, hasta llegar a 0

static struct option long_options[] = {
	{ .name = "producers",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "consumers",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "buffer_size",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "iterations",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "espera",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{0, 0, 0, 0}
};

static void usage(int i)
{
	printf(
		"Usage:  producers [OPTION] [DIR]\n"
		"Launch producers and consumers\n"
		"Opciones:\n"
		"  -p n, --producers=<n>: number of producers\n"
		"  -c n, --consumers=<n>: number of consumers\n"
		"  -b n, --buffer_size=<n>: number of elements in buffer\n"
		"  -i n, --iterations=<n>: total number of itereations\n"
		"  -e n, --espera=<n>: max time to wait of a thread\n"
		"  -h, --help: muestra esta ayuda\n\n"
	);
	exit(i);
}

static int get_int(char *arg, int *value)
{
	char *end;
	*value = strtol(arg, &end, 10);

	return (end != NULL);
}

static void handle_long_options(struct option option, char *arg)
{
	if (!strcmp(option.name, "help"))
		usage(0);

	if (!strcmp(option.name, "producers")) {
		if (!get_int(arg, &num_producers)
		    || num_producers <= 0) {
			printf("'%s': no es un entero vÃ¡lido\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "consumers")) {
		if (!get_int(arg, &num_consumers)
		    || num_consumers <= 0) {
			printf("'%s': no es un entero vÃ¡lido\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "buffer_size")) {
		if (!get_int(arg, &buffer_size)
		    || buffer_size <= 0) {
			printf("'%s': no es un entero vÃ¡lido\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "iterations")) {
		if (!get_int(arg, &iterations)
		    || iterations <= 0) {
			printf("'%s': no es un entero vÃ¡lido\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "espera")) {
		if (!get_int(arg, &espera)
		    || espera <= 0) {
			printf("'%s': no es un entero vÃ¡lido\n", arg);
			usage(-3);
		}
	}
}

static int handle_options(int argc, char **argv)
{
	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long (argc, argv, "hp:c:b:i:e:",
				 long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			handle_long_options(long_options[option_index],
				optarg);
			break;

		case 'p':
			if (!get_int(optarg, &num_producers)
			    || num_producers <= 0) {
				printf("'%s': no es un entero vÃ¡lido\n",
				       optarg);
				usage(-3);
			}
			break;


		case 'c':
			if (!get_int(optarg, &num_consumers)
			    || num_consumers <= 0) {
				printf("'%s': no es un entero vÃ¡lido\n",
				       optarg);
				usage(-3);
			}
			break;

		case 'b':
			if (!get_int(optarg, &buffer_size)
			    || buffer_size <= 0) {
				printf("'%s': no es un entero vÃ¡lido\n",
				       optarg);
				usage(-3);
			}
			break;

		case 'i':
			if (!get_int(optarg, &iterations)
			    || iterations <= 0) {
				printf("'%s': no es un entero vÃ¡lido\n",
				       optarg);
				usage(-3);
			}
			break;
		
		case 'e':
			if (!get_int(optarg, &espera)
			    || espera <= 0) {
				printf("'%s': no es un entero vÃ¡lido\n",
				       optarg);
				usage(-3);
			}
			break;
			
		case '?':
		case 'h':
			usage(0);
			break;

		default:
			printf ("?? getopt returned character code 0%o ??\n", c);
			usage(-1);
		}
	}
	return 0;
}

struct element {
	int producer;
	int value;
	int time;
};

struct element **elements = NULL;
int count = 0;

void insert_element(struct element *element)
{
	if (count == buffer_size) {
		printf("buffer is full\n");
		exit(-1);
	}
	elements[count] = element;
	count++;
}

struct element *remove_element(void)
{
	if (count == 0) {
		printf("buffer is empty\n");
		exit(-1);
	}
	count--;
	return elements[count];
}


pthread_cond_t buffer_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER;

pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t iterations_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumir_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lleno_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vaciomorir_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;


struct thread_info {    /* Used as argument to thread_start() */
	pthread_t thread_id;        /* ID returned by pthread_create() */
	int       thread_num;       /* Application-defined thread # */
};

struct thread_info *consumer_infos; //todos los consumidores estan en este struct global, por que se crean en dos sitios diferentes.


void *producer_function(void *ptr)
{
	struct thread_info *t =  ptr;
	unsigned int seed = t->thread_num;

	printf("producer thread %d\n", t->thread_num);
	while(1) {

		pthread_mutex_lock(&iterations_mutex);
			if(iterations <= 0) {
			printf("\nPRODUCTOR -> %d SALE DEL BUCLEEEEEEEEE, count al salir -> %d\n", t->thread_num,count);
			pthread_mutex_unlock(&iterations_mutex);
				break;
			}
		iterations--;
//el iterations se decrementa aquÃ­ por que si no, otro puede decrementar iterations antes de que este thread aÃ±ada el elemento al buffer
		pthread_mutex_unlock(&iterations_mutex);

		printf("ITERATIONS -> %d\n", iterations);
		struct element *e = malloc(sizeof(*e));
		if (e == NULL) {
			printf("Ount of memory");
			exit(-1);
		}
		
		e->producer = t->thread_num;
		e->value = rand_r(&seed) % 1000;
		e->time = rand_r(&seed) % espera;
		usleep(e->time);
		printf("%d: produces %d in %d microseconds iterations -> %d count -> %d\n", t->thread_num, e->value, e->time, iterations,count);
		pthread_mutex_lock(&buffer_mutex);
		
		while(count == buffer_size) {
			pthread_cond_wait(&buffer_full, &buffer_mutex);
		}
		
		insert_element(e);
		
		pthread_mutex_lock(&count_mutex);	
		if(count==1)
			pthread_cond_broadcast(&buffer_empty);
		pthread_mutex_unlock(&count_mutex);
		pthread_mutex_unlock(&buffer_mutex);
	}
	return NULL;
}

void *consumer_function(void *ptr)
{
	struct thread_info *t =  ptr;
	printf("consumer thread %d\n", t->thread_num);
	
	while(1) {
	
    pthread_mutex_lock(&vaciomorir_mutex);
	 if (vacio >= 10 && morir == 1){
		      morir = 0;
		      pthread_mutex_unlock(&vaciomorir_mutex);
		      printf("\n\n\n\n\n\n\n\n\nCONSUMIDOR -> %d SALE SE MUERE POR QUE HAY DEMASIADOS, count al salir ->%d \n", t->thread_num, count);
		      consumidores--;
		      break;
		   }
			pthread_mutex_unlock(&vaciomorir_mutex);
	 
	   pthread_mutex_lock(&count_mutex);
		if (count <=1) {
			pthread_mutex_unlock(&count_mutex);
			pthread_mutex_lock(&vaciomorir_mutex);
		   vacio++;
		   pthread_mutex_unlock(&vaciomorir_mutex);
		} else pthread_mutex_unlock(&count_mutex);

	//aqui hacemos lo mismo, solo que el contador es consumir para asegurarme de que se crean y se consumen los mismos elementos y que el buffer esta vacio
		pthread_mutex_lock(&consumir_mutex);
		if((consumir <= 0)) {
			printf("\nCONSUMIDOR -> %d SALE DEL BUCLEEEEEEEEE, count al salir ->%d \n", t->thread_num, count);	
			consumidores--;   
			pthread_mutex_unlock(&consumir_mutex);
			break;
		}
		consumir--;
		pthread_mutex_unlock(&consumir_mutex);
		
		
		pthread_mutex_lock(&buffer_mutex);
		struct element *e;

		while(count==0)
			pthread_cond_wait(&buffer_empty, &buffer_mutex);
		e = remove_element();
		pthread_mutex_lock(&count_mutex);
		if(count == buffer_size -1){
			pthread_mutex_unlock(&count_mutex);
			pthread_cond_broadcast(&buffer_full);
			
			pthread_mutex_lock(&lleno_mutex);
			lleno ++;
			pthread_mutex_unlock(&lleno_mutex);
		}
		pthread_mutex_unlock(&count_mutex);
		pthread_mutex_unlock(&buffer_mutex);

   	usleep(e->time);
		printf("%d: consumes %d count -> %d consumir ->%d \n", t->thread_num, e->value, count, consumir);
		free(e);
	}
	return NULL;
}

void *control_function(void *ptr)
{
//esta funcion es un thread a parte que se activa cada segundo, poniendo vacio a 0 y si lleno es mayor que 10, crea un nuevo consumidor, hasta un mÃ¡ximo de 10
	int i = 0;
	while (1) {
	   sleep(1);
	   //la variable morir se pone a 1 cada segundo, y cuando un thread muere lo pone a 0
	   pthread_mutex_lock(&vaciomorir_mutex);
		vacio = 0;
		morir = 1;
	   pthread_mutex_unlock(&vaciomorir_mutex);
	   
      pthread_mutex_lock(&lleno_mutex);
	   if (lleno >= 10){
	      lleno = 0;
	      pthread_mutex_unlock(&lleno_mutex);      
	      printf("YIPIHAIYEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n");
	     consumer_infos = realloc(consumer_infos, (sizeof(struct thread_info)*(num_consumers+i+1)));
	     //realloc aumenta el tamaÃ±o del array, aÃ±adiendo mas structs vacios para los consumidores,
	     //aun que aqui esta mal porque se pierden punteros
	     
        if (consumer_infos == NULL) {
	       printf("Not enough memory\n");
	       exit(1);
	     }
	     
	      printf("creating 1 consumer\n");
	      consumidores++;
		      consumer_infos[num_consumers +i].thread_num = num_consumers+i;
		      if ( 0 != pthread_create(&consumer_infos[num_consumers +i].thread_id, NULL,
					       consumer_function, &consumer_infos[num_consumers +i])) {
			      printf("Failing creating consumer thread %d", num_consumers +i);
			      exit(1);
		      }
		    i++;  
		        
	   } else {
	      lleno = 0;
	      pthread_mutex_unlock(&lleno_mutex);
	    }
      pthread_mutex_lock(&consumir_mutex); 
		if(consumir <= 0 && iterations <= 0 && consumidores == 0) {
		printf("\nCONTROL ACABA\n");
		printf("\nESPERA = %d\n", espera);
		printf("\nconsumidores que quedan -> %d\n", consumidores);
		pthread_mutex_unlock(&consumir_mutex);
			break;
		}
	   pthread_mutex_unlock(&consumir_mutex);
	}
	return NULL;
}

void producers_consumers(num_producers, num_consumers)
{
	int i;
	struct thread_info *producer_infos;
	struct thread_info *control_flujo;

	printf("creating buffer with %d elements\n", buffer_size);
	elements = malloc(sizeof(struct element) * buffer_size);

	if (elements == NULL) {
		printf("Not enough memory\n");
		exit(1);
	}

	printf("creating %d producers\n", num_producers);
	producer_infos = malloc(sizeof(struct thread_info) * num_producers);

	if (producer_infos == NULL) {
		printf("Not enough memory\n");
		exit(1);
	}

	/* Create independent threads each of which will execute function */
	for (i = 0; i < num_producers; i++) {
		producer_infos[i].thread_num = i;
		if ( 0 != pthread_create(&producer_infos[i].thread_id, NULL,
					 producer_function, &producer_infos[i])) {
			printf("Failing creating consumer thread %d", i);
			exit(1);
		}
	}

	printf("creating %d consumers\n", num_consumers);
	consumer_infos = malloc(sizeof(struct thread_info) * num_consumers);

	if (consumer_infos == NULL) {
		printf("Not enough memory\n");
		exit(1);
	}

	for (i = 0; i < num_consumers; i++) {
	   consumidores++;
		consumer_infos[i].thread_num = i;
		if ( 0 != pthread_create(&consumer_infos[i].thread_id, NULL,
					 consumer_function, &consumer_infos[i])) {
			printf("Failing creating consumer thread %d", i);
			exit(1);
		}
	}
	printf("creating 1 control_flujo\n");
	control_flujo = malloc(sizeof(struct thread_info));

	if (control_flujo == NULL) {
		printf("Not enough memory\n");
		exit(1);
	}

		control_flujo[0].thread_num = 0;
		if ( 0 != pthread_create(&control_flujo[0].thread_id, NULL,
					 control_function, &control_flujo[i])) {
			printf("Failing creating consumer thread %d", i);
			exit(1);
		}
	
	pthread_exit(0);
}

int main (int argc, char **argv)
{
	int result = handle_options(argc, argv);
	
	if (result != 0)
		exit(result);

	if (argc - optind != 0) {
		printf ("Extra arguments\n\n");
		while (optind < argc)
			printf ("'%s' ", argv[optind++]);
		printf ("\n");
		usage(-2);
	}
	consumir = iterations;
	producers_consumers(num_producers, num_consumers);

	exit (0);
}

/*



