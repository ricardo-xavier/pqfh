#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(3,0,0)
#include <gdk/gdkkeysyms.h>
#endif

extern char integral;
extern char linhas[25][81];

char vermelho22 = 0;
char vermelho23 = 0;

short decimal_point = '?';

void ava_integral(int y);

void ava_move(int y, int x) {

	int i;

	if (y > 24) {
		return;
	}
	for (i = x; linhas[y][i] == ' '; i++);

	fprintf(stderr, "ava_move %d %d\n", y, x);
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
				decimal_point = decimal == '.' ? '.' : ',';
				break;
			}
		}
	}

	//fprintf(f, "move %d %d %d\n", y, x, decimal_point);
	//fprintf(f, "%s\n", linhas[y]+x);
	//fclose(f);
	
}

short ava_converte_decimal(short tecla, int y, int x) {

	if (!integral 
		|| (decimal_point == '?')
		|| (y > 24) 
		|| (x == 0)
		|| (!isdigit(linhas[y][x-1]) && (linhas[y][x-1] != ' '))) {
		return tecla;
	}

	if ((tecla == '.')
		|| (tecla == ',')
		|| (tecla == GDK_KEY_KP_Decimal)      // .
		|| (tecla == GDK_KEY_KP_Separator)) { // ,

		fprintf(stderr, "ava_converte_decimal %d %d %d\n", y, x, tecla);
		return decimal_point;
	}

	return tecla;
}

void ava_seta_cor(int y, int attr) {
	fprintf(stderr, "seta cor %d %d\n", y, attr);
	if (y == 22) {
		vermelho22 = attr == 31;
		ava_integral(y);
	} else if (y == 23) {
		vermelho23 = attr == 31;
		ava_integral(y);
	}
}

void ava_integral(int y) {

	int i;

	integral = !strncmp(linhas[y], "Avanco", 6);
	if (integral) {
		fprintf(stderr, "ava_integral %d [%s]\n", y, linhas[y]);
		return;
	}

	for (i=2; i<80; i++) {
		if ((linhas[y][i] == ':')
			&& (linhas[y][i-2] == 'F')
			&& isdigit(linhas[y][i-1])) {
			integral = 1;
			fprintf(stderr, "ava_integral %d [%s]\n", y, linhas[y]);
			break;
		}
	}
	if (integral) {
		return;
	}

	integral = vermelho22 || vermelho23;
	if (integral) {
		fprintf(stderr, "ava_integral %d %d %d\n", y, vermelho22, vermelho23);
	}
}
