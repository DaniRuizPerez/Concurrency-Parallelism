#include <pthread.h>
typedef struct TNodoCola { 
	int id; 	
	double horallegada;
	double horacorte;
	pthread_cond_t *waiting_roomc;
	struct TNodoCola *Siguiente; 
} TNodoCola; 

typedef struct{ 
	TNodoCola *Primero;
	TNodoCola *Ultimo;
} TCola; 

void C_Crear(TCola *pC); 
void C_Vaciar(TCola *pC); 
int C_Vacia(TCola C); 
int C_Agregar(TCola *pC, TNodoCola E); 
TNodoCola C_Sacar(TCola *pC); 
int C_Eliminar(TCola *pC, int E);
double C_Mirar(TCola pC);
