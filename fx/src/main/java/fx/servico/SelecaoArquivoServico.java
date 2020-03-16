package fx.servico;

import java.io.File;
import java.io.FilenameFilter;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import fx.modelo.Arquivo;

public class SelecaoArquivoServico {
	
	public static List<Arquivo> carrega(String diretorio, String padrao) {
		
		List<Arquivo> arquivos = new ArrayList<Arquivo>();
		
		File dir = new File(diretorio);
		
		FilenameFilter filter = new FilenameFilter() {
			
			@Override
			public boolean accept(File dir, String name) {
				String regex = padrao.replace("*", ".*");
				return name.matches(regex);
			}
		};
		
		DateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm");
		
		File[] files = dir.listFiles(filter);
		for (File file : files) {
			Date d = new Date(file.lastModified());
			Arquivo arquivo = new Arquivo(file.getName(), df.format(d), (int) file.length());
			arquivos.add(arquivo);
		}
		
		
		return arquivos;
		
	}

}
