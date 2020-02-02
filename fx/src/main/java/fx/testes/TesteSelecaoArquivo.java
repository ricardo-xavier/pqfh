package fx.testes;

import fx.modelo.Arquivo;
import fx.visao.SelecaoArquivo;
import javafx.application.Application;
import javafx.stage.Stage;

public class TesteSelecaoArquivo extends Application {
	
	public static void main(String[] args) {
		launch(args);
	}

	@Override
	public void start(Stage primaryStage) throws Exception {
		Parameters args = getParameters();
		SelecaoArquivo stage = new SelecaoArquivo(args.getRaw().get(0), args.getRaw().get(1));
		stage.show();
		Arquivo arquivo = stage.getArquivo();
		if (arquivo != null) {
			System.out.println(arquivo.getNome());
		}
		
	}

}
