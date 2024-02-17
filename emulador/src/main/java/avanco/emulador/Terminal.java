package avanco.emulador;

import javafx.scene.Scene;
import javafx.scene.canvas.Canvas;
import javafx.scene.layout.BorderPane;
import javafx.stage.Stage;

public class Terminal extends Stage {
    public Terminal() {
        Canvas canvas = new Canvas(800, 600);
        BorderPane screen = new BorderPane();
        screen.setCenter(canvas);

        Scene scene = new Scene(screen);
        setScene(scene);
    }
}
