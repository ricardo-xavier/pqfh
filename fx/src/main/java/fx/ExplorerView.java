package fx;

import javafx.application.Application;
import javafx.stage.Stage;

public class ExplorerView extends Application {

	public static void main(String[] args) {
		launch(args);
	}

	@Override
	public void start(Stage primaryStage) throws Exception {
		
		ExplorerStage stage = new ExplorerStage();
		stage.show();
		
	}

}
