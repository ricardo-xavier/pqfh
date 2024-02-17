package avanco.emulador;

import javafx.application.Application;
import javafx.stage.Stage;


public class Main extends Application {
    private Communication com;

    public static void main(String[] args) {
        Main main = new Main();
        Debug.open(1);
        main.com = new Communication();
        main.com.start();
        launch(args);
        if (main.com.isAlive()) {
            main.com.close();
            main.com.stop();
        }
    }


    @Override
    public void start(Stage primaryStage) {
        new Terminal().show();
    }
}
