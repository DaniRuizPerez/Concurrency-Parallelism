#include "cola.h" 
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>  

int num_chairs = 5;
int num_barbers = 5;
int num_customers = 100;
int max_waiting_time = 1000000;
int choosy_percent = 30;
int barberos_que_quedan = 0;

TCola cola;
struct timeval ti;

static struct option long_options[] = { 
	{ .name = "chairs",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "barbers",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "customers",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "max_waiting_time",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = "choosy_percent",
	  .has_arg = required_argument,
	  .flag = NULL,
	  .val = 0},
	{ .name = NULL,
	  .has_arg = 0,
	  .flag = NULL,
	  .val = 0}
};

static void usage(int i)
{
	printf(
		"Usage:  producers [OPTION] [DIR]\n"
		"Launch producers and consumers\n"
		"Opciones:\n"
		"  -b n, --barbers=<n>: number of barbers\n"
		"  -c n, --chairs=<n>: number of chairs\n"
		"  -n n, --clients=<n>: number of customers\n"
		"  -t n, --max_waiting_time=<n>: maximum time a customer will wait\n"
		"  -p n, --choosy-percent=<n>: percentage of customers that want a specific barber\n"
		"  -h, --help: show this help\n\n"
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

	if (!strcmp(option.name, "barbers")) {
		if (!get_int(arg, &num_barbers)
		    || num_barbers <= 0) {
			printf("'%s': not a valid integer\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "chairs")) {
		if (!get_int(arg, &num_chairs)
		    || num_chairs <= 0) {
			printf("'%s': not a valid integer\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "customers")) {
		if (!get_int(arg, &num_customers)
		    || num_customers <= 0) {
			printf("'%s': not a valid integer\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "max_waiting_time")) {
		if (!get_int(arg, &max_waiting_time)
		    || max_waiting_time <= 0) {
			printf("'%s': not a valid integer\n", arg);
			usage(-3);
		}
	}
	if (!strcmp(option.name, "choosy_percent")) {
		if (!get_int(arg, &choosy_percent)
		    || choosy_percent <= 0) {
			printf("'%s': not a valid integer\n", arg);
			usage(-3);
		}
	}
}

static int handle_options(int argc, char **argv)
{
	while (1) {
		int c;
		int option_index = 0;

		c = getopt_long (argc, argv, "hb:c:n:t:p:",
				 long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			handle_long_options(long_options[option_index],
				optarg);
			break;

		case 'b':
			if (!get_int(optarg, &num_barbers)
			    || num_barbers <= 0) {
				printf("'%s': not a valid integer\n",
				       optarg);
				usage(-3);
			}
			break;

		case 'c':
			if (!get_int(optarg, &num_chairs)
			    || num_chairs <= 0) {
				printf("'%s': not a valid integer\n",
				       optarg);
				usage(-3);
			}
			break;

		case 'n':
			if (!get_int(optarg, &num_customers)
			    || num_customers <= 0) {
				printf("'%s': not a valid integer\n",
				       optarg);
				usage(-3);
			}
			break;

		case 't':
			if (!get_int(optarg, &max_waiting_time)
			    || max_waiting_time <= 0) {
				printf("'%s': not a valid integer\n",
				       optarg);
				usage(-3);
			}
			break;
		case 'p':
			if (!get_int(optarg, &choosy_percent)
			    || choosy_percent <= 0) {
				printf("'%s': not a valid integer\n",
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


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t han_pasado_mutex = PTHREAD_MUTEX_INITIALIZER;
int han_pasado = 0;

pthread_mutex_t atendidos_mutex = PTHREAD_MUTEX_INITIALIZER;
int atendidos = 0;

pthread_mutex_t llena_mutex = PTHREAD_MUTEX_INITIALIZER;
int llena = 0;

pthread_mutex_t esperar_mutex = PTHREAD_MUTEX_INITIALIZER;
int esperar = 0;

pthread_mutex_t cola_mutex = PTHREAD_MUTEX_INITIALIZER;


int waiting_customers = 0;

struct barber_info *barber_info; 

enum barber_state_t { SLEEPING, WORKING} ;

struct barber_info {    /* Used as argument to thread_start() */
	pthread_t thread_id;        /* ID returned by pthread_create() */
	int       barber_num;
	pthread_cond_t *no_customers;
	TCola *colaParticular;
	enum barber_state_t barber_state;
	int atendiendo;
};

struct customer_info {    /* Used as argument to thread_start() */
	pthread_t thread_id;        /* ID returned by pthread_create() */
	int       customer_num;
};

unsigned int seed = 12345678;

double hora_actual(struct timeval tv) // a esta funcion le pasas un timestamp y te devuelve la hora actual tomando como partida ti
{
	gettimeofday(&tv, NULL); 
	return ((tv.tv_sec + tv.tv_usec*0.000001) - (ti.tv_sec + ti.tv_usec*0.000001))*1000000;
}

void cut_hair(int id, double hora)
{
	unsigned int time = rand_r(&seed) % 500000;
	// time that takes to do the haircut
	usleep(time);
	printf("cut hair of customer %d took %d time and finishes at %f\n", id, time,hora);
}

void *barber_function(void *ptr)
{
	int i = 0;	
	struct timeval tb;		
	struct barber_info *t =  ptr;
	TNodoCola cliente;
	TCola colapart;
	C_Crear(&colapart);
	t->colaParticular = &colapart;
	int pasoporcola = 0;
	int mesalgo = 0;

	printf("barber thread %d\n", t->barber_num);

	while(1) {

		pthread_mutex_lock(&han_pasado_mutex);
		printf("han_pasado -> %d num->customers ->%d waiting_customers->%d\n", han_pasado,num_customers,waiting_customers);
		if ((han_pasado == num_customers)&&(!waiting_customers)&&(atendidos+llena+esperar==num_customers)) {
			printf("BARBERO %d EMPIEZA A IRSE\n",t->barber_num);
			while (i < num_barbers){  //despierta a todos los barberos durmiendo
				if(barber_info[i].barber_state==SLEEPING){
					pthread_cond_signal(barber_info[i].no_customers);
					barber_info[i].barber_state = WORKING;
					printf("BARBERO %d DESPIERTA AL BARBERO %d\n",t->barber_num,barber_info[i].barber_num);
				}
				i++;			
			} 
			printf("BARBERO %d DESPERTO A SUS CONGENERES\n",t->barber_num);
			if (barberos_que_quedan != 1) {	
				printf("BARBERO %d SE VA A CASITA\n", t->barber_num);
			} 
			else {

				printf("BARBERO %d SE VA A CASITA\n", t->barber_num);
				printf("\n*****ESTADISTICAS*****\n\n");
				printf("Han pasado %d clientes por la barbería\n",han_pasado);
				printf("Se ha atendido a %d clientes\n",atendidos);
				printf("Se encontraron la barbería llena %d clientes\n",llena);	
				printf("Se cansaron de esperar y se fueron %d clientes\n\n",esperar);	
				if (C_Vacia(cola))
					printf("LA COLA ESTA VACIA\n");	
				else
					printf("NO ESTA VACIA!\n");			
			}
			barberos_que_quedan--;
			pthread_mutex_unlock(&han_pasado_mutex);
			return NULL;	 
		}

		pthread_mutex_unlock(&han_pasado_mutex);

		pthread_mutex_lock(&mutex);
		pthread_mutex_lock(&cola_mutex);
		if ((C_Vacia(cola)) && (C_Vacia(*t->colaParticular))){

			pasoporcola = 0;
			pthread_mutex_unlock(&cola_mutex);
			printf("colas vacias\n");
			
			if ((atendidos+llena+esperar != num_customers)){
				mesalgo = 0;
				t->barber_state = SLEEPING;
				printf("BARBER %d goes to sleep\n", t->barber_num);
				pthread_cond_wait(t->no_customers, &mutex);
				printf("BARBER %d despierta\n", t->barber_num);
			} else mesalgo = 1;
		}
		else {pasoporcola = 1;}
		if (pasoporcola){
			if ((C_Vacia(cola))&&(!C_Vacia(*t->colaParticular))){
				cliente = C_Sacar(t->colaParticular);
				printf("saco de la cola particular\n");
			}
			else 
			if ((C_Vacia(*t->colaParticular))&&(!C_Vacia(cola))){
				cliente = C_Sacar(&cola);
				printf("saco de la cola pública\n");
			}
			else
			if (C_Mirar(cola) <= C_Mirar(*t->colaParticular)){
				cliente = C_Sacar(&cola);
				printf("saco de la cola pública\n");
			}
			else {
				cliente = C_Sacar(t->colaParticular);
				printf("saco de la cola particular\n");
			} 
		pthread_cond_signal(cliente.waiting_roomc);
		printf("barber %d despertando cliente ------>> %d\n", t->barber_num,cliente.id);
		
		pthread_mutex_unlock(&cola_mutex);
		pthread_mutex_unlock(&mutex);
		cut_hair(cliente.id,hora_actual(tb));
		free(cliente.waiting_roomc);
		}
		else {
			pthread_mutex_unlock(&mutex);	
			pthread_mutex_unlock(&cola_mutex);
			if (!mesalgo)
				cut_hair(t->atendiendo,hora_actual(tb));
		}
	}
}

void get_hair_cut(int i)
{
	printf("CUSTOMER %d is getting a hair cut\n", i);
}

void *customer_function(void *ptr) 
{
		//VARIABLES
	struct timeval tp;
	struct timespec   ts;
	struct customer_info *t =  ptr;
	unsigned int max_time = rand_r(&seed) % max_waiting_time;
	int rc; int i = 0;
	unsigned int time = rand_r(&seed) % 100000 + t->customer_num * 100000;
	unsigned int choosy = rand_r(&seed) % 100;
	TNodoCola elemcola;
	usleep(time);
	rc = gettimeofday(&tp, NULL); 
	ts.tv_sec  = tp.tv_sec;
	ts.tv_nsec = tp.tv_usec * 1000;
	ts.tv_sec += max_time;	
	elemcola.horallegada = hora_actual(tp);

	pthread_mutex_lock(&han_pasado_mutex);
	han_pasado++; 
	pthread_mutex_unlock(&han_pasado_mutex);

		//LLEGA A LA BARBERÍA
	pthread_mutex_lock(&mutex);

	printf("consumer thread %d arrives in %d at %f\n", t->customer_num, time, elemcola.horallegada);
	printf("waiting customers -> %d lo dice el cliente %d\n",waiting_customers,t->customer_num); 

	if((waiting_customers == num_chairs))  {

		pthread_mutex_lock(&llena_mutex);
		llena++;
		pthread_mutex_unlock(&llena_mutex);
		printf("CLIENTE %d  SE VA INDIGNADO PORQUE LA BARBERÍA ESTABA LLENA\n\n", t->customer_num);
		pthread_mutex_unlock(&mutex);
		return NULL;
	}
	
	int pasodirectamente = 0;
	unsigned int preferido = rand_r(&seed) % num_barbers; 
		//DESPIERTO BARBEROS SI DORMIDOS
	if (choosy >= choosy_percent){ //si mete en la cola común, despierta a algún barbero dormido si lo hay
		while (i < num_barbers){ 
			if(barber_info[i].barber_state==SLEEPING){
				pasodirectamente = 1;
				barber_info[i].atendiendo = t->customer_num;
				pthread_cond_signal(barber_info[i].no_customers);
				barber_info[i].barber_state = WORKING;
				printf("cliente -> %d PASO SIN ENTRAR EN LA COLA NORMAL porque despierto al b ->%d \n",t->customer_num,i);
				break;
			}
			i++;	
		}
		if (!pasodirectamente){
			//RELLENO EL NODO
			printf("paso de forma normal en la cola normal :(\n");
			elemcola.id = t->customer_num;
			elemcola.waiting_roomc = malloc(sizeof(pthread_cond_t));
			pthread_cond_init(elemcola.waiting_roomc,NULL);

			pthread_mutex_lock(&cola_mutex);
			C_Agregar(&cola,elemcola);
			pthread_mutex_unlock(&cola_mutex);
		}

	} else { // sino, si el barbero preferido está dormido lo despierta
		pasodirectamente = 0;
		if(barber_info[preferido].barber_state==SLEEPING){
				pasodirectamente = 1;
				printf("cliente -> %d PASO SIN ENTRAR EN LA COLA PRIVADA \n",t->customer_num);
				pthread_cond_signal(barber_info[preferido].no_customers);				
				barber_info[preferido].atendiendo = t->customer_num;
				barber_info[preferido].barber_state = WORKING;
				
		}
		if (!pasodirectamente){

			//RELLENO EL NODO
			printf("cliente -> %d paso de forma normal en la cola privada:(\n",t->customer_num);
			elemcola.id = t->customer_num;
			elemcola.waiting_roomc = malloc(sizeof(pthread_cond_t));
			pthread_cond_init(elemcola.waiting_roomc,NULL);

			pthread_mutex_lock(&cola_mutex);
			C_Agregar(barber_info[preferido].colaParticular,elemcola);
			printf("PREFERIDO -> %d\n\n",preferido);
			pthread_mutex_unlock(&cola_mutex);
		}
	}
		
	if (!pasodirectamente){
		//ESPERO A QUE ME ATIENDAN
		waiting_customers++;
		printf("waiting customers ++ -> %d lo dice el cliente %d\n",waiting_customers,t->customer_num); 
		rc = pthread_cond_timedwait(elemcola.waiting_roomc, &mutex, &ts);
		waiting_customers--;
		printf("waiting customers -- -> %d lo dice el cliente %d\n",waiting_customers,t->customer_num); 
	}
	if (rc == ETIMEDOUT) { //Si llega aquí, salio del wait por que se acabó el tiempo
		pthread_mutex_lock(&esperar_mutex);
		esperar++;
		pthread_mutex_unlock(&esperar_mutex);
		printf("CLIENTE %d SE VA INDIGNADO PORQUE SE HA CANSADO DE ESPERAR\n", t->customer_num);	
				
		pthread_mutex_lock(&cola_mutex);
		C_Eliminar(&cola,elemcola.id);
		C_Eliminar(barber_info[preferido].colaParticular,elemcola.id);
		pthread_mutex_unlock(&cola_mutex); 

		pthread_mutex_unlock(&mutex); 
		return NULL; 
	} 
	else {
		//YA ME VAN A ATENDER		
		pthread_mutex_lock(&atendidos_mutex);
		atendidos++;
		pthread_mutex_unlock(&atendidos_mutex);

		pthread_mutex_unlock(&mutex);
		get_hair_cut(t->customer_num);

		return NULL;
	}
}

void create_threads(void)
{
	int i;

	struct customer_info *customer_info;

	printf("creating %d barbers\n",num_barbers);

	barber_info = malloc(sizeof(struct barber_info)*num_barbers);

	if (barber_info == NULL) {
		printf("Not enough memory\n");
		exit(1);
	}

	/* Create independent threads each of which will execute function */
	for (i = 0; i < num_barbers; i++) {
		barber_info[i].barber_num = i;
		barber_info[i].atendiendo= 1000;
		barber_info[i].barber_state = WORKING;
		barber_info[i].no_customers = malloc(sizeof(pthread_cond_t));
		pthread_cond_init(barber_info[i].no_customers,NULL);
		if ( 0 != pthread_create(&barber_info[i].thread_id, NULL,
					 barber_function, &barber_info[i])) {
			printf("Failing creating barber thread %d", i); 
			exit(1);
		}
	}

	printf("creating %d consumers\n", num_customers);
	customer_info = malloc(sizeof(struct customer_info) * num_customers);

	if (customer_info == NULL) {
		printf("Not enough memory\n");
		exit(1);
	}

	for (i = 0; i < num_customers; i++) {
		customer_info[i].customer_num = i;
		if ( 0 != pthread_create(&customer_info[i].thread_id, NULL,
					 customer_function, &customer_info[i])) {
			printf("Failing creating customer thread %d", i);
			exit(1);
		}
	}
	pthread_exit(0);
}

int main (int argc, char **argv)
{
	C_Crear(&cola); //creo la cola compartida
	seed = time(0);
    	gettimeofday(&ti, NULL);   // Instante inicial	

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
	barberos_que_quedan = num_barbers;
	create_threads();
	exit (0);
}
