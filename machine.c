#include "machine.h"
#include "exec.h"
#include "instruction.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "debug.h"

Instruction* instructionToFree;
Word * dataToFree;
static int needFree = 0;
//! Chargement d'un programme

/*!
 * La machine est réinitialisée et ses segments de texte et de données sont
 * remplacés par ceux fournis en paramètre.
 *
 * \param pmach la machine en cours d'exécution
 * \param textsize taille utile du segment de texte
 * \param text le contenu du segment de texte
 * \param datasize taille utile du segment de données
 * \param data le contenu initial du segment de texte
 */
void load_program(Machine *pmach, unsigned textsize, Instruction text[textsize], unsigned datasize, Word data[datasize], unsigned dataend) {

    pmach->_textsize = textsize;
    pmach->_text = text;
    pmach->_datasize = datasize;
    pmach->_data = data;
    pmach->_dataend = dataend;
    pmach->_pc = 0;
    pmach->_cc = CC_U;


    for (int i = 0; i < NREGISTERS - 1; i++) {
        pmach->_registers[i] = 0x0;
    }
    pmach->_registers[15] = datasize - 1;
}

//! Lecture d'un programme depuis un fichier binaire

static void free_memory() {

    if (needFree == 1) {
        free(instructionToFree);
        free(dataToFree);
    }

}

/*!
 * Le fichier binaire a le format suivant :
 *
 *    - 3 entiers non signés, la taille du segment de texte (\c textsize),
 *    celle du segment de données (\c datasize) et la première adresse libre de
 *    données (\c dataend) ;
 *
 *    - une suite de \c textsize entiers non signés représentant le contenu du
 *    segment de texte (les instructions) ;
 *
 *    - une suite de \c datasize entiers non signés représentant le contenu initial du
 *    segment de données.
 *
 * Tous les entiers font 32 bits et les adresses de chaque segment commencent à
 * 0. La fonction initialise complétement la machine.
 *
 * \param pmach la machine à simuler
 * \param programfile le nom du fichier binaire
 *
 */
void read_program(Machine *mach, const char *programfile) {
    FILE * program = fopen(programfile, "r");
    if (program == NULL) {
        perror("machine.c");
        exit(1);
    }
    unsigned int textsize;
    unsigned int datasize;
    unsigned int dataend;

    fread(&textsize, sizeof (unsigned int), 1, program);
    fread(&datasize, sizeof (unsigned int), 1, program);
    fread(&dataend, sizeof (unsigned int), 1, program);

    Instruction* text = malloc(sizeof (Instruction) * textsize);
    Word *data = malloc(sizeof (Word) * datasize);

    fread(text, sizeof (Instruction), textsize, program);
    fread(data, sizeof (Word), datasize, program);
    if (ferror(program) != 0) {
        perror("machine");
        exit(1);
    }
    fclose(program);

    instructionToFree = text;
    dataToFree = data;
    needFree = 1;
    atexit(free_memory);

    load_program(mach, textsize, text, datasize, data, dataend);

}

//! Affichage du programme et des données

/*!
 * On affiche les instruction et les données en format hexadécimal, sous une
 * forme prête à être coupée-collée dans le simulateur.
 *
 * Pendant qu'on y est, on produit aussi un dump binaire dans le fichier
 * dump.prog. Le format de ce fichier est compatible avec l'option -b de
 * test_simul.
 *
 * \param pmach la machine en cours d'exécution
 */
void dump_memory(Machine *pmach) {

    FILE * dump = fopen("dump.prog", "w");
    if (dump == NULL) {
        perror("machine");
        exit(1);
    }
    fwrite(&pmach->_textsize, sizeof (unsigned int), 1, dump);
    fwrite(&pmach->_datasize, sizeof (unsigned int), 1, dump);
    fwrite(&pmach->_dataend, sizeof (unsigned int), 1, dump);
    fwrite(pmach->_text, sizeof (unsigned int), pmach->_textsize, dump);
    fwrite(pmach->_data, sizeof (unsigned int), pmach->_datasize, dump);
    if (ferror(dump) != 0) {
        perror("machine");
        exit(1);
    }
    fclose(dump);

    printf("Instruction text[] = {\n\t");
    for (int i = 0; i < pmach->_textsize; i++) {
        if (i % 4 == 0) {
            printf("\n\t");
        }
        printf("0x%08X,", pmach->_text[i]);

    }
    printf("\n");
    printf("};\n");
    printf("unsigned textsize = %d;\n", pmach->_textsize);
    printf("\nWord data[] = {\n\t");
    for (int i = 0; i < pmach->_datasize; i++) {
        if (i % 4 == 0) {
            printf("\n\t");
        }
        printf("0x%08X,", pmach->_data[i]);
    }
    printf("\n");
    printf("};\n");
    printf("unsigned datasize = %d;\n", pmach->_datasize);
    printf("unsigned dataend = %d;\n", pmach->_dataend);

}

//! Affichage des instructions du programme

/*!
 * Les instructions sont affichées sous forme symbolique, précédées de leur adresse.
.*
 * \param pmach la machine en cours d'exécution
 */
void print_program(Machine *pmach) {

    printf("\n\n*** PROGRAM (size: %d) ***\n", pmach->_textsize);
    for (int i = 0; i < pmach->_textsize; i++) {
        print_instruction(pmach->_text[i], i);
        printf("\n");
    }
}


//! Affichage des données du programme

/*!
 * Les valeurs sont affichées en format hexadécimal et décimal.
 *
 * \param pmach la machine en cours d'exécution
 */
void print_data(Machine *pmach) {

    printf("\n\n*** DATA (size: %d, end = %08X (%d)) ***\n", pmach->_datasize, pmach->_dataend, pmach->_dataend);
    for (int i = 0; i < pmach->_datasize; i++) {
        if (i % 3 == 0) {
            printf("\n");
        }
        printf("0x%04X: %08X %d\t", i, pmach->_data[i], pmach->_data[i]);
    }
    printf("\n");

}

//! Affichage des registres du CPU

/*!
 * Les registres généraux sont affichées en format hexadécimal et décimal.
 *
 * \param pmach la machine en cours d'exécution
 */
void print_cpu(Machine *pmach) {
    printf("\n\n*** CPU ***\n");
    printf("PC: %08X\t CC: ", pmach->_pc);
    switch (pmach->_cc) {
        case CC_U:
            printf("U");
            break;
        case CC_Z:
            printf("Z");
            break;
        case CC_N:
            printf("N");
            break;
        case CC_P:
            printf("P");
            break;
    }
    printf("\n");
    for (int i = 0; i < NREGISTERS; i++) {
        if (i % 3 == 0) {
            printf("\n");
        }
        if (i < 10) {
            printf("R0%d ", i);
        } else {
            printf("R%d ", i);
        }
        printf("0x%08X %d\t", pmach->_registers[i], pmach->_registers[i]);
    }
    printf("\n");

}


//! Simulation

/*!
 * La boucle de simualtion est très simple : recherche de l'instruction
 * suivante (pointée par le compteur ordinal \c _pc) puis décodage et exécution
 * de l'instruction.
 *
 * \param pmach la machine en cours d'exécution
 * \param debug mode de mise au point (pas à apas) ?
 */
void simul(Machine *pmach, bool debug) {
    int execute = 1;
    while (execute) {
        pmach->_pc = pmach->_pc + 1;
        trace("TRACE: Executing:", pmach, pmach->_text[pmach->_pc - 1], pmach->_pc - 1);
        execute = decode_execute(pmach, pmach->_text[pmach->_pc - 1]);
        if (debug) {
            debug = debug_ask(pmach);
        }
    }

}
