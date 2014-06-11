#include "exec.h"

//! retourne True si l'instruction est immédiate sinon false.
/*!
 * \param instr instruction en cours
 * \param addr adresse de l'instruction
 */
bool is_immediate(Instruction instr, unsigned addr) 
{
	if (instr.instr_generic._immediate)
		return true;
	return false;
}

//! Récupère l'adresse réelle, à partir d'un adressage indexé ou absolu.
/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 */
unsigned int get_address(Machine *pmach, Instruction instr) 
{	
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
void check_seg_data(Machine *pmach, unsigned int data_addr, unsigned addr) 
{
	if (data_addr > pmach->_datasize)
		error(ERR_SEGDATA, addr);
}

//! Met à jour CC selon la valeur de reg.
/*!
 * \param pmach machine en cours d'exécution
 * \param reg numéro de registre
 */
void refresh_cc(Machine *pmach, unsigned int reg) 
{
	if (reg < 0) //Si résultat nul
	        pmach->_cc = CC_N;
    	else if (reg > 0) // positif
        	pmach->_cc = CC_P;
    	else // zéro
        	pmach->_cc = CC_Z;
}

//! Décode et exécute de l'instruction LOAD.
//! Adressage immédiat, absolu et indexé pour la source.
//! Il faut indiquer un registre de destination.
/*!
 * \param pmach machine en cours d'exécution
 * \param instr instruction en cours
 * \param addr adresse de l'instruction en cours
 */
bool load(Machine *pmach, Instruction instr, unsigned addr) {
	if (is_immediate(instr,addr)) { // Si I = 1 : Immediat, on est en immediat
		pmach->_registers[instr.instr_generic._regcond] = instr.instr_immediate._value;
	} else {
		unsigned int address = get_address(pmach, instr);
		check_seg_data(pmach, address, addr);
		pmach->_registers[instr.instr_generic._regcond] = pmach->_data[address];
	}
	refresh_cc(pmach, pmach->_registers[instr.instr_generic._regcond]);
	return true;
}

bool store(Machine *pmach, Instruction instr, unsigned addr) {


}

bool add(Machine *pmach, Instruction instr, unsigned addr) {


}

bool sub(Machine *pmach, Instruction instr, unsigned addr) {


}

bool branch(Machine *pmach, Instruction instr, unsigned addr) {


}

bool call(Machine *pmach, Instruction instr, unsigned addr) {


}

bool ret(Machine *pmach, Instruction instr, unsigned addr) {


}

bool push(Machine *pmach, Instruction instr, unsigned addr) {


}

bool pop(Machine *pmach, Instruction instr, unsigned addr) {


}

bool decode_execute(Machine *pmach, Instruction instr) {
	switch (instr.instr_generic._cop) {
		case ILLOP :
			error(ERR_ILLEGAL, pmach->pc-1);
		case NOP :
			return true;
		case LOAD :
			return load(pmach, instr, pmach->pc-1);
		case STORE :
			return store(pmach, instr, pmach->pc-1);
		case ADD :
			return add(pmach, instr, pmach->pc-1);
		case SUB :
			return sub(pmach, instr, pmach->pc-1);
		case BRANCH :
			return branch(pmach, instr, pmach->pc-1);
		case CALL :
			return call(pmach, instr, pmach->pc-1);
		case RET :
			return ret(pmach, instr, pmach->pc-1);
		case PUSH :
			return push(pmach, instr, pmach->pc-1);
		case POP :
			return pop(pmach, instr, pmach->pc-1);
		case HALT :
			warning(WARN_HALT, pmach->pc-1);
			return false;
		default :
			error(ERR_UNKNOWN, pmach->pc-1);
	}
}


void trace(const char *msg, Machine *pmach, Instruction instr, unsigned addr) {
	printf("TRACE: %s: 0x%04x: ",msg, addr);
	print_instruction(instr, addr);
	printf("\n");
}
