#include <stdio.h>
#include <ctype.h>
#include <string.h>
#define WORD int

extern char integral;
extern char linhas[25][81];

char vermelho22 = 0;
char vermelho23 = 0;

WORD decimal_point = '?';

void ava_integral(int y);

void ava_move(int y, int x) {

	fprintf(stderr, "ava_move %d %d\n", y, x);
/*
	int i;

	if (y > 24) {
		return;
	}
	for (i = x; linhas[y][i] == ' '; i++);

	//FILE *f=fopen("putty.log", "a");

	for (; linhas[y][i] && (i < 78); i++) {

		if (i == 0) {
			continue;
		}

		if (linhas[y][i] == ' ') {
			break;
		}

		if ((linhas[y][i] != '.') && (linhas[y][i] != ',')) {
			continue;
		}

		// 9x99
		if (isdigit(linhas[y][i-1])
			&& isdigit(linhas[y][i+1])
			&& isdigit(linhas[y][i+2])) {

			char ok = 1;
			char decimal = linhas[y][i];
			for (i=i+2; linhas[y][i] && (i < 80); i++) {
				
				if (linhas[y][i] == ' ') {
					break;
				}

				if (!isdigit(linhas[y][i])) {
					ok = 0;
					break;
				}
			}
					
			if (ok == 1) {
				decimal_point = decimal == '.' 
					? VK_OEM_PERIOD
					: VK_OEM_COMMA;
				break;
			}
		}
	}

	//fprintf(f, "move %d %d %d\n", y, x, decimal_point);
	//fprintf(f, "%s\n", linhas[y]+x);
	//fclose(f);
*/
	
}

WORD ava_converte_decimal(WORD tecla, int y, int x) {

	fprintf(stderr, "ava_converte_decimal %d %d %d\n", tecla, y, x);
/*
	if (!integral 
		|| (decimal_point == '?')
		|| (y > 24) 
		|| (x == 0)
		|| (!isdigit(linhas[y][x-1]) && (linhas[y][x-1] != ' '))) {
		return tecla;
	}

	if ((tecla == VK_OEM_PERIOD)
		|| (tecla == VK_OEM_COMMA)
		|| (tecla == VK_DECIMAL)
		|| (tecla == 194)) {

		return decimal_point;
	}
*/

	return tecla;
}

void ava_seta_cor(int y, int r, int g) {
	if (y == 22) {
		vermelho22 = r == 187 && g == 0;
	} else {
		vermelho23 = r == 187 && g == 0;
	}
	ava_integral(y);
}

void ava_integral(int y) {

	int i;

	fprintf(stderr, "ava_integral %d [%s]\n", y, linhas[y]);
	integral = !strncmp(linhas[y], "Avanco", 6);
	if (integral) {
		return;
	}

	for (i=2; i<80; i++) {
		if ((linhas[y][i] == ':')
			&& (linhas[y][i-2] == 'F')
			&& isdigit(linhas[y][i-1])) {
			integral = 1;
			break;
		}
	}
	if (integral) {
		return;
	}

	integral = vermelho22 || vermelho23;
}
