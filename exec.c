/*!
 * \fichier exec.c
 * \brief Exécution des instructions pour la simulation du processeur.
 */

#include "exec.h"
#include "error.h"
#include <stdio.h>

//! retourne True si l'instruction est immédiate sinon false.

/*!
 * \param instr instruction en cours
 * \param addr adresse de l'instruction
 */
static bool is_immediate(Instruction instr, unsigned addr) {
    if (instr.instr_generic._immediate)
        return true;
    return false;
}

//! Vérifie que l'instruction n'est pas en immédiate sinon erreur

/*!
 * \param instr instruction en cours
 * \param addr adresse de l'instruction
 */
static void check_immediate(Instruction instr, unsigned addr) {
    if (is_immediate(instr, addr)) {
        error(ERR_IMMEDIATE, addr);
    }
}

//! Récupère l'adresse réelle, à partir d'un adressage indexé ou absolu.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 */
static unsigned int get_address(Machine *pmach, Instruction instr) {
    if (instr.instr_generic._indexed) {
        return pmach->_registers[instr.instr_indexed._rindex] + instr.instr_indexed._offset;
    }
    return instr.instr_absolute._address;
}

//! Vérifie qu'il n'y a pas d'ERR_SEGDATA sur le tableau des données.

/*!
 * \param pmach machine en cours d'exécution
 * \param data_addr adresse réelle
 * \param addr adresse de l'instruction en cours
 */
static void check_seg_data(Machine *pmach, unsigned int data_addr, unsigned addr) {
    if (data_addr > pmach->_datasize) {
        error(ERR_SEGDATA, addr);
    }
}

//! Change la valeur de CC selon la valeur de reg.

/*!
 * \param pmach machine en cours d'exécution
 * \param reg numéro de registre
 */
void change_cc(Machine *pmach, unsigned int reg) {
    if (reg < 0) { //Si résultat nul
        pmach->_cc = CC_N;
    } else if (reg > 0) { // positif
        pmach->_cc = CC_P;
    } else { // zéro
        pmach->_cc = CC_Z;
    }
}

//! Vérification de la condition de branchement.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 */
static bool check_condition(Machine *pmach, Instruction instr, unsigned addr) {
    switch (instr.instr_generic._regcond) {
        case NC: // Pas de condition
            return true;
        case EQ: // Egal à 0
            return (pmach->_cc == CC_Z);
        case NE: // Different de zero
            return (pmach->_cc != CC_Z);
        case GT: // Strictement positif
            return (pmach->_cc == CC_P);
        case GE: // Positif ou nul
            return (pmach->_cc == CC_P || pmach->_cc == CC_Z);
        case LT: // Strictement négatif
            return (pmach->_cc == CC_N);
        case LE: // Négatif ou nul
            return (pmach->_cc == CC_N || pmach->_cc == CC_Z);
        default:
            error(ERR_CONDITION, addr);
    }
}

//! Vérification que Stack Pointer (SP) ne dépasse pas la zone dédiée à la pile.

/*!
 * \param pmach machine en cours d'exécution
 * \param addr adresse de l'instruction
 */
static void check_stack(Machine *pmach, unsigned addr) {
    if (pmach->_sp < pmach->_dataend || pmach->_sp >= pmach->_datasize)
        error(ERR_SEGSTACK, addr);
}

//! Décodage et exécution de l'instruction LOAD.
//! Adressage immédiat, absolu et indexé pour la source.
//! Il faut indiquer un registre de destination.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool load(Machine *pmach, Instruction instr, unsigned addr) {
    if (is_immediate(instr, addr)) { // Si I = 1 : Immediat, on est en immediat, on load directement la valeur dans le registre
        pmach->_registers[instr.instr_generic._regcond] = instr.instr_immediate._value;
    } else { // on va chercher la valeur dans data pour la mettre dans le registre
        unsigned int address = get_address(pmach, instr);
        check_seg_data(pmach, address, addr);
        pmach->_registers[instr.instr_generic._regcond] = pmach->_data[address];
    }
    change_cc(pmach, pmach->_registers[instr.instr_generic._regcond]);
    return true;
}

//! Décodage et exécution de l'instruction STORE.
//! Adressage absolu et indexé pour la destination.
//! Indiquer un registre pour la source.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool store(Machine *pmach, Instruction instr, unsigned addr) {
    check_immediate(instr, addr); // vérifie que l'on est pas en immédiat, sinon erreur
    unsigned int address = get_address(pmach, instr);
    check_seg_data(pmach, address, addr);
    pmach->_data[address] = pmach->_registers[instr.instr_generic._regcond]; // on stocke la valeur du registre dans data
    return true;
}

//! Décodage et exécution de l'instruction ADD.
//! Adressage immédiat, absolu et indexé pour la source.
//! Indiquer un registre pour la destination.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool add(Machine *pmach, Instruction instr, unsigned addr) {
    if (is_immediate(instr, addr)) { // Si I = 1 : Immediat, on est en immediat, on ajoute la valeur immédiatement
        pmach->_registers[instr.instr_generic._regcond] += instr.instr_immediate._value;
    } else { // on récupère la valeur dans data et on l'ajoute au registre
        unsigned int address = get_address(pmach, instr);
        check_seg_data(pmach, address, addr);
        pmach->_registers[instr.instr_generic._regcond] += pmach->_data[address];
    }
    change_cc(pmach, pmach->_registers[instr.instr_generic._regcond]);
    return true;
}

//! Décodage et exécution de l'instruction SUB.
//! Adressage immédiat, absolu et indexé pour la source.
//! Indiquer un registre pour la destination.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool sub(Machine *pmach, Instruction instr, unsigned addr) {
    if (is_immediate(instr, addr)) { // Si I = 1 : Immediat, on est en immediat, on enlève directement la valeur
        pmach->_registers[instr.instr_generic._regcond] -= instr.instr_immediate._value;
    } else { // on récupère la valeur dans data et on la soustrait
        unsigned int address = get_address(pmach, instr);
        check_seg_data(pmach, address, addr);
        pmach->_registers[instr.instr_generic._regcond] -= pmach->_data[address];
    }
    change_cc(pmach, pmach->_registers[instr.instr_generic._regcond]);
    return true;
}

//! Décodage et exécutution de l'instruction BRANCH.
//! Adressage absolu et indexé pour l'adresse de l'instruction à exécuter.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool branch(Machine *pmach, Instruction instr, unsigned addr) {
    check_immediate(instr, addr); // vérifie que l'on est pas en immédiat, sinon erreur
    if (check_condition(pmach, instr, addr)) {
        unsigned int address = get_address(pmach, instr);
        pmach->_pc = address; //on jump à la suite du programme
    }
    return true;
}

//! Décodage et exécution de l'instruction CALL.
//! Adressage absolu et indexé pour l'adresse du sous-programme.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool call(Machine *pmach, Instruction instr, unsigned addr) {
    check_immediate(instr, addr); // vérifie que l'on est pas en immédiat, sinon erreur
    check_stack(pmach, addr);

    if (check_condition(pmach, instr, addr)) {
        pmach->_data[pmach->_sp--] = pmach->_pc;
        unsigned int address = get_address(pmach, instr);
        pmach->_pc = address; // on jump a l'adresse du sous programme
    }
    return true;
}

//! Décodage et exécutution de l'instruction RET.
//! RET est employé seul.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool ret(Machine *pmach, Instruction instr, unsigned addr) {
    ++pmach->_sp;
    check_stack(pmach, addr);
    pmach->_pc = pmach->_data[pmach->_sp]; //retour à l'endroit du programme on l'on était avant le CALL
    return true;
}

//! Décodage et exécution de l'instruction PUSH.
//! Adresse immédiat, indexé et absolu.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool push(Machine *pmach, Instruction instr, unsigned addr) {
    check_stack(pmach, addr);
    if (is_immediate(instr, addr)) { // Si I = 1 : Immediat, on est en immediat, on push directement la valeur
        pmach->_data[pmach->_sp--] = instr.instr_immediate._value;
    } else { // on récupère la valeur dans data et on la push
        unsigned int address = get_address(pmach, instr);
        check_seg_data(pmach, address, addr);
        pmach->_data[pmach->_sp--] = pmach->_data[address];
    }
    return true;
}

//! Décodage et exécution de l'instruction POP.
//! Adresse absolue ou indexée.

/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
static bool pop(Machine *pmach, Instruction instr, unsigned addr) {
    check_immediate(instr, addr); // vérifie que l'on est pas en immédiat, sinon erreur
    unsigned int address = get_address(pmach, instr);
    check_seg_data(pmach, address, addr);
    ++pmach->_sp;
    check_stack(pmach, addr);
    pmach->_data[address] = pmach->_data[pmach->_sp]; //On met la valeur de la pile dans Data
    return true;
}

bool decode_execute(Machine *pmach, Instruction instr) {
    switch (instr.instr_generic._cop) {
        case ILLOP:
            error(ERR_ILLEGAL, pmach->_pc - 1);
        case NOP:
            return true;
        case LOAD:
            return load(pmach, instr, pmach->_pc - 1);
        case STORE:
            return store(pmach, instr, pmach->_pc - 1);
        case ADD:
            return add(pmach, instr, pmach->_pc - 1);
        case SUB:
            return sub(pmach, instr, pmach->_pc - 1);
        case BRANCH:
            return branch(pmach, instr, pmach->_pc - 1);
        case CALL:
            return call(pmach, instr, pmach->_pc - 1);
        case RET:
            return ret(pmach, instr, pmach->_pc - 1);
        case PUSH:
            return push(pmach, instr, pmach->_pc - 1);
        case POP:
            return pop(pmach, instr, pmach->_pc - 1);
        case HALT:
            warning(WARN_HALT, pmach->_pc - 1);
            return false;
        default:
            error(ERR_UNKNOWN, pmach->_pc - 1);
    }
}

void trace(const char *msg, Machine *pmach, Instruction instr, unsigned addr) {
    printf("TRACE: %s: 0x%04x: ", msg, addr);
    print_instruction(instr, addr);
    printf("\n");
}
