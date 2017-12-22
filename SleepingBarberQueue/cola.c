#include "cola.h" 
#include <malloc.h> 
#include <memory.h> 

 
void C_Crear(TCola *pC) { 
	pC->Primero = pC->Ultimo = NULL; 
} 

void C_Vaciar(TCola *pC) { 
	TNodoCola *pAux = pC->Primero; 
	TNodoCola *pSig; 
	while (pAux){ 
		pSig = pAux->Siguiente; 
		free(pAux); 
		pAux = pSig; 
	} 
	pC->Primero = pC->Ultimo = NULL; 

} 

int C_Vacia(TCola C){ 
	return (C.Primero==NULL && C.Ultimo==NULL); 
} 

int C_Agregar(TCola *pC, TNodoCola E) { 
	printf("metido en la cola al cliente %d\n", E.id);
	TNodoCola *pNodo = (TNodoCola*) malloc(sizeof(TNodoCola)); 
	if (!pNodo) 
		return 0; 
	else{
		if (pC->Ultimo) 
			pC->Ultimo->Siguiente = pNodo; 
		if (!pC->Primero) //Está vacía 
			pC->Primero = pNodo; 
		*pNodo = E; 
		pNodo->Siguiente = NULL; 
		pC->Ultimo = pNodo; 
		return 1; 
	}	
} 


TNodoCola C_Sacar(TCola *pC) { 
	printf("Eliminado de la cabeza de la cola cliente %d \n", pC->Primero->id);
	TNodoCola Aux = *pC->Primero; 
	pC->Primero = pC->Primero->Siguiente; 
	if (pC->Primero == NULL) 
		pC->Ultimo = NULL; 
	return Aux;
}


double C_Mirar(TCola pC){
	if (C_Vacia(pC))
		return 999999999;
	return pC.Primero->horallegada;
}

int C_Eliminar(TCola *pC, int E){
	if (C_Vacia(*pC)){
		return 0;
		printf("\ncola vacía no se puede eliminar\n\n\n\n\n");
	}
	else {
		printf("Eliminado del medio de la cola cliente %d \n",E);
		TNodoCola *pAux = pC->Primero;
		if (pAux->id == E) {
			pC->Primero = pC->Primero->Siguiente; 
			if (pC->Primero == NULL) 
				pC->Ultimo = NULL; 
			free(pAux);
			return 1;
		} 	
		while (pAux->Siguiente != NULL) {
			if (pAux->Siguiente->id == E) {
				TNodoCola *elim = pAux->Siguiente;
				pAux->Siguiente = pAux->Siguiente->Siguiente;
				if (elim == pC->Ultimo)
					pC->Ultimo = pAux;
				free(elim);
				return 1;
			}
			pAux= pAux->Siguiente;
		}
		return 0;
	}
}



