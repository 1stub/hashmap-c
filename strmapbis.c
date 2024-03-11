// cs270 proj0
// Written by Anthony Arnold
// Used djb2 hash function from http://www.cse.yorku.ca/~oz/hash.html
// Collaborated with Dayne Freudenberg and Charles Oakes

#include "strmapbis.h"

int hash(char *str){
    int hash = 5381;
    int c;

    while((c = *str++))
	    hash = ((hash << 5) + hash) + c;
    return hash;
}

strmap_t *strmap_create(int numbuckets){
    if(numbuckets >= MAX_BUCKETS) numbuckets = MAX_BUCKETS;
    if(numbuckets <= MIN_BUCKETS) numbuckets = MIN_BUCKETS;	
    strmap_t *map = (strmap_t *)malloc(sizeof(strmap_t)); //creates map
    map->strmap_buckets = (smel_t **)calloc(numbuckets,sizeof(smel_t *));
    map->strmap_size = 0;
    map->strmap_nbuckets = numbuckets;   
    return map;
}

void *strmap_get(strmap_t *m, char *key){
    int key_index = hash(key) % m->strmap_nbuckets;
    smel_t *e = m->strmap_buckets[key_index];
    while(e != NULL){
	if(strcmp(e->sme_key, key) == 0){
	    return e->sme_value;
	}
	e = e->sme_next;
    }
    return NULL;
}
void *strmap_put(strmap_t *m, char *key, void *value){
    int index = hash(key) % m->strmap_nbuckets;

    smel_t *newE = (smel_t *)malloc(sizeof(smel_t));
    newE->sme_key = strdup(key);
    newE->sme_value = value;
    smel_t *list_head= m->strmap_buckets[index];
    smel_t *current;
    smel_t **prev; //previous bucket   
 
    current = list_head;
    prev = &list_head;

    // Bucket is empty, no other elements
    if(current == NULL){
	newE->sme_next = NULL;
        m->strmap_buckets[index] = newE;
        current = newE;
        *prev = newE;
	m->strmap_size++;
	return NULL;
    }
    
    while(current != NULL){
	smel_t *temp = list_head;
	    //loop bucket and check if key appears more than once
	    while(temp != NULL){ //temp allows for comparisons between every key
		if(strcmp(key, temp->sme_key) == 0){
		    void *old_value = temp->sme_value;
		    temp->sme_value=value;
		    free(newE->sme_key);
		    free(newE);
		    return old_value;
		}
		temp = temp->sme_next;
	    }
	    *prev = current;
	    //lexiographically smaller than next value
	    if(strcmp(key, m->strmap_buckets[index]->sme_key) < 0){
		m->strmap_buckets[index] = newE; //forwhatever reason list_head = newE; doesnt work here
		newE->sme_next = current;
		(*prev) = newE;
		m->strmap_size++;
		return NULL;
	    }
        //insertion of element between two nodes
	    else{
		if(current->sme_next == NULL || (strcmp(key, (*prev)->sme_key) > 0 && strcmp(key, current->sme_next->sme_key) < 0)){
		    newE->sme_next = current->sme_next;
		    (*prev)->sme_next = newE;
		    m->strmap_size++;		
		    return NULL;
		}
	    }
	    
	//increment currents position in the bucket
	*prev = current->sme_next;
	current = current->sme_next;
    }
    return NULL;
}

void *strmap_remove(strmap_t *m, char *key){ 
    int index = hash(key) % m->strmap_nbuckets;
    smel_t *de_v = NULL;
    smel_t *list_head = m->strmap_buckets[index];
    smel_t *current;
    smel_t **prev = NULL;
    
    prev = &list_head;
    current = list_head;

    while(current != NULL){
	if(strcmp(key, current->sme_key) == 0){
	    if(current == list_head){
		m->strmap_buckets[index] = current->sme_next;
	    } else {
		*prev = current->sme_next;
	    }
		de_v = current->sme_value;
		m->strmap_size--;
		return de_v;
	}	
	prev = &current->sme_next;
	current = current->sme_next;
    }
    return NULL;
}

int strmap_getsize(strmap_t *m){
    return m->strmap_size;
}

int strmap_getnbuckets(strmap_t *m){
    return m->strmap_nbuckets;
}

void strmap_dump(strmap_t *m){
    printf("total elements = %i.\n", m->strmap_size);
    for(int i = 0; i < m->strmap_nbuckets; i++){
	smel_t *current = m->strmap_buckets[i];
        if(current!=NULL) printf("bucket %i:\n", i);
	    while(current!=NULL){   
	        printf("\t%s->%p\n", current->sme_key, current->sme_value); 
	        current = current->sme_next;
	    }

    free(current);
    }    
    printf("\n");
}

double strmap_getloadfactor(strmap_t *m){
    return ((double)m->strmap_size)/((double)m->strmap_nbuckets);
}

void strmap_resize(strmap_t *m, double target){
    double loadFactor = strmap_getloadfactor(m);
    double min = target - LFSLOP*target;
    double max = target + LFSLOP*target;
    int oldNumBuckets = strmap_getnbuckets(m);

    if(loadFactor >= min && loadFactor <= max){
	return;
    }
    else{
	while(loadFactor < min){
	    m->strmap_nbuckets--;
	    loadFactor = strmap_getloadfactor(m);
	}
	while(loadFactor > max){
	    m->strmap_nbuckets++;
	    loadFactor = strmap_getloadfactor(m);
	}
	if(oldNumBuckets != strmap_getnbuckets(m)){
	    strmap_t *newMap = strmap_create(m->strmap_nbuckets);
	    for(int i = 0; i < oldNumBuckets; i++){    	    
		smel_t *current = m->strmap_buckets[i];
		while(current != NULL){
		    strmap_put(newMap, current->sme_key, current->sme_value);	
		    current = current->sme_next;
		}
	    }
	    free(m->strmap_buckets);
	    m->strmap_buckets = newMap->strmap_buckets;
	    free(newMap);
	}
    }
}

