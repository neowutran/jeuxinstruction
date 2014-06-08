#include "machine.h"
#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>


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

void load_program(Machine *pmach,
        unsigned textsize, Instruction text[textsize],
        unsigned datasize, Word data[datasize], unsigned dataend) {

    pmach->_dataend = dataend;
    pmach->_data = data;
    pmach->_datasize = datasize;
    pmach->_textsize = textsize;
    pmach->_text = text;
    for (int i = 0; i < NREGISTERS; i++) {
        pmach->_registers[i] = 0;
    }
    pmach->_pc = 0;
    pmach->_cc = CC_U;
}


//! Lecture d'un programme depuis un fichier binaire

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
    FILE* fp = fopen(programfile, "r");
    if (fp == NULL) {
        perror("machine.c");
        exit(1);
    }
    unsigned int textsize;
    unsigned int datasize;
    unsigned int dataend;
    fread(&textsize, sizeof (unsigned int), 1, fp);
    fread(&datasize, sizeof (unsigned int), 1, fp);
    fread(&dataend, sizeof (unsigned int), 1, fp);
    Instruction text[textsize];
    Word data[datasize];
    fread(text, sizeof (Instruction), textsize, fp);
    fread(data, sizeof (Word), datasize, fp);
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

    printf("\nMemory dump\n");
    print_program(pmach);
    print_cpu(pmach);
    print_data(pmach);
    printf("\n");

}


//! Affichage des instructions du programme

/*!
 * Les instructions sont affichées sous forme symbolique, précédées de leur adresse.
.*
 * \param pmach la machine en cours d'exécution
 */
void print_program(Machine *pmach) {
    printf("\n*** INSTRUCTION ***\n");
    for (int i = 0; i < pmach->_textsize; i++) {
        printf("%08X %08X\n", i, pmach->_text[i]);
    }
}

//! Affichage des données du programme

/*!
 * Les valeurs sont affichées en format hexadécimal et décimal.
 *
 * \param pmach la machine en cours d'exécution
 */
void print_data(Machine *pmach) {
    printf("\n*** DATA (size: %d, end = 0x%08X (%d)) ***\n", pmach->_datasize, pmach->_dataend, pmach->_dataend);
    for (int i = 0; i < pmach->_datasize; i++) {
        printf("0x%04X: 0x%08X %d", i, pmach->_data[i], pmach->_data[i]);
        if (i % 3 == 0) {
            printf("\n");
        }
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
    //Affichage des registres
    printf("\n*** CPU ***\n");
    printf("PC: 0x%08X CC: ", pmach->_pc);
    switch (pmach->_pc) {
        case CC_P:
            printf("P\n");
            break;
        case CC_Z:
            printf("Z\n");
            break;
        case CC_N:
            printf("N\n");
            break;
        case CC_U:
            printf("U\n");
            break;
    }
    for (int i = 0; i < NREGISTERS; i++) {
        printf("R%d: 0x%08X\n", i, pmach->_registers[i]);
    }
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

    while (pmach->_pc < pmach->_textsize - 1) {
        decode_execute(pmach, pmach->_text[pmach->_pc]);
    }
}