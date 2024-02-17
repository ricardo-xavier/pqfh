package avanco.emulador;

import java.io.FileNotFoundException;
import java.io.PrintStream;
import java.util.Date;

public class Debug {
    public static void open(int versao) {
        PrintStream log;
        if (System.getenv("EMULADOR_DBG") != null) {
            try {
                log = new PrintStream("emulador.log");
                synchronized (log) {
                    log.println("emulador v" + versao);
                    log.println(new Date());
                    log.println();
                    log.flush();
                }

            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }
    }
}
