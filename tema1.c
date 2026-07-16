/* STOICA Izabela - 315CC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct unit {
    int id;
    char type;
    int availability;
};

struct incident {
    int id;
    char priority[7];
    char *description;
    char status[11];
    //adaugam nodurile urmator si anterior pentru a obtine
    //o lista dublu inlantuita
    struct incident *urm;
    struct incident *ant;
};

struct intervention {
    struct incident *incident;
    struct unit *unit;
    struct intervention *urm;
    struct intervention *ant;
};

//datele necesare pentru cozi
struct NodeQueue {
    struct incident *incident;
    struct NodeQueue *urm;
};

struct Queue {
    struct NodeQueue *head;
    struct NodeQueue *tail;
    int len;
};

//datele necesare pentru coada de echipaje
struct NodeUnit {
    struct unit *unit;
    struct NodeUnit *urm;
};

struct Queue2 {
    struct NodeUnit *head;
    struct NodeUnit *tail;
    int len;
};

struct Stack {
    struct intervention *intervention;
    struct Stack *next;
};

struct system {
    struct unit units[50];
    int nr_units;
    struct incident *incidents;
    struct intervention *interventions;
    //cele 3 cozi
    struct Queue queue_high;
    struct Queue queue_medium;
    struct Queue queue_low;
    struct Queue2 queue_available_units;
    //stiva
    struct Stack *stiva;
};

//functie care initializeaza toate componentele sistemului
void initializare_sistem(struct system *sistem)
{
    sistem->nr_units = 0;
    sistem->incidents = (struct incident*)malloc(sizeof(struct incident));
    //initializarea santinelei pt lista de incidente
    sistem->incidents->id = 0;
    strcpy(sistem->incidents->priority, "low");
    sistem->incidents->description = (char*)malloc(strlen("test") + 1);
    strcpy(sistem->incidents->description, "test");
    strcpy(sistem->incidents->status, "solved");
    sistem->incidents->urm = sistem->incidents;
    sistem->incidents->ant = sistem->incidents;
    //lista interventii tot cu santinela
    sistem->interventions = (struct intervention*)malloc(sizeof(struct intervention));
    sistem->interventions->urm = sistem->interventions;
    sistem->interventions->ant = sistem->interventions;
    //initializare cele 3 cozi
    //+coada de echipaje disponibile
    sistem->queue_high.len = 0;
    sistem->queue_medium.len = 0;
    sistem->queue_low.len = 0;
    sistem->queue_available_units.len = 0;
    sistem->queue_high.head = NULL;
    sistem->queue_high.tail = NULL;
    sistem->queue_medium.head = NULL;
    sistem->queue_medium.tail = NULL;
    sistem->queue_low.head = NULL;
    sistem->queue_low.tail = NULL;
    sistem->queue_available_units.head = NULL;
    sistem->queue_available_units.tail = NULL;
    //stiva
    sistem->stiva = NULL;
}

//functie push pt stiva, pentru a adauga interventii
void adauga_in_stiva(struct system *sistem, struct intervention *intv)
{
    struct Stack *nou = (struct Stack*)malloc(sizeof(struct Stack));
    nou->intervention = intv;
    nou->next = sistem->stiva;
    sistem->stiva = nou;
}

void ADD_INCIDENT(struct system *sistem, int id, char *priority, char *description)
{
    //alocare memorie pt un incident nou
    struct incident *incident_nou = (struct incident *)malloc(sizeof(struct incident));
    incident_nou->id = id;
    strcpy(incident_nou->priority, priority);
    incident_nou->description = (char*)malloc(strlen(description) + 1);
    strcpy(incident_nou->description, description);
    strcpy(incident_nou->status, "queued");

    //inserare la finalul listei de incidente
    //folosim santinela si faptul ca lista noastra e dublu inlantuita
    struct incident *santinela = sistem->incidents;
    struct incident *ultimul = santinela->ant;
    ultimul->urm = incident_nou;
    incident_nou->ant = ultimul;
    incident_nou->urm = santinela;
    santinela->ant = incident_nou;

    //bagare in coada
    struct NodeQueue *nou = (struct NodeQueue*)malloc(sizeof(struct NodeQueue));
    struct Queue *coada = NULL;
    nou->urm = NULL;
    nou->incident = incident_nou;

    //selectam coada cu prioritatea pe care o are incidentul
    if(!strcmp(priority, "low"))
        coada = &sistem->queue_low;
    else if(!strcmp(priority, "medium"))
        coada = &sistem->queue_medium;
    else if(!strcmp(priority, "high"))
        coada = &sistem->queue_high;
    coada->len++;
    if(coada->tail == NULL) { //daca coada e goala
        coada->head = nou;
        coada->tail = nou;
    }
    else { //daca coada mai are deja incidente
        coada->tail->urm = nou;
        coada->tail = nou;
    }
}

//functie de push in coada de echipaje disponibile
void adauga_in_queue_available_units(struct system *sistem, struct unit *unit)
{
    struct Queue2 *coada = &sistem->queue_available_units;
    struct NodeUnit *nou = (struct NodeUnit*)malloc(sizeof(struct NodeUnit));
    nou->unit = unit;
    nou->urm = NULL;
    if(coada->tail == NULL) {
        coada->head = nou;
        coada->tail = nou;
    } else {
        coada->tail->urm = nou;
        coada->tail = nou;
    }
    coada->len++;
}

//functie pt afisarea echipajelor
void SHOW_UNIT(struct system *sistem, int id, FILE *f)
{
    int gasit = 0;
    for(int i = 0; i < sistem->nr_units; i++)
        if(sistem->units[i].id == id) {
            gasit = 1;
            if(sistem->units[i].availability)
                fprintf(f, "Unit %d is type %c and is available\n", id, sistem->units[i].type);
            else
                fprintf(f, "Unit %d is type %c and is unavailable\n", id, sistem->units[i].type);
        }
    if(!gasit)
        fprintf(f, "INVALID OPERATION! ERROR 404\n");
}

//functie pt afisarea incidentelor
void SHOW_INCIDENT(struct system *sistem, int id, FILE *f)
{
    int gasit = 0;
    struct incident *curent = sistem->incidents->urm;
    while(curent != sistem->incidents) {
        if(curent->id == id) {
            fprintf(f, "Incident %d has %s priority, the following description: %s and is %s\n", id, curent->priority, curent->description, curent->status);
            gasit = 1;
            break;
        }
        curent = curent->urm;
    }
    if(!gasit)
        fprintf(f, "INVALID OPERATION! ERROR 404\n");
}

//functie pentru citirea datelor din instructiunile ADD_INCIDENT
//pentru a nu le mai citi in main
void citire_add_incident(FILE *f, struct system *sistem)
{
    int id;
    char priority[7];
    char description[30];
    fscanf(f, "%d %s", &id, priority);
    fgetc(f);
    fscanf(f, "%[^\n]", description);
    fgetc(f);
    ADD_INCIDENT(sistem, id, priority, description);
}

void CHECK_UNITS_AVAILABILITY(struct system *sistem, FILE *f)
{
    //tinem constant cont de lungimea cozii, 
    //asadar putem afisa direct cate echipaje disponibile sunt
    fprintf(f, "Number of available units: %d\n", sistem->queue_available_units.len);
}

void DISPATCH(struct system *sistem, FILE *f)
{
    //selectam coada cu prioritatea corespunzatoare incidentului nostru
    struct Queue *q = NULL;
    if(sistem->queue_high.len)
        q = &sistem->queue_high;
    else if(sistem->queue_medium.len)
        q = &sistem->queue_medium;
    else if(sistem->queue_low.len)
        q = &sistem->queue_low;
    else { //daca nu mai sunt incidente in coada, da eroare
        fprintf(f, "INVALID OPERATION! ERROR 404\n");
        return;
    }
    if(sistem->queue_available_units.len == 0) { //daca nu sunt echipaje disponibile, eroare
        fprintf(f, "INVALID OPERATION! ERROR 404\n");
        return;
    }

    //extragem incidentul din coada
    struct NodeQueue *tmp = q->head;
    struct incident *incid = tmp->incident;
    q->head = q->head->urm;
    if(q->head == NULL) q->tail = NULL;
    free(tmp);
    q->len--;

    //extragem primul echipaj din coada
    struct NodeUnit *temp = sistem->queue_available_units.head;
    struct unit *unit_curent = temp->unit;
    sistem->queue_available_units.head = temp->urm;
    if(sistem->queue_available_units.head == NULL)
        sistem->queue_available_units.tail = NULL;
    free(temp);
    sistem->queue_available_units.len--;

    //marcam incidentul si echipajul
    strcpy(incid->status, "intervened");
    unit_curent->availability = 0; //unavailable

    //cream interventia la finalul listei
    struct intervention *intv_curent = (struct intervention*)malloc(sizeof(struct intervention));
    intv_curent->incident = incid;
    intv_curent->unit = unit_curent;
    struct intervention *santinela = sistem->interventions;
    struct intervention *ultim = santinela->ant;
    ultim->urm = intv_curent;
    intv_curent->ant = ultim;
    intv_curent->urm = santinela;
    santinela->ant = intv_curent;
    adauga_in_stiva(sistem, intv_curent);
}

void UNDO_LAST_DISPATCH(struct system *sistem, FILE *f)
{
    //parcurgem stiva pana ajungem la o interventie cu un incident cu status != solved
    struct Stack *curent = sistem->stiva;
    struct Stack *ant = NULL;
    while(curent && !strcmp(curent->intervention->incident->status, "solved")) {
        ant = curent;
        curent = curent->next;
    }
    if(curent == NULL) {
        fprintf(f, "INVALID OPERATION! ERROR 404\n");
        return;
    }

    //scoatem din stiva
    if(ant == NULL)
        sistem->stiva = curent->next;
    else
        ant->next = curent->next;
    struct intervention *intv = curent->intervention;
    struct incident *inc_curent = curent->intervention->incident;
    free(curent);

    //scoatem din lista de interventii
    intv->ant->urm = intv->urm;
    intv->urm->ant = intv->ant;

    //refacem si plasam incidentul la inceputul cozii sale
    //in functie de gradul de prioritate
    strcpy(inc_curent->status, "queued");
    struct Queue *q = NULL;
    if(!strcmp(inc_curent->priority, "high"))
        q = &sistem->queue_high;
    else if(!strcmp(inc_curent->priority, "medium"))
        q = &sistem->queue_medium;
    else
        q = &sistem->queue_low;
    struct NodeQueue *nou = (struct NodeQueue*)malloc(sizeof(struct NodeQueue));
    nou->incident = inc_curent;
    nou->urm = q->head;
    q->head = nou;
    if(q->tail == NULL)
        q->tail = nou;
    q->len++;

    //introducem echipajul la finalul cozii de disponibilitate
    struct unit *unitate_curenta = intv->unit;
    unitate_curenta->availability = 1;
    adauga_in_queue_available_units(sistem, unitate_curenta);
    free(intv);
}

void SOLVED_INCIDENT(struct system *sistem, int id, FILE *f)
{
    int gasit = 0;
    struct intervention *curent = sistem->interventions->urm;
    while(curent != sistem->interventions) {
        if(curent->incident->id == id) {
            gasit = 1;
            break;
        }
        curent = curent->urm;
    }
    if(!gasit) { //daca incidentul cu id-ul specificat nu e in sistem, eroare
        fprintf(f, "INVALID OPERATION! ERROR 404\n");
        return;
    }
    if(strcmp(curent->incident->status, "intervened")) { //daca e in sistem, dar nu are statusul corespunzator, eroare
        fprintf(f, "INVALID OPERATION! ERROR 404\n");
        return;
    }
    strcpy(curent->incident->status, "solved");
    struct unit *unitate_curenta = curent->unit;
    unitate_curenta->availability = 1;
    adauga_in_queue_available_units(sistem, unitate_curenta);
}

//functie pt afisarea interventiilor din sistem
void SHOW_INTERVENTIONS(struct system *sistem, FILE *f)
{
    struct intervention *curent = sistem->interventions->urm;
    if(curent == sistem->interventions) {
        fprintf(f, "No intervention has been initiated\n");
    }
    while(curent != sistem->interventions) {
        fprintf(f, "Incident %d was assigned to unit %d, and has the following status: \"%s\"\n", curent->incident->id, curent->unit->id, curent->incident->status);
        curent = curent->urm;
    }
}

int main()
{
    struct system sistem;
    initializare_sistem(&sistem);
    FILE *f = fopen("tema1.in", "r");
    FILE *f2 = fopen("tema1.out", "w");
    int n, i, t;
    fscanf(f, "%d", &n);
    sistem.nr_units = n;

    for(i = 0; i < n; i++) {
        fscanf(f, "%d %c", &sistem.units[i].id, &sistem.units[i].type);
        sistem.units[i].availability = 1;
        //adaugam unitatea in coada
        adauga_in_queue_available_units(&sistem, &sistem.units[i]);
    }
    fscanf(f, "%d", &t);
    for(i = 0; i < t; i++) {
        char instr[100];
        fscanf(f, "%s", instr);
        //in functie de comanda citita, realizam operatiile necesare
        if(!strcmp(instr, "ADD_INCIDENT")) {
            citire_add_incident(f, &sistem);
        }
        else if(!strcmp(instr, "SHOW_INCIDENT")) {
            int id;
            fscanf(f, "%d", &id);
            SHOW_INCIDENT(&sistem, id, f2);
        }
        else if(!strcmp(instr, "SHOW_UNIT")) {
            int id;
            fscanf(f, "%d", &id);
            SHOW_UNIT(&sistem, id, f2);
        }
        else if(!strcmp(instr, "CHECK_UNITS_AVAILABILITY")) {
            CHECK_UNITS_AVAILABILITY(&sistem, f2);
        }
        else if(!strcmp(instr, "DISPATCH")) {
            DISPATCH(&sistem, f2);
        }
        else if(!strcmp(instr, "UNDO_LAST_DISPATCH")) {
            UNDO_LAST_DISPATCH(&sistem, f2);
        }
        else if(!strcmp(instr, "SOLVED_INCIDENT")) {
            int id;
            fscanf(f, "%d", &id);
            SOLVED_INCIDENT(&sistem, id, f2);
        }
        else if(!strcmp(instr, "SHOW_INTERVENTIONS")) {
            SHOW_INTERVENTIONS(&sistem, f2);
        }
    }

    //eliberarea intregului sistem

    //eliberarea incidentelor
    struct incident *curent = sistem.incidents->urm;
    while(curent != sistem.incidents) {
        struct incident *aux = curent;
        curent = curent->urm;
        free(aux->description);
        free(aux);
    }
    free(sistem.incidents->description);
    free(sistem.incidents);

    //eliberarea interventiilor
    struct intervention *curent2 = sistem.interventions->urm;
    while(curent2 != sistem.interventions) {
        struct intervention *aux = curent2;
        curent2 = curent2->urm;
        free(aux);
    }
    free(sistem.interventions);

    //eliberarea stivei
    while(sistem.stiva) {
        struct Stack *aux = sistem.stiva;
        sistem.stiva = sistem.stiva->next;
        free(aux);
    }

    //eliberarea cozilor
    struct NodeQueue *q;
    q = sistem.queue_high.head;
    while(q) {
        struct NodeQueue *aux = q;
        q = q->urm;
        free(aux);
    }
    q = sistem.queue_medium.head;
    while(q) {
        struct NodeQueue *aux = q;
        q = q->urm;
        free(aux);
    }
    q = sistem.queue_low.head;
    while(q) {
        struct NodeQueue *aux = q;
        q = q->urm;
        free(aux);
    }

    //eliberarea unitatilor
    struct NodeUnit *u = sistem.queue_available_units.head;
    while(u) {
        struct NodeUnit *aux = u;
        u = u->urm;
        free(aux);
    }
    //inchiderea fisierelor
    fclose(f);
    fclose(f2);
    return 0;
}