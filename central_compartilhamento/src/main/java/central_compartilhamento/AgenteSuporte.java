package central_compartilhamento;

import java.io.IOException;
import java.net.Socket;

public class AgenteSuporte extends Thread {

	public AgenteSuporte(Socket sock, Agente agente) {

		try {

			String[] dados = agente.getDados();
			String[] frente = agente.getFrente();
			String[] fundo = agente.getFundo();
			String[] atributos = agente.getAtributos();

			for (int lin = 0; lin < 25; lin++) {

				String cmd = String.format("%-10s%02d%s%n", "DADOS", lin, new String(dados[lin]));
				System.err.print(cmd);
				sock.getOutputStream().write(cmd.getBytes());

				cmd = String.format("%-10s%02d%s%n", "FRENTE", lin, new String(frente[lin]));
				sock.getOutputStream().write(cmd.getBytes());

				cmd = String.format("%-10s%02d%s%n", "FUNDO", lin, new String(fundo[lin]));
				sock.getOutputStream().write(cmd.getBytes());

				cmd = String.format("%-10s%02d%s%n", "ATRIBUTOS", lin, new String(atributos[lin]));
				sock.getOutputStream().write(cmd.getBytes());

			}
			
		} catch (IOException e) {
			e.printStackTrace();
		}

	}

}
