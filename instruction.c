/*!
 * \file instruction.c
 * \brief Description du jeu d'instruction.
 */

#include "instruction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! tableau rassemblant les différentes operations possibles 
const char* cop_names[]={"ILLOP","NOP","LOAD","STORE","ADD","SUB","BRANCH","CALL","RET","PUSH","POP","HALT"};

//! tableau rassemblant les conditions possibles poue BRANCH et CALL
const char* condition_names[]={"NC","EQ","NE","GT","GE","LT","LE"};

//! affiche le registre d'une instruction de façon lisible
//! affiche R0x si x<=9 et Rx sinon
/*!
 *  \param instr l'instruction dont on veut afficher le registre
 */	
static void affichage_registre(Instruction instr){
	printf("R%02d", (int) instr.instr_generic._regcond);
}

//! affiche une instruction sous forme lisible
/*!
 * \param instr l'instruction a afficher
 * \param addr  l'adresse de l'instruction
 */
void print_instruction(Instruction instr, unsigned addr){
	
	//On recupere la condition
	char *cop_name=cop_names[instr.instr_generic._cop];

	//On la compare afin de savoir quoi afficher
	//Si l'operation est HALT, ILLOP, NOP ou RET, il suffit de l'afficher
	if(strcmp(cop_name,"HALT")==0 || strcmp(cop_name,"ILLOP")==0 || strcmp(cop_name,"NOP")==0 || strcmp(cop_name,"RET")==0){
		printf("%s", cop_name);
	}else if(!instr.instr_generic._immediate){	//si I=0 
		//si X=0 : adressage direct
		if(!instr.instr_generic._indexed){
			//si l'operation est BRACH ou CALL, on affiche la condition, sinon on affiche le registre de façon lisible
			if(strcmp(cop_name,"BRANCH")==0 || strcmp(cop_name,"CALL")==0){
				printf("%s %s, @%04x", cop_name, condition_names[instr.instr_generic._regcond], (int) instr.instr_absolute._address);
			}else{	
				printf("%s ",cop_name);
				affichage_registre(instr);
				printf(", @%04x", (int) instr.instr_absolute._address);
			}
		//si X=1 : adressage indexe
		}else{
			if(strcmp(cop_name,"BRANCH")==0 || strcmp(cop_name,"CALL")==0){
				printf("%s %s", cop_name, condition_names[instr.instr_generic._regcond]);
				//offset sous la forme +/-offset
				printf(", %+d[", (int) instr.instr_indexed._offset);
				//on affiche le registre pour l'affichage indirect
				printf("R%02d]", (int) instr.instr_indexed._rindex);
			}else{
				printf("%s ", cop_name);
				affichage_registre(instr);
				printf(", %+d[", (int) instr.instr_indexed._offset);
				printf("R%02d]", (int) instr.instr_indexed._rindex);
			}
			
		}
	//si I=1 : immediat
	}else{
		if(strcmp(cop_name,"BRANCH")==0 || strcmp(cop_name,"CALL")==0){
			printf("%s %s, #%d", cop_name, condition_names[instr.instr_generic._regcond], (int) instr.instr_immediate._value);
		}else{
			printf("%s ", cop_name);
			affichage_registre(instr);
			printf(", #%d", instr.instr_immediate._value);
		}
	}
}



