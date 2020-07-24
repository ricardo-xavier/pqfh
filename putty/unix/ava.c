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

char inativo = 0;
time_t t_enviado = 0;
time_t t_recebido = 0;

void ava_integral();

void ava_inativa() {
	inativo = 1;
	integral = 0;
}

void ava_enviado() {
	t_enviado = time(NULL);
}

void ava_recebido() {
	t_recebido = time(NULL);
}

void ava_tempo_resposta() {
	int tempo = (int) (t_recebido - t_enviado);
	//fprintf(stderr, "tempo_resposta %d\n", tempo);
	if ((tempo > 10) || (tempo < -10)) {
		//fprintf(stderr, "INATIVO\n");
		integral = 0;
	}
}

void ava_move(int y, int x) {

	int i;

	if (y > 24) {
		return;
	}
	for (i = x; linhas[y][i] == ' '; i++);

	//fprintf(stderr, "ava_move %d %d\n", y, x);
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

short ava_converte_decimal(int tecla, int y, int x) {

	if (!integral 
		|| (decimal_point == '?')
		|| (y > 24) 
		|| (x == 0)
		|| (!isdigit(linhas[y][x-1]) 
			&& (linhas[y][x-1] != ' ') 
			&& (linhas[y][x-1] != '|') 
			&& (((unsigned char) linhas[y][x-1]) > 31)
			&& (((unsigned char) linhas[y][x-1]) < 128))) {
		return tecla;
	}

	if ((tecla == '.')
		|| (tecla == ',')
		|| (tecla == GDK_KEY_KP_Decimal)      // .
		|| (tecla == GDK_KEY_KP_Separator)) { // ,

		//fprintf(stderr, "ava_converte_decimal %d %d %d %d\n", y, x, tecla, decimal_point);
		return decimal_point;
	}

	return tecla;
}

void ava_seta_cor(int y, int nbg) {
	//fprintf(stderr, "seta cor %d %d\n", y, nbg);
	if (y == 22) {
		vermelho22 = nbg == 1;
		ava_integral(y);
	} else if (y == 23) {
		vermelho23 = nbg == 1;
		ava_integral(y);
	}
}

void ava_integral() {

	int i, y;

	if (inativo) {
		integral = 0;
		return;
	}

	integral = !strncmp(linhas[22], "Avanco", 6)
			|| !strncmp(linhas[23], "Avanco", 6)
			|| (strstr(linhas[22], "Confirma") != NULL)
			|| (strstr(linhas[23], "Confirma") != NULL);
	if (integral) {
		//fprintf(stderr, "ava_integral %d [%s]\n", y, linhas[y]);
		return;
	}

	for (i=2; i<80; i++) {
		for (y=22; y<24; y++) {
			if ((linhas[y][i] == ':')
				&& (linhas[y][i-2] == 'F')
				&& isdigit(linhas[y][i-1])) {
				integral = 1;
				y = 24;
				//fprintf(stderr, "ava_integral %d [%s]\n", y, linhas[y]);
				break;
			}
		}
	}
	if (integral) {
		return;
	}

	integral = vermelho22 || vermelho23;
	if (integral) {
		//fprintf(stderr, "ava_integral %d %d %d\n", y, vermelho22, vermelho23);
	}
}
