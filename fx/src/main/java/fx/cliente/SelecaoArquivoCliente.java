package fx.cliente;

import java.util.List;

import fx.modelo.Arquivo;
import fx.servico.SelecaoArquivoServico;

public class SelecaoArquivoCliente {
	
	public static List<Arquivo> carrega(String diretorio, String padrao) {
		return SelecaoArquivoServico.carrega(diretorio, padrao);
	}
	

}
