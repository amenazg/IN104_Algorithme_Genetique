#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <stdbool.h>

Individu * getNewIndividu(int l, int *t){
    // création d'un nouveau individu 
    Individu * indv  = malloc(sizeof(Individu)); // initialisation des attributs 
    indv->value = 0;
    indv->next= NULL;
    indv->nb = 0;
    indv->DNA = malloc(l*sizeof(int));
    int cnt=0;
    for(int i=0;i<l;i++){
        int ra = rand()%60; // choix de la gene avec une 1/3 probabilté entre {0,1,2}
        if(ra<20){//20
            indv->DNA[i]=0;
            indv->nb++;
        }else if(ra<40){//40
            indv->DNA[i]=1;
            cnt++;
        }else{
            indv->nb++;
            indv->DNA[i]=2;
        }
        indv->value+=(indv->DNA[i]-1)*t[i];// sommation suivant la configuration de l'adn
    }
    indv->value = abs(indv->value); // valeur absolue de la somme 
    if(cnt==l){
        // si l'individu génré ne prend aucun element de l'ensemble de départ dans la somme , on l'elimine 
        free(indv);
        return getNewIndividu(l, t);
    }
    return indv;
}

Individu * getCopy(Individu * indiv, int l){
    // copie de l'individu sans avoir un succeur . 
    Individu * indv  = malloc(sizeof(Individu));
    indv->value = indiv->value;
    indv->next= NULL;
    indv->nb=indiv->nb;
    indv->DNA = malloc(l*sizeof(int));
    for(int i=0;i<l;i++){
        indv->DNA[i] = indiv->DNA[i];
    }
    return indv;
}

Individu * getElementByIndex (Individu * gen , int idx){
    // retoune de l'individu suivant sa position (ou indice ) dans la liste chainée
    if(idx == 0)return gen; //  si idx=0 , elle retourne l'elemnt actuelle qui correspond à la tête de la liste
    return getElementByIndex(gen->next, --idx); //  sinon , on décrémentant l'indice et appliquer la fonction au élement suivant  
}

Individu * GetBestElements(Individu * gen, int k){
    //retourne les meilleures k individus , mais notre liste est triée grace à la fonction insertion .  
    if(gen==NULL)return NULL;
    if(k>0){
        gen->next = GetBestElements(gen->next, k-1);
        return gen;
    }else{
        gen->next =  GetBestElements(gen->next, k-1); // on libere le reste de la liste 
        free(gen);
        return NULL;
    }
}

Individu * inser(Individu * gen , Individu * indiv, int l){
    // insérer l'individus dans la liste chaînée suivant sa pertinence , pour avoir une liste chaînée triée . 
    if(gen==NULL){
        return indiv;
    }
    if(cmp(indiv, gen)){ //  cmp : fonction de comparaison entre deux individus .
        indiv->next = gen;
        return indiv;
    }
    Individu * pcrawl = gen; // pointeur pour parcourir la liste
    Individu * last = gen; //  dérnier individu visité dans la liste 
    while(1){
        if(pcrawl == NULL){
            // si on atteind la fin de la liste on ajoute l'individu à la fin de la liste
            last->next = indiv;
            return gen;
        }else{
            if(!cmp(indiv, pcrawl)){
                if(eq(indiv, pcrawl,l))return gen;
                // si l'individu dans  pcrawl est plus pertinent  que notre individu , on continue notre parcours .
                
                last = pcrawl;
                pcrawl = pcrawl->next;
            }else{
                // sinon on l'insére avant pcrawl cad àprés l'individu last . 
                last->next = indiv;
                indiv->next = pcrawl;
                return gen;
            }
        }
    }
    return gen;
}

Individu * getInitialGeneration( int n, int* t, int l){
    // cette fonction génere la generation initiale de n individus . 
    Individu * gen = getNewIndividu(l,t); // 1ér individu avant le tri 
    for(int i=0;i<n-1;i++){
        // on ajoute les n-1 élements dans l'ordre en fonction de la pertinence . 
        gen = inser(gen,getNewIndividu(l,t),l);
    }
    return gen;
}



Individu * addChildGen(Individu * gen , int n , int l, int *t){
    // cette fonction génere les individus de la géneration suivante à partir des meilleures individus e la géneration précedente . 
    Individu  * childGen = NULL;
    double a[n+1];
    double s = 1.7;
    int lambda = n;
    for(int i=0;i<n;i++){
        a[i]=(2.0-s)/(double)(n*1.0) + (2.0*(n-i-1)*(s-1))/(double)(1.0*n*(n-1));
        if(i>0)a[i]+=a[i-1];
    }// le tableau a contient une distribution de probabilité qui favorise les meilleures individus pour participer à la réproduction 
    double r = ((double)rand() / (double)RAND_MAX ) /((double)1.0*lambda);
=    int j=0;
    int current_member=0;
    while(current_member<lambda){
        while(r<=a[j] && current_member<lambda){ 
            int father_idx =current_member ;// choix aléatoire de l'indice du pére 
            int mother_idx = j;
            
            Individu * father = getCopy(getElementByIndex(gen,father_idx),l); 
            Individu * mother = getCopy(getElementByIndex(gen,mother_idx),l); //  copie des parents 

            croisement(father, mother, t , l); // on fait le croisement entre les deux parents 

            if(checkIndiv(father,l))childGen = inser(childGen, father,l); // si l'individu est valide , on l'ajoute à la nouvelle géneration
            if(checkIndiv(mother,l))childGen = inser(childGen, mother,l); // si l'individu est valide , on l'ajoute à la nouvelle géneration

            r = r + (1.0/(double)lambda);
            current_member++;
        }
        j++;
    }
    for(int i=0;i<n;i++){
        int ra =  rand()%1000;
       // appliquer la mutation suivant une probabilité de 8%
        if(ra<=80){
            int idx = rand()%(n/2); //  cchoix aléatoire de l'indice de meilleures (n/2) individus . 
            Individu * to_mutate = getCopy(getElementByIndex(gen,idx),l); 
            mutation(to_mutate, l); // appliquer la mutation
            to_mutate->value = fitness(to_mutate, l , t); //  mise à jour de la valeur de fitness
            to_mutate->nb = fit_nb(to_mutate, l); //mise à jour du nombre des élements pris de l'ensemble de départ .
            if(checkIndiv(to_mutate,l))childGen = inser(childGen, to_mutate,l); //  si l'individus est valide on l'insére à la liste.
        }else{
            childGen = inser(childGen, getNewIndividu(l,t),l);
        }
    }

    while(childGen){// cette boucle while a pour but de mettre à jour la liste de generation precedente avec la nouvelle géneration 
        // et la libération de la liste utilisée dans la fonction
        Individu * indiv = getCopy(childGen,l); 
        Individu * lastindiv = childGen; 
        childGen = childGen->next; 
        free(lastindiv); 
        gen = inser(gen , indiv,l);
    }
    return gen;
}


void mutation(Individu * indiv, int l){
    int pos  =  rand() % l; //  choose a position by random to apply mutation in it
    int ra = rand()%50; // there are two other possibilities so we are selection them equally
    if(ra<25)indiv->DNA[pos] = (indiv->DNA[pos] + 1)%3; // +1 and +2 modular 3 will give the other two possibilities
    else indiv->DNA[pos] = (indiv->DNA[pos] + 2)%3;
}


void croisement(Individu * father, Individu * mother , int *t, int l ){
    int ra = rand() % (l-1); // choose the position of the cut randomly
    father->value = 0;
    mother->value = 0;
    father->nb = 0;
    mother->nb = 0;
    father->next = NULL;
    mother->next = NULL;
    for(int i=0;i<l;i++){
        if(i<=ra){
            // if i <= cut position we swap the configuration of the father and the mother otherwise let it the same
            int aux = father->DNA[i];
            father->DNA[i]=mother->DNA[i];
            mother->DNA[i]=aux;
        }
        // updating parameters
        father->value += (father->DNA[i]-1)*t[i];
        father->nb += abs((father->DNA[i]-1));
        mother->value += (mother->DNA[i]-1)*t[i];
        mother->nb += abs((mother->DNA[i]-1));
    }
    father->value = abs(father->value);
    mother->value = abs(mother->value);
}

int fitness(Individu * ind , int l , int*t){
    // function to calculate the fitness of an individual
    int sum=0;
    for(int i=0;i<l;i++){
        sum+=(ind->DNA[i]-1)*t[i];
    }
    return abs(sum);
}

int fit_nb(Individu* indiv, int l){
    // function to calculate the number of taken numbers in a configuration of an individual
    int sum=0;
    for(int i=0;i<l;i++){
        if(indiv->DNA[i]==1)continue;
        sum++;
    }
    return sum;
}

int cmp(Individu *  indiv1 , Individu * indiv2){
    // function to compare 2 individuals
    // return true if indiv1 is better than indiv2
    if(indiv1->value != indiv2->value){
        // if they have different fitness values , the one with the lowest sum is better
        return indiv1->value < indiv2->value;
    }
    // otherwise the one with the maximum size set is better
    return indiv1->nb > indiv2->nb;
}
int eq(Individu *  indiv1 , Individu * indiv2, int l){
    for(int i=0;i<l;i++){
        if(indiv1->DNA[i]!=indiv2->DNA[i])return 0;
    }
    return 1;
}

int checkIndiv(Individu * indiv, int l){
    // to check if an individual have an empty set or not
    int cnt=0;
    for(int i=0;i<l;i++){
        if(indiv->DNA[i]==1)cnt++;
    }
    // return true if it's valid (not empty) otherwise false
    return (cnt!=l);
}


void printIndiv(Individu * indiv, int *t, int l){
    // function to show an individual
   printf("The score of this individual is %d  : ", indiv->value);
   int cnt=0;
    for(int i=0;i<l;i++){
        if(indiv->DNA[i]==1){
            cnt++;
            continue;
        }
        if(indiv->DNA[i]==0){
            printf(" - %d ", t[i]);
        }else{
            printf(" + %d ", t[i]);
        }
    }
    printf("\n");
    printf("set size = %d",l-cnt);
    printf("\n");
}



void printIndivForInterface(Individu * indiv, int *t, int l){
    // function to show an individual
   int cnt=0;
    for(int i=0;i<l;i++){
        if(indiv->DNA[i]==1){
            cnt++;
            continue;
        }
        if(indiv->DNA[i]==0){
            printf(" - %d ", t[i]);
        }else{
            printf(" + %d ", t[i]);
        }
    }
}

